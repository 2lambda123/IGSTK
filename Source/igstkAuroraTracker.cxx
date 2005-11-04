/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkAuroraTracker.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISIS Georgetown University. All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more DEBUGrmation.

=========================================================================*/

#if defined(_MSC_VER)
// Warning about: identifier was truncated to '255' characters in the
// debug information (MVC6.0 Debug)
#pragma warning( disable : 4786 )
#endif

#include "igstkAuroraTracker.h"
#include "igstkTrackerPort.h"

namespace igstk
{

/** Constructor: Initializes all internal variables. */
AuroraTracker::AuroraTracker(void)
{
  m_CommandInterpreter = CommandInterpreterType::New();
  m_NumberOfTools = 0;
  for (unsigned int i = 0; i < NumberOfPorts; i++)
    {
    this->m_PortEnabled[i] = 0;
    this->m_PortHandle[i] = 0;
    }
    
  this->SetThreadingEnabled( true );

  m_BufferLock = itk::MutexLock::New();
}

/** Destructor */
AuroraTracker::~AuroraTracker(void)
{
}

/** Helper function for reporting interpreter errors. */
AuroraTracker::ResultType
AuroraTracker::CheckError(CommandInterpreterType *interpreter)
{
  const int errnum = interpreter->GetError();
  if (errnum)
    {
    igstkLogMacro( WARNING, interpreter->ErrorString(errnum) << "\n");
    return FAILURE;
    }

  return SUCCESS;
}

/** Set the communication object, it will be initialized as necessary
  * for use with the Aurora */
void AuroraTracker::SetCommunication( CommunicationType *communication )
{
  igstkLogMacro( DEBUG, "AuroraTracker:: Entered SetCommunication ...\n");
  m_Communication = communication;
  m_CommandInterpreter->SetCommunication( communication );

  // data records are of variable length and end with a carriage return
  if( communication )
    {
    communication->SetUseReadTerminationCharacter( true );
    communication->SetReadTerminationCharacter( '\r' );
    }

  igstkLogMacro( DEBUG, "AuroraTracker:: Exiting SetCommunication ...\n"); 
}

/** Open communication with the tracking device. */
AuroraTracker::ResultType AuroraTracker::InternalOpen( void )
{
  igstkLogMacro( DEBUG, "AuroraTracker::InternalOpen called ...\n");

  m_CommandInterpreter->RESET();
  m_CommandInterpreter->INIT();

  // If it still failed to initialize, fail
  return this->CheckError(m_CommandInterpreter);
}

/** Close communication with the tracking device. */
AuroraTracker::ResultType AuroraTracker::InternalClose( void )
{
  igstkLogMacro( DEBUG, "AuroraTracker::InternalClose called ...\n");

  // return the device back to its initial comm setttings
  m_CommandInterpreter->COMM(CommandInterpreterType::NDI_9600,
                             CommandInterpreterType::NDI_8N1,
                             CommandInterpreterType::NDI_NOHANDSHAKE);

  return this->CheckError(m_CommandInterpreter);
}

/** Activate the tools attached to the tracking device. */
AuroraTracker::ResultType AuroraTracker::InternalActivateTools( void )
{
  igstkLogMacro( DEBUG, "AuroraTracker::InternalActivateTools called ...\n");

  this->EnableToolPorts();

  this->ClearPorts();

  m_NumberOfTools = 0;

  for(unsigned int i = 0; i < NumberOfPorts; i++)
    { 
    if( this->m_PortEnabled[i] )
      {
      AuroraTrackerToolPointer tool = AuroraTrackerToolType::New();
      TrackerPortPointer port = TrackerPortType::New();
      port->AddTool(tool);
      this->AddPort(port);
      m_NumberOfTools++;
      }
    }

  return SUCCESS;
}

/** Deactivate the tools attached to the tracking device. */
AuroraTracker::ResultType AuroraTracker::InternalDeactivateTools( void )
{
  this->DisableToolPorts();

  return SUCCESS;
}

/** Put the tracking device into tracking mode. */
AuroraTracker::ResultType AuroraTracker::InternalStartTracking( void )
{
  igstkLogMacro( DEBUG, "AuroraTracker::InternalStartTracking called ...\n");  

  m_CommandInterpreter->TSTART();

  return this->CheckError(m_CommandInterpreter);
}

/** Take the tracking device out of tracking mode. */
AuroraTracker::ResultType AuroraTracker::InternalStopTracking( void )
{
  igstkLogMacro( DEBUG, "AuroraTracker::InternalStopTracking called ...\n");

  m_CommandInterpreter->TSTOP();

  return this->CheckError(m_CommandInterpreter);
}

/** Reset the tracking device to put it back to its original state. */
AuroraTracker::ResultType AuroraTracker::InternalReset( void )
{
  m_CommandInterpreter->RESET();
  m_CommandInterpreter->INIT();

  return this->CheckError(m_CommandInterpreter);
}

/** Update the status and the transforms for all TrackerTools. */
AuroraTracker::ResultType AuroraTracker::InternalUpdateStatus()
{
  igstkLogMacro( DEBUG, "AuroraTracker::InternalUpdateStatus called ...\n");


  // these flags are set for tools that can be used for tracking
  const unsigned long mflags = (CommandInterpreterType::NDI_TOOL_IN_PORT |
                                CommandInterpreterType::NDI_INITIALIZED |
                                CommandInterpreterType::NDI_ENABLED);

  m_BufferLock->Lock();

  for (unsigned int port = 0; port < NumberOfPorts; port++) 
    {
    // convert m_StatusBuffer flags from NDI to vtkTracker format
    const int portStatus = m_StatusBuffer[port];

    // only report tools that are enabled
    if ((portStatus & mflags) != mflags) 
      {
      continue;
      }

    // only report tools that are in view
    if (m_AbsentBuffer[port] ||
        portStatus & CommandInterpreterType::NDI_OUT_OF_VOLUME)
      {
      // there should be a method to set that the tool is not in view
      continue;
      }

    // create the transform
    TransformType transform;

    typedef TransformType::VectorType TranslationType;
    TranslationType translation;

    translation[0] = m_TransformBuffer[port][4];
    translation[1] = m_TransformBuffer[port][5];
    translation[2] = m_TransformBuffer[port][6];

    typedef TransformType::VersorType RotationType;
    RotationType rotation;
    const double normsquared = 
      m_TransformBuffer[port][0]*m_TransformBuffer[port][0] +
      m_TransformBuffer[port][1]*m_TransformBuffer[port][1] +
      m_TransformBuffer[port][2]*m_TransformBuffer[port][2] +
      m_TransformBuffer[port][3]*m_TransformBuffer[port][3];

    // don't allow null quaternions
    if (normsquared < 1e-6)
      {
      rotation.Set(1.0, 0.0, 0.0, 0.0);
      igstkLogMacro( WARNING, "AuroraTracker::InternUpdateStatus: bad "
                     "quaternion, norm=" << sqrt(normsquared) << "\n");
      }
    else
      {
      rotation.Set(m_TransformBuffer[port][0],
                   m_TransformBuffer[port][1],
                   m_TransformBuffer[port][2],
                   m_TransformBuffer[port][3]);
      }

    // report NDI error value
    typedef TransformType::ErrorType  ErrorType;
    ErrorType errorValue = m_TransformBuffer[port][7];

    typedef TransformType::TimePeriodType TimePeriodType;
    const TimePeriodType validityTime = 100.0;


    transform.SetToIdentity(validityTime);
    transform.SetTranslationAndRotation(translation, rotation, errorValue,
                                        validityTime);

    this->SetToolTransform(port, 0, transform);
    }
  m_BufferLock->Unlock();

  return SUCCESS;
}

/** Update the m_StatusBuffer and the transforms. 
    This function is called by a separate thread. */
AuroraTracker::ResultType AuroraTracker::InternalThreadedUpdateStatus( void )
{
  igstkLogMacro( DEBUG, "AuroraTracker::InternalThreadedUpdateStatus "
                 "called ...\n");

  // Initialize transformations to identity.
  // The NDI transform is 8 values:
  // the first 4 values are a quaternion
  // the next 3 values are an x,y,z position
  // the final value is an error estimate in the range [0,1]
  m_BufferLock->Lock();
  unsigned int port;   // physical port number
  for (port = 0; port < NumberOfPorts; port++)
    {
    m_TransformBuffer[port][0] = 1.0;
    m_TransformBuffer[port][1] = 0.0;
    m_TransformBuffer[port][2] = 0.0;
    m_TransformBuffer[port][3] = 0.0;
    m_TransformBuffer[port][4] = 0.0;
    m_TransformBuffer[port][5] = 0.0;
    m_TransformBuffer[port][6] = 0.0;
    m_TransformBuffer[port][7] = 0.0;
    }
  m_BufferLock->Unlock();

  // get the transforms for all tools from the NDI
  m_CommandInterpreter->TX(CommandInterpreterType::NDI_XFORMS_AND_STATUS);

  ResultType result = this->CheckError(m_CommandInterpreter);

  if (result == SUCCESS)
    {
    m_BufferLock->Lock();
    unsigned long frame[NumberOfPorts];
    for (port = 0; port < NumberOfPorts; port++)
      {
      const int ph = this->m_PortHandle[port];
      m_AbsentBuffer[port] = 0;
      m_StatusBuffer[port] = 0;

      frame[port] = 0;
      if (ph == 0)
        {
        continue;
        }

      double transform[8];
      const int tstatus = m_CommandInterpreter->GetTXTransform(ph, transform);
      const int absent = (tstatus != CommandInterpreterType::NDI_VALID);
      const int status = m_CommandInterpreter->GetTXPortStatus(ph);
      frame[port] = m_CommandInterpreter->GetTXFrame(ph);

      for(unsigned int i = 0; i < 8; i++ )
        {
        m_TransformBuffer[port][i] = transform[i];
        }

      m_AbsentBuffer[port] = absent;
      m_StatusBuffer[port] = status;
      }
    m_BufferLock->Unlock();
    }

  // In the original vtkNDITracker code, there was a check at this
  // point in the code to see if any new tools had been plugged in

  return result;
}


/** Enable all tool ports that are occupied. */
void AuroraTracker::EnableToolPorts()
{
  // reset our information about the tool ports
  for (unsigned int port = 0; port < NumberOfPorts; port++)
    {
    this->m_PortHandle[port] = 0;
    this->m_PortEnabled[port] = 0;
    }

  // free ports that are waiting to be freed
  m_CommandInterpreter->PHSR(CommandInterpreterType::NDI_STALE_HANDLES);
  unsigned int ntools = m_CommandInterpreter->GetPHSRNumberOfHandles();
  unsigned int tool;
  for (tool = 0; tool < ntools; tool++)
    {
    const int ph = m_CommandInterpreter->GetPHSRHandle(tool);
    m_CommandInterpreter->PHF(ph);

    // if an error occurs, print the error but don't abort
    this->CheckError(m_CommandInterpreter);
    }

  // initialize ports waiting to be initialized
  do // repeat as necessary (in case multi-channel tools are used) 
    {
    m_CommandInterpreter->PHSR(
      CommandInterpreterType::NDI_UNINITIALIZED_HANDLES);
    
    ntools = m_CommandInterpreter->GetPHSRNumberOfHandles();
    for (tool = 0; tool < ntools; tool++)
      {
      const int ph = m_CommandInterpreter->GetPHSRHandle(tool);
      m_CommandInterpreter->PINIT(ph);

      if (this->CheckError(m_CommandInterpreter) == FAILURE)
        {
        break;
        }
      }
    }
  while( ntools > 0 );

  // enable initialized tools
  m_CommandInterpreter->PHSR(CommandInterpreterType::NDI_UNENABLED_HANDLES);

  ntools = m_CommandInterpreter->GetPHSRNumberOfHandles();
  
  for (tool = 0; tool < ntools; tool++)
    {
    const int ph = m_CommandInterpreter->GetPHSRHandle(tool);
    m_CommandInterpreter->PHINF(ph, CommandInterpreterType::NDI_BASIC);

    // tool identity and type information
    char identity[512];
    m_CommandInterpreter->GetPHINFToolInfo(identity);

    // use tool type information to figure out mode for enabling
    int mode = CommandInterpreterType::NDI_DYNAMIC;

    if (identity[1] == CommandInterpreterType::NDI_TYPE_BUTTON)
      { // button-box or foot pedal
      mode = CommandInterpreterType::NDI_BUTTON_BOX;
      }
    else if (identity[1] == CommandInterpreterType::NDI_TYPE_REFERENCE)
      { // reference
      mode = CommandInterpreterType::NDI_STATIC;
      }

    // enable the tool
    m_CommandInterpreter->PENA(ph, mode);

    // print any warnings
    this->CheckError(m_CommandInterpreter);
    }

  // get information for all tools
  m_CommandInterpreter->PHSR(CommandInterpreterType::NDI_ALL_HANDLES);

  ntools = m_CommandInterpreter->GetPHSRNumberOfHandles();

  for (tool = 0; tool < ntools; tool++)
    {
    const int ph = m_CommandInterpreter->GetPHSRHandle(tool);
    m_CommandInterpreter->PHINF(ph,
                                CommandInterpreterType::NDI_PORT_LOCATION |
                                CommandInterpreterType::NDI_PART_NUMBER |
                                CommandInterpreterType::NDI_BASIC );

    if (this->CheckError(m_CommandInterpreter) == FAILURE)
      {
      continue;
      }

    // get the physical port identifier
    char location[512];
    m_CommandInterpreter->GetPHINFPortLocation(location);

    unsigned int port = (location[10]-'0')*10 + (location[11]-'0') - 1;

    if (port < NumberOfPorts)
      {
      this->m_PortHandle[port] = ph;
      }

    const int status = m_CommandInterpreter->GetPHINFPortStatus();

    this->m_PortEnabled[port] = 
      ((status & CommandInterpreterType::NDI_ENABLED) != 0);
    }
}

/** Disable all tool ports. */
void AuroraTracker::DisableToolPorts( void )
{
  // disable all enabled tools
  m_CommandInterpreter->PHSR(CommandInterpreterType::NDI_ENABLED_HANDLES);
  unsigned int ntools = m_CommandInterpreter->GetPHSRNumberOfHandles();

  for (unsigned int tool = 0; tool < ntools; tool++)
    {
    const int ph = m_CommandInterpreter->GetPHSRHandle(tool);
    m_CommandInterpreter->PDIS( ph );

    // print warning if failed to disable
    this->CheckError(m_CommandInterpreter);
    }

  // disable the enabled ports
  for (unsigned int port = 0; port < NumberOfPorts; port++)
    {
    this->m_PortEnabled[port] = 0;
    }
}


/** Print Self function */
void AuroraTracker::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

  unsigned int i;
  for( i = 0; i < NumberOfPorts; ++i )
    {
    os << indent << "Port " << i << " Enabled: " << m_PortEnabled[i] << std::endl;
    }
  for( i = 0; i < NumberOfPorts; ++i )
    {
    os << indent << "Port " << i << " Handle: " << m_PortHandle[i] << std::endl;
    }
  os << indent << "Number of tools: " << m_NumberOfTools << std::endl;
}

} // end of namespace igstk

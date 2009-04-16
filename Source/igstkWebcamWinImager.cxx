/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkWebcamWinImager.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if defined(_MSC_VER)
// Warning about: identifier was truncated to '255' characters in the
// debug information (MVC6.0 Debug)
#pragma warning( disable : 4786 )
#endif


#include "igstkWebcamWinImager.h"
#include "igstkEvents.h"
#include "vtkImageData.h"
#include <itksys/SystemTools.hxx>

#include "vtkWin32VideoSource.h"
#include "vtkImageViewer2.h"
#include "vtkImageData.h"
#include "vtkImageIterator.h"
#include <vtkPNGWriter.h>

#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "itkImage.h"
#include "itkImportImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkRGBPixel.h"

#define MAX_DEVICES 64
#define MAX_FORMATS 64


namespace igstk
{


//Initialize static variables
std::map< unsigned int, std::string> WebcamWinImager::m_ErrorCodeContainer;
bool WebcamWinImager::m_ErrorCodeListCreated = false;
unsigned char* WebcamWinImager::pixels[1] = {NULL};
unsigned char* WebcamWinImager::frameBuffer = NULL;
itk::MutexLock::Pointer WebcamWinImager::m_FrameBufferLock = itk::MutexLock::New();


/** Constructor: Initializes all internal variables. */
WebcamWinImager::WebcamWinImager(void):m_StateMachine(this)
{
  this->m_NumberOfTools = 0;

  this->SetThreadingEnabled( true );

  // Lock for the data buffer that is used to transfer the
  // frames from thread that is communicating
  // with the imager to the main thread.
  m_BufferLock = itk::MutexLock::New();



  // Create error code list if it hasn't been
  // created.
  if( ! m_ErrorCodeListCreated )
    {
    this->CreateErrorCodeList();
    m_ErrorCodeListCreated = true;
    }

  this->capture = 0;
  this->cvframe = 0;
  key = 0;


}

/** Destructor */
WebcamWinImager::~WebcamWinImager(void)
{
  cvReleaseCapture( &capture );
}

/** Create a map data structure containing MTC error code and description */
void WebcamWinImager::CreateErrorCodeList()
{
  ErrorCodeContainerType & ecc = m_ErrorCodeContainer;

  ecc[0]  = "OK";
  ecc[1]  = "Invalid object handle";
  ecc[2]  = "Reentrant access - library is not thread-safe";
  ecc[3]  = "Internal WebcamWinImager software error";
  ecc[4]  = "Null pointer parameter";
  ecc[5]  = "Out of memory";
  ecc[6]  = "Parameter out of range";
  ecc[7]  = "String parameter too long";
  ecc[8]  = "Insufficient space allocated by the client to the output buffer";
  ecc[9]  = "Camera not initialized";
  ecc[10] = "Camera already initialized - cannot be initialized twice";
  ecc[11] = "Camera initialization failed";
  ecc[12] = "MTC is incompatible with a software module it calls";
  ecc[13] = "Calibration file error: unrecognized camera model";
  ecc[14] = "Path not set";
  ecc[15] = "Cannot access the directory specified";
  ecc[16] = "Write to file failed";
  ecc[17] = "Invalid Index parameter";
  ecc[18] = "Invalid SideI parameter";
  ecc[19] = "Invalid Divisor parameter";
  ecc[20] = "Attempting to access an item of an empty IntCollection";
  ecc[21] = "Insufficient samples";
  ecc[22] = "Insufficient samples that fit within the acceptance tolerance";
  ecc[23] = "Odd number of vector samples";
  ecc[24] = "Less than 2 vectors";
  ecc[25] = "More than maximum vectors per facet";
  ecc[26] = "Error exceeds tolerance";
  ecc[27] = "Insufficient angle between vectors";
  ecc[28] = "First vector is shorter than the second";
  ecc[29] = "Vector lengths are too similar";
  ecc[30] = "Template vector has 0 length";
  ecc[31] = "The template has not been created or loaded";
  ecc[32] = "Template file is corrupt";
  ecc[33] = "Maximum number of marker templates allowed exceeded";
  ecc[34] = "Geometries of different facets are too similar";
  ecc[35] = "Noncompliant facet definition";
  ecc[36] = "The SampledVectorPairsCollection contains non-Vector handles";
  ecc[37] = "Empty pixels buffer";
  ecc[38] = "Dimensions do not match";
  ecc[39] = "File open failed";
  ecc[40] = "File read failed";
  ecc[41] = "File write failed";
  ecc[42] = "Cannot open calibration file "
            "(typically named [driver]_[ser num].calib";
  ecc[43] = "Not a calibration file";
  ecc[44] = "Calibration file contents corrupt";
  ecc[45] = "Calibration file was not generated from this camera";
  ecc[46] = "Calibration file not loaded";
  ecc[47] = "Incorrect file version";
  ecc[48] = "Input image location is out of bounds of the measurement volume";
  ecc[49] = "Input image locations do not triangulate to a valid 3-D point";
  ecc[50] = "Transform between coordinate spaces is unknown";
  ecc[51] = "The given camera object was not found in the cameras array";
  ecc[52] = "Feature Data unavailable for the current frame";
  ecc[53] = "Feature Data is corrupt or incompatible with the current version";
  ecc[54] = "XYZ position is outside of calibrated field of view";
  ecc[55] = "Grab frame error";
}

std::string
WebcamWinImager::GetErrorDescription( unsigned int code )
{
  if ( code >= 0 && code <= 55 )
    {
    return WebcamWinImager::m_ErrorCodeContainer[code];
    }
  else
    {
    return "Unknown error code";
    }
}

WebcamWinImager::ResultType WebcamWinImager::InternalOpen( void )
{
  igstkLogMacro( DEBUG, "igstk::WebcamWinImager::InternalOpen called ...\n");

  if( ! this->Initialize() )
    {
    igstkLogMacro( CRITICAL, "Error initializing");
    return FAILURE;
    }

  return SUCCESS;
}

/** Initialize socket */
bool WebcamWinImager::Initialize( void )
{
  igstkLogMacro( DEBUG, "igstk::WebcamWinImager::Initialize called ...\n");

    capture = cvCaptureFromCAM( 0 );
    if ( !capture ) {
        fprintf( stderr, "Cannot open initialize webcam!\n" );
        return 1;
    }

  return SUCCESS;
}

/** Verify imager tool information. */
WebcamWinImager::ResultType
WebcamWinImager
::VerifyImagerToolInformation( const ImagerToolType * imagerTool )
{
  igstkLogMacro( DEBUG,
    "igstk::WebcamWinImager::VerifyImagerToolInformation called ...\n");

  return SUCCESS;
}

/** Detach camera. */
WebcamWinImager::ResultType WebcamWinImager::InternalClose( void )
{
  igstkLogMacro( DEBUG, "igstk::WebcamWinImager::InternalClose called ...\n");

  cvReleaseCapture( &capture );

  return SUCCESS;
}

/** Put the imaging device into imaging mode. */
WebcamWinImager::ResultType WebcamWinImager::InternalStartImaging( void )
{
  igstkLogMacro( DEBUG,
    "igstk::WebcamWinImager::InternalStartImaging called ...\n");

    igstk::DoubleTypeEvent evt;
    evt.Set( 1.0 );
    this->InvokeEvent( evt );
    std::cout << "Start capture on device!" << std::endl;

  return SUCCESS;
}

/** Take the imaging device out of imaging mode. */
WebcamWinImager::ResultType WebcamWinImager::InternalStopImaging( void )
{
  igstkLogMacro( DEBUG,
    "igstk::WebcamWinImager::InternalStopImaging called ...\n");
  /*
    Stop the device
  */
  return SUCCESS;
}

/** Reset the imaging device to put it back to its original state. */
WebcamWinImager::ResultType WebcamWinImager::InternalReset( void )
{
  igstkLogMacro( DEBUG, "igstk::WebcamWinImager::InternalReset called ...\n");
  return SUCCESS;
}

/** Update the status and the transforms for all ImagerTools. */
WebcamWinImager::ResultType WebcamWinImager::InternalUpdateStatus()
{
  igstkLogMacro( DEBUG,
    "igstk::WebcamWinImager::InternalUpdateStatus called ...\n");

  // This method and the InternalThreadedUpdateStatus are both called
  // continuously in the Imaging state.  This method is called from
  // the main thread, while InternalThreadedUpdateStatus is called
  // from the thread that actually communicates with the device.
  // A shared memory buffer is used to transfer information between
  // the threads, and it must be locked when either thread is
  // accessing it.
  m_BufferLock->Lock();

  typedef ImagerToolFrameContainerType::const_iterator  InputConstIterator;

  InputConstIterator inputItr = this->m_ToolFrameBuffer.begin();
  InputConstIterator inputEnd = this->m_ToolFrameBuffer.end();

  ImagerToolsContainerType imagerToolContainer =
  this->GetImagerToolContainer();

  unsigned int toolId = 0;

  while( inputItr != inputEnd )
    {
    // only report tools that have useful data
    if (! this->m_ToolStatusContainer[inputItr->first])
    {
      igstkLogMacro( DEBUG, "igstk::WebcamWinImager::InternalUpdateStatus: " <<
              "tool " << inputItr->first << " failed\n");

      // report to the imager tool that the imager is not available
      this->ReportImagingToolNotAvailable(
        imagerToolContainer[inputItr->first]);

      ++inputItr;
      continue;
    }

    // report to the imager tool that the tool is sending frames
    this->ReportImagingToolVisible(imagerToolContainer[inputItr->first]);

    cout << ":" << endl;

    this->SetImagerToolFrame(
          imagerToolContainer[inputItr->first], (inputItr->second) );

    this->SetImagerToolUpdate(
      imagerToolContainer[inputItr->first], true );

    ++inputItr;
    ++toolId;
    }

  m_BufferLock->Unlock();

  return SUCCESS;
}

/** Update the shared memory buffer and the tool's internal frame. This function
 *  is called by the thread that communicates with the imager while
 *  the imager is in the Imaging state. */
WebcamWinImager::ResultType
WebcamWinImager::InternalThreadedUpdateStatus( void )
{
  igstkLogMacro( DEBUG, "igstk::WebcamWinImager::InternalThreadedUpdateStatus called ...\n");

  // Lock the buffer that this method shares with InternalUpdateStatus
  m_BufferLock->Lock();

  //reset the status of all the imager tools
  typedef ImagerToolFrameContainerType::const_iterator  InputConstIterator;
  InputConstIterator inputItr = this->m_ToolFrameBuffer.begin();
  InputConstIterator inputEnd = this->m_ToolFrameBuffer.end();

  while( inputItr != inputEnd )
  {
    this->m_ToolStatusContainer[inputItr->first] = 0;
    ++inputItr;
  }

  try
  {
    igstkLogMacro( DEBUG, "InternalThreadedUpdateStatus Receive passed" );
    // Check if an imager tool was added with this device name
    typedef ImagerToolFrameContainerType::iterator InputIterator;

    //TODO toolname hard coded
    InputIterator deviceItr = this->m_ToolFrameBuffer.find( "Camera" );

    if( deviceItr != this->m_ToolFrameBuffer.end() )
      {
      // create the frame
      ImagerToolsContainerType imagerToolContainer =
                                              this->GetImagerToolContainer();

      FrameType frame;
      this->GetImagerToolFrame( imagerToolContainer[deviceItr->first], frame );

      unsigned int frameDims[3];
      imagerToolContainer[deviceItr->first]->GetFrameDimensions(frameDims);

      WebcamWinImager::m_FrameBufferLock->Lock();

      cvframe = cvQueryFrame( capture );

      if( !cvframe )
        cout << "frame failed ";

      memcpy(frame.GetImagePtr(),
          (unsigned char*)cvframe->imageData,frameDims[0]*frameDims[1]*frameDims[2]);

      WebcamWinImager::m_FrameBufferLock->Unlock();

      //update frame validity time
      frame.SetTimeToExpiration(this->GetValidityTime());

      this->m_ToolFrameBuffer[ deviceItr->first ] = frame;
      this->m_ToolStatusContainer[ deviceItr->first ] = 1;
      }
     m_BufferLock->Unlock();
     return SUCCESS;
  }
  catch(...)
  {
    igstkLogMacro( CRITICAL, "Unknown error catched" );
    m_BufferLock->Unlock();
    return FAILURE;
  }
}

WebcamWinImager::ResultType
WebcamWinImager::
AddImagerToolToInternalDataContainers( const ImagerToolType * imagerTool )
{
  igstkLogMacro( DEBUG,
    "igstk::WebcamWinImager::AddImagerToolToInternalDataContainers "
                 "called ...\n");

  if ( imagerTool == NULL )
  {
    return FAILURE;
  }

  const std::string imagerToolIdentifier =
                  imagerTool->GetImagerToolIdentifier();

  igstk::Frame frame;

  this->m_ToolFrameBuffer[ imagerToolIdentifier ] = frame;
  this->m_ToolStatusContainer[ imagerToolIdentifier ] = 0;

  return SUCCESS;
}


WebcamWinImager::ResultType
WebcamWinImager::
RemoveImagerToolFromInternalDataContainers
( const ImagerToolType * imagerTool )
{
  igstkLogMacro( DEBUG,
    "igstk::WebcamWinImager::RemoveImagerToolFromInternalDataContainers "
                 "called ...\n");

  const std::string imagerToolIdentifier =
                      imagerTool->GetImagerToolIdentifier();

  // remove the tool from the frame buffer and status container
  this->m_ToolStatusContainer.erase( imagerToolIdentifier );
  this->m_ToolFrameBuffer.erase( imagerToolIdentifier );

  return SUCCESS;
}

/**The "ValidateSpecifiedFrequency" method checks if the specified
  * frequency is valid for the imaging device that is being used. */
WebcamWinImager::ResultType
WebcamWinImager::ValidateSpecifiedFrequency( double frequencyInHz )
{
  const double MAXIMUM_FREQUENCY = 30;
  if ( frequencyInHz < 0.0 || frequencyInHz > MAXIMUM_FREQUENCY )
    {
    return FAILURE;
    }
  return SUCCESS;
}

/** Print Self function */
void WebcamWinImager::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << " output Imaging Source Converter parameters " << std::endl;
}

} // end of namespace igstk
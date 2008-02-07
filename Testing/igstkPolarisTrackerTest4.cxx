/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkPolarisTrackerTest4.cxx
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
//  Warning about: identifier was truncated to '255' characters 
//  in the debug information (MVC6.0 Debug)
#pragma warning( disable : 4786 )
#endif

#include <iostream>
#include <fstream>
#include <set>

#include "itkCommand.h"
#include "igstkLogger.h"
#include "itkStdStreamLogOutput.h"
#include "itkVector.h"
#include "itkVersor.h"

#include "igstkSystemInformation.h"
#include "igstkSerialCommunication.h"
#include "igstkSerialCommunicationSimulator.h"
#include "igstkPolarisTracker.h"
#include "igstkPolarisTrackerTool.h"
#include "igstkTransform.h"

class CoordinateReferenceSystemObserver : public ::itk::Command
{
public:
  typedef igstk::CoordinateReferenceSystemTransformToEvent  EventType;
  typedef igstk::CoordinateReferenceSystemTransformToResult PayloadType;
  typedef igstk::Transform                                  TransformType;

  /** Standard class typedefs. */
  typedef CoordinateReferenceSystemObserver         Self;
  typedef ::itk::Command                            Superclass;
  typedef ::itk::SmartPointer<Self>        Pointer;
  typedef ::itk::SmartPointer<const Self>  ConstPointer;
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(CoordinateReferenceSystemObserver, ::itk::Command);
  itkNewMacro(CoordinateReferenceSystemObserver);

  CoordinateReferenceSystemObserver()
    {
    m_GotPayload = false;
    m_Payload.Clear();
    }

  ~CoordinateReferenceSystemObserver()
    {
    m_GotPayload = false;
    m_Payload.Clear();
    }

  void ClearPayload()
    {
    m_GotPayload = false;
    m_Payload.Clear();
    }

  void Execute(const itk::Object *caller, const itk::EventObject & event)
    {
    this->ClearPayload();
    if( EventType().CheckEvent( &event ) )
      {
      const EventType * transformEvent = 
                dynamic_cast< const EventType *>( &event );
      if( transformEvent )
        {
        m_Payload = transformEvent->Get();
        m_GotPayload = true;
        }
      }
    else
      {
      std::cout << "Got unexpected event : " << std::endl;
      event.Print(std::cout);
      }
    }

  void Execute(itk::Object *caller, const itk::EventObject & event)
    {
    this->Execute(static_cast<const itk::Object*>(caller), event);
    }

  bool GotPayload() const
    {
    return m_GotPayload;
    }

  const PayloadType & GetPayload() const
    {
    return m_Payload;
    }

  const TransformType & GetTransform() const
    {
    return m_Payload.GetTransform();
    }

protected:

  TransformType m_Transform;
  PayloadType   m_Payload;
  bool          m_GotPayload;

};

class PolarisTrackerTest4Command : public itk::Command 
{
public:
  typedef  PolarisTrackerTest4Command   Self;
  typedef  itk::Command                Superclass;
  typedef itk::SmartPointer<Self>      Pointer;
  itkNewMacro( Self );
protected:
  PolarisTrackerTest4Command() {};

public:
  void Execute(itk::Object *caller, const itk::EventObject & event)
    {
    Execute( (const itk::Object *)caller, event);
    }

  void Execute(const itk::Object * object, const itk::EventObject & event)
    {
    // don't print "CompletedEvent", only print interesting events
    if (!igstk::CompletedEvent().CheckEvent(&event) &&
        !itk::DeleteEvent().CheckEvent(&event) )
      {
      std::cout << event.GetEventName() << std::endl;
      }
    }
};

/** This program tests multiple wired tracker tools */
int igstkPolarisTrackerTest4( int argc, char * argv[] )
{

  igstk::RealTimeClock::Initialize();

  typedef igstk::Object::LoggerType   LoggerType;
  typedef itk::StdStreamLogOutput       LogOutputType;

  if( argc < 4 )
    {
    std::cerr << " Usage: " << argv[0] << "\t" 
                            << "Logger_Output_filename "
                            << "port_number1 "
                            << "port_number2 "
                            << std::endl;
    return EXIT_FAILURE;
    }


  igstk::PolarisTrackerTool::Pointer tool = igstk::PolarisTrackerTool::New();

  igstk::SerialCommunication::Pointer 
                     serialComm = igstk::SerialCommunication::New();

  PolarisTrackerTest4Command::Pointer 
                                my_command = PolarisTrackerTest4Command::New();

  typedef CoordinateReferenceSystemObserver ObserverType;
  typedef ObserverType::EventType CoordinateSystemEventType;

  ObserverType::Pointer coordSystemAObserver = ObserverType::New();
 
  std::string filename = argv[1];
  std::cout << "Logger output saved here:\n";
  std::cout << filename << "\n"; 

  std::ofstream loggerFile;
  loggerFile.open( filename.c_str() );
  LoggerType::Pointer   logger = LoggerType::New();
  LogOutputType::Pointer logOutput = LogOutputType::New();  
  logOutput->SetStream( loggerFile );
  logger->AddLogOutput( logOutput );
  logger->SetPriorityLevel( itk::Logger::DEBUG);

  serialComm->AddObserver( itk::AnyEvent(), my_command);

  serialComm->SetLogger( logger );

  serialComm->SetPortNumber( IGSTK_TEST_POLARIS_PORT_NUMBER );
  serialComm->SetParity( igstk::SerialCommunication::NoParity );
  serialComm->SetBaudRate( igstk::SerialCommunication::BaudRate115200 );
  serialComm->SetDataBits( igstk::SerialCommunication::DataBits8 );
  serialComm->SetStopBits( igstk::SerialCommunication::StopBits1 );
  serialComm->SetHardwareHandshake( igstk::SerialCommunication::HandshakeOff );

  serialComm->SetCaptureFileName( "RecordedStreamByPolarisTrackerTest4.txt" );
  serialComm->SetCapture( true );

  serialComm->OpenCommunication();

  igstk::PolarisTracker::Pointer  tracker;

  tracker = igstk::PolarisTracker::New();

  tracker->AddObserver( itk::AnyEvent(), my_command);

  tracker->SetLogger( logger );

  std::cout << "SetCommunication()" << std::endl;
  tracker->SetCommunication( serialComm );

  std::cout << "RequestOpen()" << std::endl;
  tracker->RequestOpen();

  typedef igstk::PolarisTrackerTool         TrackerToolType;
  typedef TrackerToolType::TransformType    TransformType;

  // instantiate and attach wired tracker tool  
  TrackerToolType::Pointer trackerTool = TrackerToolType::New();
  trackerTool->SetLogger( logger );
  //Select wired tracker tool
  trackerTool->RequestSelectWiredTrackerTool();
  //Set the port number
  unsigned int portNumber1 = atoi(argv[2]); 
  trackerTool->RequestSetPortNumber( portNumber1 );
  //Configure
  trackerTool->RequestConfigure();
  //Attach to the tracker
  trackerTool->RequestAttachToTracker( tracker );
  //Add observer to listen to events throw by the tracker tool
  trackerTool->AddObserver( itk::AnyEvent(), my_command);
  //Add observer to listen to transform events 
  trackerTool->AddObserver( CoordinateSystemEventType(), 
                                  coordSystemAObserver );

  // instantiate and attach a second wired tracker tool  
  TrackerToolType::Pointer trackerTool2 = TrackerToolType::New();
  trackerTool2->SetLogger( logger );
  //Select wired tracker tool
  trackerTool2->RequestSelectWiredTrackerTool();
  //Set the port number
  unsigned int portNumber2 = atoi(argv[3]); 
  trackerTool2->RequestSetPortNumber( portNumber2 );
  //Configure
  trackerTool2->RequestConfigure();
  //Attach to the tracker
  trackerTool2->RequestAttachToTracker( tracker );
  //Add observer to listen to events throw by the tracker tool
  trackerTool2->AddObserver( itk::AnyEvent(), my_command);
  //Add observer to listen to transform events 
  trackerTool2->AddObserver( CoordinateSystemEventType(), 
                                  coordSystemAObserver );



  //start tracking 
  std::cout << "Start tracking..." << std::endl;
  tracker->RequestStartTracking();

  typedef igstk::Transform            TransformType;
  typedef ::itk::Vector<double, 3>    VectorType;
  typedef ::itk::Versor<double>       VersorType;


  for(unsigned int i=0; i<20; i++)
    {
    tracker->RequestUpdateStatus();

    TransformType             transform;
    VectorType                position;


    transform = trackerTool->GetCalibratedTransform();
    position = transform.GetTranslation();
    std::cout << "Trackertool:" << trackerTool->GetTrackerToolIdentifier() 
              << "  Position = (" << position[0]
              << "," << position[1] << "," << position[2]
              << ")" << std::endl;

    transform = trackerTool2->GetCalibratedTransform();
    position = transform.GetTranslation();
    std::cout << "Trackertool:" << trackerTool2->GetTrackerToolIdentifier() 
              << "  Position = (" << position[0]
              << "," << position[1] << "," << position[2]
              << ")" << std::endl;
    }
  
  std::cout << "Stop Tracking" << std::endl;
  tracker->RequestStopTracking();

  // restart tracking
  std::cout << "Restart tracking" << std::endl;
  tracker->RequestStartTracking();

  for(unsigned int i=0; i<20; i++)
    {
    tracker->RequestUpdateStatus();

    TransformType             transform;
    VectorType                position;


    //There are two ways of accessing the transform
    //First option: use GetCalibratedTransform method
    transform = trackerTool->GetCalibratedTransform();

    position = transform.GetTranslation();
    std::cout << "Trackertool:" << trackerTool->GetTrackerToolIdentifier() 
              << "  Position = (" << position[0]
              << "," << position[1] << "," << position[2]
              << ")" << std::endl;

    //Second option: use coordinate system convenience method
    trackerTool->RequestGetTransformToParent();
    if (coordSystemAObserver->GotPayload())
      {
      transform = coordSystemAObserver->GetTransform();
      position = transform.GetTranslation();
      std::cout << "\t\t  Position = (" << position[0]
              << "," << position[1] << "," << position[2]
              << ")" << std::endl;
      }
    }
 
  std::cout << "RequestClose()" << std::endl;
  tracker->RequestClose();

  std::cout << "CloseCommunication()" << std::endl;
  serialComm->CloseCommunication();

  return EXIT_SUCCESS;
}
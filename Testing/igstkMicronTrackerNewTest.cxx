/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkMicronTrackerNewTest.cxx
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
#include "itkLogger.h"
#include "itkStdStreamLogOutput.h"
#include "itkVector.h"
#include "itkVersor.h"

#include "igstkMicronTrackerNew.h"
#include "igstkMicronTrackerTool.h"
#include "igstkTransform.h"

class MicronTrackerNewTrackerTestCommand : public itk::Command 
{
public:
  typedef  MicronTrackerNewTrackerTestCommand   Self;
  typedef  itk::Command                Superclass;
  typedef itk::SmartPointer<Self>      Pointer;
  itkNewMacro( Self );
protected:
  MicronTrackerNewTrackerTestCommand() {};

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


int igstkMicronTrackerNewTest( int argc, char * argv[] )
{

  igstk::RealTimeClock::Initialize();

  typedef itk::Logger                   LoggerType; 
  typedef itk::StdStreamLogOutput       LogOutputType;

  MicronTrackerNewTrackerTestCommand::Pointer 
                                my_command = MicronTrackerNewTrackerTestCommand::New();

  if( argc < 5 )
    {
    std::cerr << " Usage: " << argv[0] << "\t" 
                            << "MicronTracker_Camera_Calibration_file" << "\t"
                            << "MicronTracker_initialization_file"  << "\t"
                            << "Marker_template_directory " << "\t"
                            << "Logger_Output_directory" << std::endl;
    return EXIT_FAILURE;
    }

  std::string filename = argv[4];
  std::cout << "Logger output saved here:\n";
  std::cout << filename << "\n"; 

  std::ofstream loggerFile;
  loggerFile.open( filename.c_str() );
  LoggerType::Pointer   logger = LoggerType::New();
  LogOutputType::Pointer logOutput = LogOutputType::New();  
  logOutput->SetStream( loggerFile );
  logger->AddLogOutput( logOutput );
  logger->SetPriorityLevel( itk::Logger::DEBUG);


  igstk::MicronTrackerNew::Pointer  tracker;

  tracker = igstk::MicronTrackerNew::New();

  tracker->AddObserver( itk::AnyEvent(), my_command);
  tracker->SetLogger( logger );

  // Set necessary parameters of the tracker
  std::string calibrationFilesDirectory = argv[1];
  tracker->SetCameraCalibrationFilesDirectory( 
                            calibrationFilesDirectory );

  std::string initializationFile = argv[2];
  tracker->SetInitializationFile( initializationFile );

  std::string markerTemplateDirectory = argv[3];
  tracker->LoadMarkerTemplate( markerTemplateDirectory );


  // Start communication
  tracker->RequestOpen();

  // Add tracker tools
  typedef igstk::MicronTrackerToolNew  TrackerToolType;

  TrackerToolType::Pointer trackerTool = TrackerToolType::New();
  trackerTool->SetLogger( logger );
  std::string markerNameTT = "TTblock";
  trackerTool->RequestSetMarkerName( markerNameTT );  
  trackerTool->RequestInitialize();
  trackerTool->RequestAttachToTracker( tracker );

  TrackerToolType::Pointer trackerTool2 = TrackerToolType::New();
  trackerTool2->SetLogger( logger );
  std::string markerNamesPointer = "sPointer";
  trackerTool2->RequestSetMarkerName( markerNamesPointer );  
  trackerTool2->RequestInitialize();
  trackerTool2->RequestAttachToTracker( tracker );

  //initialize tracker
  tracker->RequestInitialize();

  //start tracking 
  tracker->RequestStartTracking();

  typedef igstk::Transform            TransformType;
  typedef ::itk::Vector<double, 3>    VectorType;
  typedef ::itk::Versor<double>       VersorType;

  for(unsigned int i=0; i<10; i++)
    {
    tracker->RequestUpdateStatus();

    TransformType             transform;
    VectorType                position;

    tracker->GetToolTransform( 
      trackerTool->GetTrackerToolIdentifier(), transform );

    position = transform.GetTranslation();
    std::cout << "Trackertool:" << trackerTool->GetTrackerToolIdentifier() 
              << "  Position = (" << position[0]
              << "," << position[1] << "," << position[2]
              << ")" << std::endl;

    tracker->GetToolTransform( 
      trackerTool2->GetTrackerToolIdentifier(), transform );

    position = transform.GetTranslation();
    std::cout << "Trackertool:" << trackerTool2->GetTrackerToolIdentifier() 
              << "  Position = (" << position[0]
              << "," << position[1] << "," << position[2]
              << ")" << std::endl;
 
    }
  
  std::cout << "RequestStopTracking()" << std::endl;
  tracker->RequestStopTracking();

  //Remove one of the tracker tools and restart tracking
  std::cout << "Detach the tracker tool from the tracker" << std::endl;
  trackerTool2->RequestDetach( );

  // restart tracking
  tracker->RequestStartTracking();

  for(unsigned int i=0; i<10; i++)
    {
    tracker->RequestUpdateStatus();

    TransformType             transform;
    VectorType                position;

    tracker->GetToolTransform( 
      trackerTool->GetTrackerToolIdentifier(), transform );

    position = transform.GetTranslation();
    std::cout << "Trackertool:" << trackerTool->GetTrackerToolIdentifier() 
              << "  Position = (" << position[0]
              << "," << position[1] << "," << position[2]
              << ")" << std::endl;

    }

  std::cout << "RequestStopTracking()" << std::endl;
  tracker->RequestStopTracking();

  std::cout << "RequestClose()" << std::endl;
  tracker->RequestClose();

  std::cout << "[PASSED]" << std::endl;

  return EXIT_SUCCESS;
}

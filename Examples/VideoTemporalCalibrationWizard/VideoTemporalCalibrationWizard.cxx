/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    VideoTemporalCalibrationWizard.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "VideoTemporalCalibrationWizard.h"

#include "FL/Fl_File_Chooser.H"
#include "FL/Fl_Input.H"

#include "itksys/SystemTools.hxx"
#include "itksys/Directory.hxx"
#include "igstkTransformObserver.h"

#include "igstkMicronTrackerConfiguration.h"
#include "igstkAuroraTrackerConfiguration.h"
#include "igstkPolarisTrackerConfiguration.h"

#include "igstkAuroraConfigurationXMLFileReader.h"
#include "igstkPolarisVicraConfigurationXMLFileReader.h"
#include "igstkPolarisSpectraConfigurationXMLFileReader.h"
#include "igstkPolarisHybridConfigurationXMLFileReader.h"
#include "igstkMicronConfigurationXMLFileReader.h"

#include "igstkVideoTemporalCalibrationIO.h"

#define VIEW_2D_REFRESH_RATE 15
#define VIEW_3D_REFRESH_RATE 15

#define IMAGER_DEFAULT_REFRESH_RATE 10

// name of the tool that is going to drive the reslicing
#define TRACKER_TOOL_NAME "probe" //sPtr // bayonet // hybrid_pointer
#define IMAGER_TOOL_NAME "reference" //sPtr // bayonet // hybrid_pointer


/** -----------------------------------------------------------------
*     Constructor
*  -----------------------------------------------------------------
*/
VideoTemporalCalibrationWizard::VideoTemporalCalibrationWizard() :
  m_StateMachine(this)
{

  std::srand( 5 );

  m_VideoEnabled = false;
  m_CollectorEnabled = false;
  m_ImagerInitialized = false;
  m_VideoRunning = false;
  m_CollectingProbeSamples = false;
  m_CollectingPointerSamples = false;

  m_SnapShotCounter = 1;

  /** Setup logger, for all igstk components. */
  m_Logger   = LoggerType::New();
  this->GetLogger()->SetTimeStampFormat( itk::LoggerBase::HUMANREADABLE );
  this->GetLogger()->SetHumanReadableFormat("%Y %b %d, %H:%M:%S");
  this->GetLogger()->SetPriorityLevel( LoggerType::DEBUG);

  /** Direct the application log message to the std::cout */
  itk::StdStreamLogOutput::Pointer m_LogCoutOutput
                                           = itk::StdStreamLogOutput::New();
  m_LogCoutOutput->SetStream( std::cout );
  this->GetLogger()->AddLogOutput( m_LogCoutOutput );

  /** Direct the igstk components log message to the file. */
  itk::StdStreamLogOutput::Pointer m_LogFileOutput
                                           = itk::StdStreamLogOutput::New();

  std::string   logFileName;
  logFileName = "logVideoTemporalCalibrationWizard"
  + itksys::SystemTools::GetCurrentDateTime( "_%Y_%m_%d_%H_%M_%S" ) + ".txt";
  m_LogFile.open( logFileName.c_str() );

  if( !m_LogFile.fail() )
  {
    m_LogFileOutput->SetStream( m_LogFile );
    this->GetLogger()->AddLogOutput( m_LogFileOutput );
    igstkLogMacro2( m_Logger, DEBUG,
      "Successfully opened Log file:" << logFileName << "\n" );
  }
  else
  {
    //Return if fail to open the log file
    igstkLogMacro2( m_Logger, DEBUG,
      "Problem opening Log file:" << logFileName << "\n" );
    return;
  }

  /** Instantiate the world reference */
  m_WorldReference        = AxesObjectType::New();

  /** Create the controller for the tracker */
  m_TrackerController = igstk::TrackerController::New();

  m_VideoTemporalCalibrationSamples = new igstk::VideoTemporalCalibration;

  m_VideoTemporalCalibrationSamples->m_PointerTransforms.clear();
  m_VideoTemporalCalibrationSamples->m_ProbeTransforms.clear();

  // set logger to the controller
  m_TrackerController->SetLogger(this->GetLogger());

  // instatiate observer for key pressed event
  m_KeyPressedObserver = LoadedObserverType::New();
  m_KeyPressedObserver->SetCallbackFunction( this, &VideoTemporalCalibrationWizard::HandleKeyPressedCallback );

  // add it to the Viewer group in our GUI
  m_ViewerGroup->AddObserver( igstk::VideoTemporalCalibrationWizardQuadrantViews::KeyPressedEvent(),
    m_KeyPressedObserver );

  // instantiate observer for mouse pressed event
  m_MousePressedObserver = LoadedObserverType::New();
  m_MousePressedObserver->SetCallbackFunction( this, &VideoTemporalCalibrationWizard::HandleMousePressedCallback );

  // add it to the Viewer group in our GUI
  m_ViewerGroup->AddObserver( igstk::VideoTemporalCalibrationWizardQuadrantViews::MousePressedEvent(),
    m_MousePressedObserver );

  /** Machine States*/

  igstkAddStateMacro( Initial );
  igstkAddStateMacro( LoadingMesh );
  igstkAddStateMacro( LoadingTrackerToolSpatialObject );
  igstkAddStateMacro( TrackerToolSpatialObjectReady );
  igstkAddStateMacro( ConfiguringTracker );
  igstkAddStateMacro( TrackerConfigurationReady );
  igstkAddStateMacro( InitializingTracker );
  igstkAddStateMacro( TrackerInitializationReady );
  igstkAddStateMacro( StartingTracker );
  igstkAddStateMacro( StoppingTracker );
  igstkAddStateMacro( DisconnectingTracker );
  igstkAddStateMacro( Tracking );
  igstkAddStateMacro( LoadingImagerToolSpatialObject );
  igstkAddStateMacro( TrackingAndImaging );
  igstkAddStateMacro( InitializingImager );
  igstkAddStateMacro( StartingImager );
  igstkAddStateMacro( StoppingImager );
  igstkAddStateMacro( DisconnectingImager );

  /** Machine Inputs*/

  igstkAddInputMacro( Success );
  igstkAddInputMacro( Failure );
  igstkAddInputMacro( LoadTrackerMesh );
  igstkAddInputMacro( LoadWorkingVolumeMesh );
  igstkAddInputMacro( ConfigureTracker );
  igstkAddInputMacro( InitializeTracker );
  igstkAddInputMacro( LoadTrackerToolSpatialObject );
  igstkAddInputMacro( StartTracking );
  igstkAddInputMacro( StopTracking );
  igstkAddInputMacro( DisconnectTracker );
  igstkAddInputMacro( LoadImagerToolSpatialObject );
  igstkAddInputMacro( InitializeImager );
  igstkAddInputMacro( StartImaging );
  igstkAddInputMacro( StopImaging );
  igstkAddInputMacro( DisconnectImager );

  /** Initial State */

  igstkAddTransitionMacro( Initial, LoadTrackerToolSpatialObject,
                           LoadingTrackerToolSpatialObject, LoadTrackerToolSpatialObject );

  //complete table for state: Initial
  igstkAddTransitionMacro( Initial, Success,
                           Initial, ReportInvalidRequest );
  igstkAddTransitionMacro( Initial, Failure,
                           Initial, ReportInvalidRequest );
  igstkAddTransitionMacro( Initial, ConfigureTracker,
                           Initial, ReportInvalidRequest );
  igstkAddTransitionMacro( Initial, LoadWorkingVolumeMesh,
                           Initial, LoadWorkingVolumeMesh );
  igstkAddTransitionMacro( Initial, LoadTrackerMesh,
                           Initial, LoadTrackerMesh );
  igstkAddTransitionMacro( Initial, InitializeTracker,
                           Initial, ReportInvalidRequest );
  igstkAddTransitionMacro( Initial, StartTracking,
                           Initial, ReportInvalidRequest );
  igstkAddTransitionMacro( Initial, StopTracking,
                           Initial, ReportInvalidRequest );
  igstkAddTransitionMacro( Initial, DisconnectTracker,
                           Initial, ReportInvalidRequest );
  igstkAddTransitionMacro( Initial, LoadImagerToolSpatialObject,
                           Initial, ReportInvalidRequest );
  igstkAddTransitionMacro( Initial, InitializeImager,
                           Initial, ReportInvalidRequest );
  igstkAddTransitionMacro( Initial, StartImaging,
                           Initial, ReportInvalidRequest );
  igstkAddTransitionMacro( Initial, StopImaging,
                           Initial, ReportInvalidRequest );
  igstkAddTransitionMacro( Initial, DisconnectImager,
                           Initial, ReportInvalidRequest );

  /** LoadingTrackerToolSpatialObject State */

  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, Success,
                           TrackerToolSpatialObjectReady, ReportSuccessTrackerToolSpatialObjectLoaded );

  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, Failure,
                           Initial, ReportFailureTrackerToolSpatialObjectLoaded );

  //complete table for state: LoadingTrackerToolSpatialObject

  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, LoadWorkingVolumeMesh,
                           LoadingTrackerToolSpatialObject, ReportInvalidRequest );

  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, LoadTrackerMesh,
                           LoadingTrackerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, LoadTrackerToolSpatialObject,
                           LoadingTrackerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, ConfigureTracker,
                           LoadingTrackerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, InitializeTracker,
                           LoadingTrackerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, StartTracking,
                           LoadingTrackerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, StopTracking,
                           LoadingTrackerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, DisconnectTracker,
                           LoadingTrackerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, InitializeImager,
                           LoadingTrackerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, LoadImagerToolSpatialObject,
                           LoadingTrackerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, StartImaging,
                           LoadingTrackerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, StopImaging,
                           LoadingTrackerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingTrackerToolSpatialObject, DisconnectImager,
                           LoadingTrackerToolSpatialObject, ReportInvalidRequest );

  /** TrackerToolSpatialObjectReady State */

  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, ConfigureTracker,
                           ConfiguringTracker, ConfigureTracker );

  //complete table for state: TrackerToolSpatialObjectReady

  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, Success,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, Failure,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, LoadTrackerToolSpatialObject,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, LoadWorkingVolumeMesh,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, LoadTrackerMesh,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, InitializeTracker,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, StartTracking,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, StopTracking,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, DisconnectTracker,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, InitializeImager,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, LoadImagerToolSpatialObject,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, StartImaging,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, StopImaging,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerToolSpatialObjectReady, DisconnectImager,
                           TrackerToolSpatialObjectReady, ReportInvalidRequest );

  /** ConfiguringTracker State */

  igstkAddTransitionMacro( ConfiguringTracker, Success,
                           TrackerConfigurationReady, ReportSuccessTrackerConfiguration );

  igstkAddTransitionMacro( ConfiguringTracker, Failure,
                           Initial, ReportFailureTrackerConfiguration );

  //complete table for state: ConfiguringTracker

  igstkAddTransitionMacro( ConfiguringTracker, LoadTrackerToolSpatialObject,
                           ConfiguringTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( ConfiguringTracker, LoadWorkingVolumeMesh,
                           ConfiguringTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( ConfiguringTracker, LoadTrackerMesh,
                           ConfiguringTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( ConfiguringTracker, ConfigureTracker,
                           ConfiguringTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( ConfiguringTracker, InitializeTracker,
                           ConfiguringTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( ConfiguringTracker, StartTracking,
                           ConfiguringTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( ConfiguringTracker, StopTracking,
                           ConfiguringTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( ConfiguringTracker, DisconnectTracker,
                           ConfiguringTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( ConfiguringTracker, InitializeImager,
                           ConfiguringTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( ConfiguringTracker, LoadImagerToolSpatialObject,
                           ConfiguringTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( ConfiguringTracker, StartImaging,
                           ConfiguringTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( ConfiguringTracker, StopImaging,
                           ConfiguringTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( ConfiguringTracker, DisconnectImager,
                           ConfiguringTracker, ReportInvalidRequest );

  /** TrackerConfigurationReady State */

  igstkAddTransitionMacro( TrackerConfigurationReady, InitializeTracker,
                           InitializingTracker, InitializeTracker );

  //complete table for state: TrackerConfigurationReady
  igstkAddTransitionMacro( TrackerConfigurationReady, Success,
                           TrackerConfigurationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerConfigurationReady, Failure,
                           TrackerConfigurationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerConfigurationReady, LoadTrackerToolSpatialObject,
                           TrackerConfigurationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerConfigurationReady, LoadWorkingVolumeMesh,
                           TrackerConfigurationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerConfigurationReady, LoadTrackerMesh,
                           TrackerConfigurationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerConfigurationReady, ConfigureTracker,
                           TrackerConfigurationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerConfigurationReady, StartTracking,
                           TrackerConfigurationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerConfigurationReady, StopTracking,
                           TrackerConfigurationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerConfigurationReady, DisconnectTracker,
                           TrackerConfigurationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerConfigurationReady, InitializeImager,
                           TrackerConfigurationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerConfigurationReady, LoadImagerToolSpatialObject,
                           TrackerConfigurationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerConfigurationReady, StartImaging,
                           TrackerConfigurationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerConfigurationReady, StopImaging,
                           TrackerConfigurationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerConfigurationReady, DisconnectImager,
                           TrackerConfigurationReady, ReportInvalidRequest );

  /** InitializingTracker State */

  igstkAddTransitionMacro( InitializingTracker, Success,
                           TrackerInitializationReady, ReportSuccessTrackerInitialization );

  igstkAddTransitionMacro( InitializingTracker, Failure,
                           TrackerConfigurationReady, ReportFailureTrackerInitialization );

  //complete table for state: InitializingTracker

  igstkAddTransitionMacro( InitializingTracker, LoadTrackerToolSpatialObject,
                           InitializingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingTracker, LoadWorkingVolumeMesh,
                           InitializingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingTracker, LoadTrackerMesh,
                           InitializingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingTracker, ConfigureTracker,
                           InitializingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingTracker, InitializeTracker,
                           InitializingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingTracker, StartTracking,
                           InitializingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingTracker, StopTracking,
                           InitializingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingTracker, DisconnectTracker,
                           InitializingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingTracker, InitializeImager,
                           InitializingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingTracker, LoadImagerToolSpatialObject,
                           InitializingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingTracker, StartImaging,
                           InitializingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingTracker, StopImaging,
                           InitializingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingTracker, DisconnectImager,
                           InitializingTracker, ReportInvalidRequest );

  /** TrackerInitializationReady State */

  igstkAddTransitionMacro( TrackerInitializationReady, StartTracking,
                           StartingTracker, StartTracking );

  igstkAddTransitionMacro( TrackerInitializationReady, DisconnectTracker,
                           DisconnectingTracker, DisconnectTracker );

  //complete table for state: TrackerInitializationReady

  igstkAddTransitionMacro( TrackerInitializationReady, LoadTrackerToolSpatialObject,
                           TrackerInitializationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerInitializationReady, LoadWorkingVolumeMesh,
                           TrackerInitializationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerInitializationReady, LoadTrackerMesh,
                           TrackerInitializationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerInitializationReady, Success,
                           TrackerInitializationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerInitializationReady, Failure,
                           TrackerInitializationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerInitializationReady, ConfigureTracker,
                           TrackerInitializationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerInitializationReady, InitializeTracker,
                           TrackerInitializationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerInitializationReady, StopTracking,
                           TrackerInitializationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerInitializationReady, InitializeImager,
                           TrackerInitializationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerInitializationReady, LoadImagerToolSpatialObject,
                           TrackerInitializationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerInitializationReady, StartImaging,
                           TrackerInitializationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerInitializationReady, StopImaging,
                           TrackerInitializationReady, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackerInitializationReady, DisconnectImager,
                           TrackerInitializationReady, ReportInvalidRequest );
  /** LoadingMesh State */

  igstkAddTransitionMacro( LoadingMesh, Success,
                           Tracking, ReportSuccessMeshLoaded );

  igstkAddTransitionMacro( LoadingMesh, Failure,
                           Tracking, ReportFailureMeshLoaded );

  //complete table for state: LoadingMesh

  igstkAddTransitionMacro( LoadingMesh, LoadTrackerToolSpatialObject,
                           LoadingMesh, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingMesh, LoadWorkingVolumeMesh,
                           LoadingMesh, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingMesh, LoadTrackerMesh,
                           LoadingMesh, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingMesh, ConfigureTracker,
                           LoadingMesh, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingMesh, InitializeTracker,
                           LoadingMesh, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingMesh, StartTracking,
                           LoadingMesh, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingMesh, StopTracking,
                           LoadingMesh, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingMesh, DisconnectTracker,
                           LoadingMesh, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingMesh, InitializeImager,
                           LoadingMesh, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingMesh, LoadImagerToolSpatialObject,
                           LoadingMesh, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingMesh, StartImaging,
                           LoadingMesh, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingMesh, StopImaging,
                           LoadingMesh, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingMesh, DisconnectImager,
                           LoadingMesh, ReportInvalidRequest );

  /** StartingTracker State*/

  igstkAddTransitionMacro( StartingTracker, Success,
                           Tracking, ReportSuccessStartTracking);

  igstkAddTransitionMacro( StartingTracker, Failure,
                           TrackerInitializationReady, ReportFailureStartTracking );

  //complete table for state: StartingTracker

  igstkAddTransitionMacro( StartingTracker, LoadTrackerToolSpatialObject,
                           StartingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingTracker, LoadWorkingVolumeMesh,
                           StartingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingTracker, LoadTrackerMesh,
                           StartingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingTracker, ConfigureTracker,
                           StartingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingTracker, InitializeTracker,
                           StartingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingTracker, StartTracking,
                           StartingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingTracker, StopTracking,
                           StartingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingTracker, DisconnectTracker,
                           StartingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingTracker, InitializeImager,
                           StartingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingTracker, LoadImagerToolSpatialObject,
                           StartingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingTracker, StartImaging,
                           StartingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingTracker, StopImaging,
                           StartingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingTracker, DisconnectImager,
                           StartingTracker, ReportInvalidRequest );
  /** Tracking State */

  igstkAddTransitionMacro( Tracking, LoadWorkingVolumeMesh,
                           LoadingMesh, LoadWorkingVolumeMesh );
  igstkAddTransitionMacro( Tracking, LoadTrackerMesh,
                           LoadingMesh, LoadTrackerMesh );
  igstkAddTransitionMacro( Tracking, InitializeImager,
                           InitializingImager, InitializeImager );
  igstkAddTransitionMacro( Tracking, StartImaging,
                           StartingImager, StartImaging );
  igstkAddTransitionMacro( Tracking, StopTracking,
                           StoppingTracker, StopTracking );

  //complete table for state: Tracking

  igstkAddTransitionMacro( Tracking, Success,
                           Tracking, ReportInvalidRequest );
  igstkAddTransitionMacro( Tracking, Failure,
                           Tracking, ReportInvalidRequest );
  igstkAddTransitionMacro( Tracking, LoadTrackerToolSpatialObject,
                           Tracking, ReportInvalidRequest );
  igstkAddTransitionMacro( Tracking, InitializeTracker,
                           Tracking, ReportInvalidRequest );
  igstkAddTransitionMacro( Tracking, ConfigureTracker,
                           Tracking, ReportInvalidRequest );
  igstkAddTransitionMacro( Tracking, StartTracking,
                           Tracking, ReportInvalidRequest );
  igstkAddTransitionMacro( Tracking, DisconnectTracker,
                           Tracking, ReportInvalidRequest );
  igstkAddTransitionMacro( Tracking, LoadImagerToolSpatialObject,
                           Tracking, ReportInvalidRequest );
  igstkAddTransitionMacro( Tracking, StopImaging,
                           Tracking, ReportInvalidRequest );
  igstkAddTransitionMacro( Tracking, DisconnectImager,
                           Tracking, ReportInvalidRequest );

   /** StoppingTracker State */

  igstkAddTransitionMacro( StoppingTracker, Success,
                           TrackerInitializationReady, ReportSuccessStopTracking );

  igstkAddTransitionMacro( StoppingTracker, Failure,
                           Tracking, ReportFailureStopTracking );


  //complete table for state: StoppingTracker

  igstkAddTransitionMacro( StoppingTracker, LoadTrackerToolSpatialObject,
                           StoppingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingTracker, LoadWorkingVolumeMesh,
                           StoppingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingTracker, LoadTrackerMesh,
                           StoppingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingTracker, InitializeTracker,
                           StoppingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingTracker, ConfigureTracker,
                           StoppingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingTracker, StartTracking,
                           StoppingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingTracker, StopTracking,
                           StoppingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingTracker, DisconnectTracker,
                           StoppingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingTracker, InitializeImager,
                           StoppingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingTracker, LoadImagerToolSpatialObject,
                           StoppingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingTracker, StartImaging,
                           StoppingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingTracker, StopImaging,
                           StoppingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingTracker, DisconnectImager,
                           StoppingTracker, ReportInvalidRequest );

   /** DisconnectingTracker Tracker */

  igstkAddTransitionMacro( DisconnectingTracker, Success,
                           TrackerInitializationReady, ReportSuccessTrackerDisconnection);

  igstkAddTransitionMacro( DisconnectingTracker, Failure,
                           TrackerInitializationReady, ReportFailureTrackerDisconnection);

  //complete table for state: DisconnectingTracker

  igstkAddTransitionMacro( DisconnectingTracker, LoadTrackerToolSpatialObject,
                           DisconnectingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingTracker, LoadWorkingVolumeMesh,
                           DisconnectingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingTracker, LoadTrackerMesh,
                           DisconnectingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingTracker, InitializeTracker,
                           DisconnectingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingTracker, ConfigureTracker,
                           DisconnectingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingTracker, StartTracking,
                           DisconnectingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingTracker, StopTracking,
                           DisconnectingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingTracker, DisconnectTracker,
                           DisconnectingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingTracker, LoadImagerToolSpatialObject,
                           DisconnectingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingTracker, InitializeImager,
                           DisconnectingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingTracker, StartImaging,
                           DisconnectingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingTracker, StopImaging,
                           DisconnectingTracker, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingTracker, DisconnectImager,
                           DisconnectingTracker, ReportInvalidRequest );

    /** LoadingImagerToolSpatialObject State */

  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, Success,
                           Tracking, ReportSuccessImagerToolSpatialObjectLoaded );

  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, Failure,
                           Tracking, ReportFailureImagerToolSpatialObjectLoaded );

  //complete table for state: LoadingImagerToolSpatialObject

  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, LoadWorkingVolumeMesh,
                           LoadingImagerToolSpatialObject, LoadWorkingVolumeMesh );
  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, LoadTrackerToolSpatialObject,
                           LoadingImagerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, LoadImagerToolSpatialObject,
                           LoadingImagerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, ConfigureTracker,
                           LoadingImagerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, InitializeTracker,
                           LoadingImagerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, StartTracking,
                           LoadingImagerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, StopTracking,
                           LoadingImagerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, DisconnectTracker,
                           LoadingImagerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, StartImaging,
                           LoadingImagerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, StopImaging,
                           LoadingImagerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, DisconnectImager,
                           LoadingImagerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, InitializeImager,
                           LoadingImagerToolSpatialObject, ReportInvalidRequest );
  igstkAddTransitionMacro( LoadingImagerToolSpatialObject, LoadTrackerMesh,
                           LoadingImagerToolSpatialObject, LoadTrackerMesh );

  /** InitializingImager State */

  igstkAddTransitionMacro( InitializingImager, Success,
                           Tracking, ReportSuccessImagerInitialization );

  igstkAddTransitionMacro( InitializingImager, Failure,
                           Tracking, ReportFailureImagerInitialization );

  //complete table for state: InitializingImager

  igstkAddTransitionMacro( InitializingImager, LoadWorkingVolumeMesh,
                           InitializingImager, LoadWorkingVolumeMesh );
  igstkAddTransitionMacro( InitializingImager, LoadTrackerToolSpatialObject,
                           InitializingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingImager, LoadImagerToolSpatialObject,
                           InitializingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingImager, ConfigureTracker,
                           InitializingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingImager, InitializeTracker,
                           InitializingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingImager, StartTracking,
                           InitializingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingImager, StopTracking,
                           InitializingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingImager, DisconnectTracker,
                           InitializingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingImager, StartImaging,
                           InitializingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingImager, StopImaging,
                           InitializingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingImager, DisconnectImager,
                           InitializingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingImager, InitializeImager,
                           InitializingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( InitializingImager, LoadTrackerMesh,
                           InitializingImager, LoadTrackerMesh );

  /** StartingImager State*/

  igstkAddTransitionMacro( StartingImager, Success,
                           TrackingAndImaging, ReportSuccessStartImaging );

  igstkAddTransitionMacro( StartingImager, Failure,
                           Tracking, ReportFailureStartImaging );

  //complete table for state: StartingImager

  igstkAddTransitionMacro( StartingImager, LoadWorkingVolumeMesh,
                           StartingImager, LoadWorkingVolumeMesh );
  igstkAddTransitionMacro( StartingImager, LoadTrackerToolSpatialObject,
                           StartingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingImager, LoadImagerToolSpatialObject,
                           StartingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingImager, ConfigureTracker,
                           StartingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingImager, InitializeTracker,
                           StartingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingImager, StartTracking,
                           StartingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingImager, StopTracking,
                           StartingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingImager, DisconnectTracker,
                           StartingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingImager, StartImaging,
                           StartingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingImager, StopImaging,
                           StartingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingImager, DisconnectImager,
                           StartingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingImager, InitializeImager,
                           StartingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StartingImager, LoadTrackerMesh,
                           StartingImager, LoadTrackerMesh );

  /** TrackingAndImagingAndImaging State */

  igstkAddTransitionMacro( TrackingAndImaging, StopImaging,
                           StoppingImager, StopImaging );

  //complete table for state: TrackingAndImaging

  igstkAddTransitionMacro( TrackingAndImaging, Success,
                           TrackingAndImaging, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackingAndImaging, Failure,
                           TrackingAndImaging, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackingAndImaging, LoadWorkingVolumeMesh,
                           TrackingAndImaging, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackingAndImaging, LoadTrackerToolSpatialObject,
                           TrackingAndImaging, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackingAndImaging, LoadImagerToolSpatialObject,
                           TrackingAndImaging, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackingAndImaging, InitializeTracker,
                           TrackingAndImaging, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackingAndImaging, ConfigureTracker,
                           TrackingAndImaging, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackingAndImaging, StartTracking,
                           TrackingAndImaging, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackingAndImaging, StopTracking,
                           TrackingAndImaging, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackingAndImaging, DisconnectTracker,
                           TrackingAndImaging, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackingAndImaging, StartImaging,
                           TrackingAndImaging, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackingAndImaging, InitializeImager,
                           TrackingAndImaging, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackingAndImaging, DisconnectImager,
                           TrackingAndImaging, ReportInvalidRequest );
  igstkAddTransitionMacro( TrackingAndImaging, LoadTrackerMesh,
                           TrackingAndImaging, LoadTrackerMesh );

  /** StoppingImager State */

  igstkAddTransitionMacro( StoppingImager, Success,
                           Tracking, ReportSuccessStopImaging);

  igstkAddTransitionMacro( StoppingImager, Failure,
                           TrackingAndImaging, ReportFailureStopImaging );

  //complete table for state: StoppingImager

  igstkAddTransitionMacro( StoppingImager, LoadTrackerToolSpatialObject,
                           StoppingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingImager, LoadWorkingVolumeMesh,
                           StoppingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingImager, InitializeTracker,
                           StoppingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingImager, ConfigureTracker,
                           StoppingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingImager, StartTracking,
                           StoppingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingImager, StopTracking,
                           StoppingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingImager, DisconnectTracker,
                           StoppingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingImager, LoadImagerToolSpatialObject,
                           StoppingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingImager, StartImaging,
                           StoppingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingImager, StopImaging,
                           StoppingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingImager, DisconnectImager,
                           StoppingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingImager, InitializeImager,
                           StoppingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( StoppingImager, LoadTrackerMesh,
                           StoppingImager, LoadTrackerMesh );

  /** DisconnectingImager */

  igstkAddTransitionMacro( DisconnectingImager, Success,
                           Tracking, ReportSuccessImagerDisconnection);

  igstkAddTransitionMacro( DisconnectingImager, Failure,
                           Tracking, ReportFailureTrackerDisconnection );

  //complete table for state: DisconnectingImager

  igstkAddTransitionMacro( DisconnectingImager, LoadWorkingVolumeMesh,
                           DisconnectingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingImager, LoadTrackerToolSpatialObject,
                           DisconnectingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingImager, InitializeTracker,
                           DisconnectingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingImager, ConfigureTracker,
                           DisconnectingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingImager, StartTracking,
                           DisconnectingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingImager, StopTracking,
                           DisconnectingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingImager, DisconnectTracker,
                           DisconnectingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingImager, LoadImagerToolSpatialObject,
                           DisconnectingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingImager, StartImaging,
                           DisconnectingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingImager, StopImaging,
                           DisconnectingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingImager, DisconnectImager,
                           DisconnectingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingImager, InitializeImager,
                           DisconnectingImager, ReportInvalidRequest );
  igstkAddTransitionMacro( DisconnectingImager, LoadTrackerMesh,
                           DisconnectingImager, LoadTrackerMesh );

  /** Set Initial State */

  igstkSetInitialStateMacro( Initial );

  m_StateMachine.SetReadyToRun();
/*
  std::ofstream ofile;
  ofile.open("VideoTemporalCalibrationWizardStateMachineDiagram.dot");
  const bool skipLoops = false;
  this->ExportStateMachineDescription( ofile, skipLoops );
  ofile.close();
*/
}

/** -----------------------------------------------------------------
*     Destructor
*  -----------------------------------------------------------------
*/
VideoTemporalCalibrationWizard::~VideoTemporalCalibrationWizard()
{
  delete m_VideoTemporalCalibrationSamples;
}

bool VideoTemporalCalibrationWizard::ReadPolarisVicraConfiguration(igstk::TrackerConfigurationFileReader::Pointer reader)
{

  igstkLogMacro2( m_Logger, DEBUG,
             "VideoTemporalCalibrationWizard::ReadPolarisVicraConfiguration called...\n" )

  igstk::TrackerConfigurationXMLFileReaderBase::Pointer
                                                        trackerCofigurationXMLReader;

  trackerCofigurationXMLReader = igstk::PolarisVicraConfigurationXMLFileReader::New();

  //setting the file name and reader always succeeds so I don't
             //observe the trackerConfigReader for their success events
 // trackerConfigReader->RequestSetFileName( TRACKER_CONFIGURATION_XML );
  reader->RequestSetReader( trackerCofigurationXMLReader );

   //need to observe if the request read succeeds or fails
   //there is a third option that the read is invalid, if the
   //file name or xml reader weren't set
  ReadTrackerConfigurationFailSuccessObserverType::Pointer trackerReaderObserver =
                                ReadTrackerConfigurationFailSuccessObserverType::New();

  reader->AddObserver( igstk::TrackerConfigurationFileReader::ReadSuccessEvent(),
                                    trackerReaderObserver );
  reader->AddObserver( igstk::TrackerConfigurationFileReader::ReadFailureEvent(),
                                    trackerReaderObserver );
  reader->RequestRead();

  if( trackerReaderObserver->GotFailure() )
  {
      igstkLogMacro2( m_Logger, DEBUG,
        "VideoTemporalCalibrationWizard::ReadPolarisVicraConfiguration error: "\
        << trackerReaderObserver->GetFailureMessage() << "\n" )
      return false;
  }

  if( !trackerReaderObserver->GotSuccess() )
  {
     igstkLogMacro2( m_Logger, DEBUG,
        "VideoTemporalCalibrationWizard::ReadPolarisVicraConfiguration error: could not read Polaris Vicra tracker configuration file\n")
     return false;
  }

  //get the configuration data from the reader
  TrackerConfigurationObserver::Pointer trackerConfigurationObserver =
    TrackerConfigurationObserver::New();

  reader->AddObserver(
    igstk::TrackerConfigurationFileReader::TrackerConfigurationDataEvent(), trackerConfigurationObserver );

  reader->RequestGetData();

  if( !trackerConfigurationObserver->GotTrackerConfiguration() )
  {
     igstkLogMacro2( m_Logger, DEBUG,
        "VideoTemporalCalibrationWizard::ReadPolarisVicraConfiguration error: could not get Polaris Vicra tracker configuration\n")
     return false;
  }

  m_TrackerConfiguration = trackerConfigurationObserver->GetTrackerConfiguration();

  return true;
}

bool VideoTemporalCalibrationWizard::ReadPolarisHybridConfiguration(igstk::TrackerConfigurationFileReader::Pointer reader)
{
  igstkLogMacro2( m_Logger, DEBUG,
             "VideoTemporalCalibrationWizard::ReadPolarisHybridConfiguration called...\n" )

  igstk::TrackerConfigurationXMLFileReaderBase::Pointer
                                                        trackerCofigurationXMLReader;

  trackerCofigurationXMLReader = igstk::PolarisHybridConfigurationXMLFileReader::New();

  //setting the file name and reader always succeeds so I don't
             //observe the trackerConfigReader for their success events
 // trackerConfigReader->RequestSetFileName( TRACKER_CONFIGURATION_XML );
  reader->RequestSetReader( trackerCofigurationXMLReader );

   //need to observe if the request read succeeds or fails
   //there is a third option that the read is invalid, if the
   //file name or xml reader weren't set
  ReadTrackerConfigurationFailSuccessObserverType::Pointer trackerReaderObserver =
                                ReadTrackerConfigurationFailSuccessObserverType::New();

  reader->AddObserver( igstk::TrackerConfigurationFileReader::ReadSuccessEvent(),
                                    trackerReaderObserver );
  reader->AddObserver( igstk::TrackerConfigurationFileReader::ReadFailureEvent(),
                                    trackerReaderObserver );
  reader->RequestRead();

  if( trackerReaderObserver->GotFailure() )
  {
      igstkLogMacro2( m_Logger, DEBUG,
        "Navigator::ReadPolarisHybridConfiguration error: "\
        << trackerReaderObserver->GetFailureMessage() << "\n" )
      return false;
  }

  if( !trackerReaderObserver->GotSuccess() )
  {
     igstkLogMacro2( m_Logger, DEBUG,
        "Navigator::ReadPolarisHybridConfiguration error: could not read Polaris Hybrid tracker configuration file\n")
     return false;
  }

  //get the configuration data from the reader
  TrackerConfigurationObserver::Pointer trackerConfigurationObserver =
    TrackerConfigurationObserver::New();

  reader->AddObserver(
    igstk::TrackerConfigurationFileReader::TrackerConfigurationDataEvent(), trackerConfigurationObserver );

  reader->RequestGetData();

  if( !trackerConfigurationObserver->GotTrackerConfiguration() )
  {
     igstkLogMacro2( m_Logger, DEBUG,
        "Navigator::ReadAuroraConfiguration error: could not get Polaris Hybrid tracker configuration\n")
     return false;
  }

  m_TrackerConfiguration = trackerConfigurationObserver->GetTrackerConfiguration();

  return true;
}


bool VideoTemporalCalibrationWizard::ReadMicronConfiguration(igstk::TrackerConfigurationFileReader::Pointer reader)
{
  igstk::TrackerConfigurationXMLFileReaderBase::Pointer trackerCofigurationXMLReader;

  trackerCofigurationXMLReader = igstk::MicronConfigurationXMLFileReader::New();

  reader->RequestSetReader( trackerCofigurationXMLReader );

   //need to observe if the request read succeeds or fails
   //there is a third option that the read is invalid, if the
   //file name or xml reader weren't set
  ReadTrackerConfigurationFailSuccessObserverType::Pointer trackerReaderObserver =
                                ReadTrackerConfigurationFailSuccessObserverType::New();

  reader->AddObserver( igstk::TrackerConfigurationFileReader::ReadSuccessEvent(),
                                    trackerReaderObserver );
  reader->AddObserver( igstk::TrackerConfigurationFileReader::ReadFailureEvent(),
                                    trackerReaderObserver );
  reader->RequestRead();

  if( trackerReaderObserver->GotFailure() )
  {
      igstkLogMacro2( m_Logger, DEBUG,
        "VideoTemporalCalibrationWizard::ReadMicronConfiguration error: "\
        << trackerReaderObserver->GetFailureMessage() << "\n" )
      return false;
  }

  if( !trackerReaderObserver->GotSuccess() )
  {
     igstkLogMacro2( m_Logger, DEBUG,
        "VideoTemporalCalibrationWizard::ReadMicronConfiguration error: could not read MICRON tracker configuration file\n")
     return false;
  }

  //get the configuration data from the reader
  TrackerConfigurationObserver::Pointer trackerConfigurationObserver =
    TrackerConfigurationObserver::New();

  reader->AddObserver(
    igstk::TrackerConfigurationFileReader::TrackerConfigurationDataEvent(), trackerConfigurationObserver );

  reader->RequestGetData();

  if( !trackerConfigurationObserver->GotTrackerConfiguration() )
  {
     igstkLogMacro2( m_Logger, DEBUG,
        "VideoTemporalCalibrationWizard::ReadMicronConfiguration error: could not get MICRON tracker configuration\n")
     return false;
  }

  m_TrackerConfiguration = trackerConfigurationObserver->GetTrackerConfiguration();

  return true;
}

bool VideoTemporalCalibrationWizard::ReadAuroraConfiguration(igstk::TrackerConfigurationFileReader::Pointer reader)
{

  igstk::TrackerConfigurationXMLFileReaderBase::Pointer
                                                        trackerCofigurationXMLReader;

  trackerCofigurationXMLReader = igstk::AuroraConfigurationXMLFileReader::New();

  //setting the file name and reader always succeeds so I don't
             //observe the trackerConfigReader for their success events
 // trackerConfigReader->RequestSetFileName( TRACKER_CONFIGURATION_XML );
  reader->RequestSetReader( trackerCofigurationXMLReader );

   //need to observe if the request read succeeds or fails
   //there is a third option that the read is invalid, if the
   //file name or xml reader weren't set
  ReadTrackerConfigurationFailSuccessObserverType::Pointer trackerReaderObserver =
                                ReadTrackerConfigurationFailSuccessObserverType::New();

  reader->AddObserver( igstk::TrackerConfigurationFileReader::ReadSuccessEvent(),
                                    trackerReaderObserver );
  reader->AddObserver( igstk::TrackerConfigurationFileReader::ReadFailureEvent(),
                                    trackerReaderObserver );
  reader->RequestRead();

  if( trackerReaderObserver->GotFailure() )
  {
      igstkLogMacro2( m_Logger, DEBUG,
        "VideoTemporalCalibrationWizard::ReadAuroraConfiguration error: "\
        << trackerReaderObserver->GetFailureMessage() << "\n" )
      return false;
  }

  if( !trackerReaderObserver->GotSuccess() )
  {
     igstkLogMacro2( m_Logger, DEBUG,
        "VideoTemporalCalibrationWizard::ReadAuroraConfiguration error: could not read AURORA tracker configuration file\n")
     return false;
  }

  //get the configuration data from the reader
  TrackerConfigurationObserver::Pointer trackerConfigurationObserver =
    TrackerConfigurationObserver::New();

  reader->AddObserver(
    igstk::TrackerConfigurationFileReader::TrackerConfigurationDataEvent(), trackerConfigurationObserver );

  reader->RequestGetData();

  if( !trackerConfigurationObserver->GotTrackerConfiguration() )
  {
     igstkLogMacro2( m_Logger, DEBUG,
        "VideoTemporalCalibrationWizard::ReadAuroraConfiguration error: could not get AURORA tracker configuration\n")
     return false;
  }

  m_TrackerConfiguration = trackerConfigurationObserver->GetTrackerConfiguration();

  return true;
}

void VideoTemporalCalibrationWizard::ConfigureTrackerProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG,
             "VideoTemporalCalibrationWizard::ConfigureTrackerProcessing called...\n" )

  const char*  fileName =
    fl_file_chooser("Select a tracker configuration file","*.xml", "auroraConfiguration.xml");

  if ( fileName == NULL )
  {
      igstkLogMacro2( m_Logger, DEBUG,
             "VideoTemporalCalibrationWizard::ConfigureTrackerProcessing none file was selected or operation canceled...\n" )
      m_StateMachine.PushInput( m_FailureInput );
      m_StateMachine.ProcessInputs();
      return;
  }

  igstk::TrackerConfigurationFileReader::Pointer trackerConfigReader =
    igstk::TrackerConfigurationFileReader::New();
    //setting the file name and reader always succeeds so I don't
             //observe the trackerConfigReader for their success events
  trackerConfigReader->RequestSetFileName( fileName );

//  if ( this->ReadAuroraConfiguration( trackerConfigReader ) )
//  {
//    m_StateMachine.PushInput( m_SuccessInput );
//  }
//  else if ( this->ReadMicronConfiguration( trackerConfigReader ) )
//  {
//    m_StateMachine.PushInput( m_SuccessInput );
//  }
//  else if ( this->ReadPolarisVicraConfiguration( trackerConfigReader ) )
//  {
//    m_StateMachine.PushInput( m_SuccessInput );
//  }
//  else
  if ( this->ReadPolarisHybridConfiguration( trackerConfigReader ) )
  {
      m_StateMachine.PushInput( m_SuccessInput );
  }
  else
  {
    m_StateMachine.PushInput( m_FailureInput );
  }

  m_StateMachine.ProcessInputs();
}

void VideoTemporalCalibrationWizard::RequestLoadTrackerMesh()
{
  igstkLogMacro2( m_Logger, DEBUG,
             "VideoTemporalCalibrationWizard::RequestLoadTrackerMesh called...\n" )
  m_StateMachine.PushInput( m_LoadTrackerMeshInput );
  m_StateMachine.ProcessInputs();
}

void VideoTemporalCalibrationWizard::RequestLoadWorkingVolumeMesh()
{
  igstkLogMacro2( m_Logger, DEBUG,
             "VideoTemporalCalibrationWizard::RequestLoadWorkingVolumeMesh called...\n" )
  m_StateMachine.PushInput( m_LoadWorkingVolumeMeshInput );
  m_StateMachine.ProcessInputs();
}

void VideoTemporalCalibrationWizard::RequestConfigureTracker()
{
  igstkLogMacro2( m_Logger, DEBUG,
             "VideoTemporalCalibrationWizard::RequestConfigureTracker called...\n" )

  m_StateMachine.PushInput( m_ConfigureTrackerInput );
  m_StateMachine.ProcessInputs();
}

/** -----------------------------------------------------------------
* Disconnects the imager and closes the socket
*---------------------------------------------------------------------
*/
void
VideoTemporalCalibrationWizard::DisconnectImagerProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG,
           "VideoTemporalCalibrationWizard::DisconnectImagerProcessing called...\n" )

  // try to disconnect
  m_ImagerController->RequestShutdown( );

  //check if succeded
  if( m_ImagerControllerObserver->Error() )
  {
    std::string errorMessage;
    m_ImagerControllerObserver->GetErrorMessage( errorMessage );
    m_ImagerControllerObserver->ClearError();
    fl_alert( errorMessage.c_str() );
    fl_beep( FL_BEEP_ERROR );
    igstkLogMacro2( m_Logger, DEBUG, "Imager disconnect error\n" )
    m_StateMachine.PushInput( m_FailureInput );
  }
  else
  {
    m_StateMachine.PushInput( m_SuccessInput );
  }

  m_StateMachine.ProcessInputs();
}

void
VideoTemporalCalibrationWizard::InitializeImagerProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG,
      "VideoTemporalCalibrationWizard::InitializeImagerProcessing called...\n" )

  m_ImagerConfiguration = new igstk::ImagingSourceImagerConfiguration();

  //set the tool parameters
  igstk::ImagingSourceToolConfiguration toolconfig;

  unsigned int dims[3];
  dims[0] = 640;
  dims[1] = 480;
  dims[2] = 2; //2 Byte 16 bit/8 in ImagingSource converter

  toolconfig.SetFrameDimensions(dims);
  toolconfig.SetPixelDepth(8);
  std::string deviceName = "Camera";
//  toolconfig.SetCalibrationFileName( "Terason2000_Calibration.igstk" );
  toolconfig.SetToolUniqueIdentifier( deviceName );

  m_ImagerConfiguration->RequestAddTool( &toolconfig );

  m_ImagerConfiguration->RequestSetFrequency( IMAGER_DEFAULT_REFRESH_RATE );

  if (!m_ImagerConfiguration)
  {
    std::string errorMessage;
    fl_alert( errorMessage.c_str() );
    fl_beep( FL_BEEP_ERROR );
    igstkLogMacro2( m_Logger, DEBUG, "Imager Initialization error\n" )
    m_StateMachine.PushInput( m_FailureInput );
    m_StateMachine.ProcessInputs();
    return;
  }

  /** Create the controller for the imager and assign observers to it*/
  m_ImagerController = igstk::ImagerController::New();

  m_ImagerControllerObserver = ImagerControllerObserver::New();
  m_ImagerControllerObserver->SetParent( this );

  m_ImagerController->AddObserver(igstk::ImagerController::InitializeFailureEvent(),
    m_ImagerControllerObserver );

  m_ImagerController->AddObserver(igstk::ImagerController::StartFailureEvent(),
    m_ImagerControllerObserver );

  m_ImagerController->AddObserver(igstk::ImagerController::StopFailureEvent(),
    m_ImagerControllerObserver );

  m_ImagerController->AddObserver(igstk::ImagerController::ShutdownFailureEvent(),
    m_ImagerControllerObserver );

  m_ImagerController->AddObserver(igstk::ImagerController::RequestImagerEvent(),
    m_ImagerControllerObserver );

  m_ImagerController->AddObserver(igstk::ImagerController::RequestToolsEvent(),
    m_ImagerControllerObserver );

  // Initialize the progress command
  m_SocketProgressCommand = ProgressCommandType::New();
  m_SocketProgressCommand->SetCallbackFunction( this, &VideoTemporalCalibrationWizard::OnSocketProgressEvent );

  // Provide a progress observer to the imager controller
  m_ImagerController->RequestSetProgressCallback( m_SocketProgressCommand );

  // initialize the imager controller with our image configuration file
  m_ImagerController->RequestInitialize( m_ImagerConfiguration );

  //check that initialization was successful
  if( m_ImagerControllerObserver->Error() )
  {
    std::string errorMessage;
    m_ImagerControllerObserver->GetErrorMessage( errorMessage );
    m_ImagerControllerObserver->ClearError();
    fl_alert( errorMessage.c_str() );
    fl_beep( FL_BEEP_ERROR );
    igstkLogMacro2( m_Logger, DEBUG, "Imager Initialization error\n" )
    m_StateMachine.PushInput( m_FailureInput );
    m_StateMachine.ProcessInputs();
    return;
  }

  m_ImagerController->RequestGetImager();
  m_ImagerController->RequestGetToolList();

  igstk::Frame frame;
  m_ImagingSourceImagerTool->SetInternalFrame(frame);

  m_VideoFrame->SetImagerTool(m_ImagingSourceImagerTool);

  m_StateMachine.PushInput( m_SuccessInput );
  m_StateMachine.ProcessInputs();
}

void
VideoTemporalCalibrationWizard::RequestInitializeTracker()
{
  igstkLogMacro2( m_Logger, DEBUG,
             "VideoTemporalCalibrationWizard::RequestInitializeTracker called...\n" )

  m_StateMachine.PushInput( m_InitializeTrackerInput );
  m_StateMachine.ProcessInputs();
}

void
VideoTemporalCalibrationWizard::RequestInitializeImager()
{
  igstkLogMacro2( m_Logger, DEBUG,
             "VideoTemporalCalibrationWizard::RequestInitializeImager called...\n" )

  m_StateMachine.PushInput( m_InitializeImagerInput );
  m_StateMachine.ProcessInputs();
}

void
VideoTemporalCalibrationWizard::RequestStartTracking()
{
  igstkLogMacro2( m_Logger, DEBUG,
                    "VideoTemporalCalibrationWizard::RequestStartTracking called...\n" )

  m_StateMachine.PushInput( m_StartTrackingInput );
  m_StateMachine.ProcessInputs();
}

void
VideoTemporalCalibrationWizard::RequestStartImaging()
{
  igstkLogMacro2( m_Logger, DEBUG,
                    "VideoTemporalCalibrationWizard::RequestStartImaging called...\n" )

  m_StateMachine.PushInput( m_StartImagingInput );
  m_StateMachine.ProcessInputs();
}

void
VideoTemporalCalibrationWizard::RequestDisconnectImager()
{
  igstkLogMacro2( m_Logger, DEBUG,
                    "VideoTemporalCalibrationWizard::RequestDisconnectImager called...\n" )

  m_StateMachine.PushInput( m_DisconnectImagerInput );
  m_StateMachine.ProcessInputs();
}

/** -----------------------------------------------------------------
* Starts imaging provided it is initialized and connected to the
* communication port
*---------------------------------------------------------------------
*/
void
VideoTemporalCalibrationWizard::StartImagingProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG,
                    "VideoTemporalCalibrationWizard::StartImagingProcessing called...\n" )

  m_ImagerController->RequestStart();

  //check if succeded
  if( m_ImagerControllerObserver->Error() )
  {
    std::string errorMessage;
    m_ImagerControllerObserver->GetErrorMessage( errorMessage );
    m_ImagerControllerObserver->ClearError();
    fl_alert( errorMessage.c_str() );
    fl_beep( FL_BEEP_ERROR );
    igstkLogMacro2( m_Logger, DEBUG, "Imager start error\n" )
    m_StateMachine.PushInput( m_FailureInput );
    m_StateMachine.ProcessInputs();
    return;
  }

  m_StateMachine.PushInput( m_SuccessInput );
  m_StateMachine.ProcessInputs();
}

/** Method to be invoked on successful imager start */
void
VideoTemporalCalibrationWizard::ReportSuccessStartImagingProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportSuccessStartImagingProcessing called...\n")

  m_ToggleRunVideoButton->label("Stop");
  m_VideoRunning = true;
}

/** Method to be invoked on Failure imager start */
void
VideoTemporalCalibrationWizard::ReportFailureStartImagingProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportFailureStartImagingProcessing called...\n")
}

void
VideoTemporalCalibrationWizard::RequestStopTracking()
{
  igstkLogMacro2( m_Logger, DEBUG,
                    "VideoTemporalCalibrationWizard::RequestStopTracking called...\n" )

  m_StateMachine.PushInput( m_StopTrackingInput );
  m_StateMachine.ProcessInputs();
}

void
VideoTemporalCalibrationWizard::RequestStopImaging()
{
  igstkLogMacro2( m_Logger, DEBUG,
     "VideoTemporalCalibrationWizard::RequestStopImaging called...\n" )

  m_StateMachine.PushInput( m_StopImagingInput );
  m_StateMachine.ProcessInputs();
}

void
VideoTemporalCalibrationWizard::RequestDisconnectTracker()
{
  igstkLogMacro2( m_Logger, DEBUG,
                    "VideoTemporalCalibrationWizard::RequestDisconnectTracker called...\n" )

  m_StateMachine.PushInput( m_DisconnectTrackerInput );
  m_StateMachine.ProcessInputs();
}


/** Method to be invoked when an invalid operation was requested */
void
VideoTemporalCalibrationWizard::ReportInvalidRequestProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportInvalidRequestProcessing called...\n");
  this->InvokeEvent(InvalidRequestErrorEvent());
}

/** Method to be invoked on successful imager tool spatial object loading */
void
VideoTemporalCalibrationWizard::ReportSuccessImagerToolSpatialObjectLoadedProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportSuccessImagerToolSpatialObjectLoadedProcessing called...\n");

//  this->RequestConfigureImager();
}


/** Method to be invoked on Failure imager initialization */
void
VideoTemporalCalibrationWizard::ReportFailureImagerInitializationProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportFailureImagerInitializationProcessing called...\n");

  std::string errorMessage;
  errorMessage = "Could not initialize imager device";
  fl_alert( errorMessage.c_str() );
  fl_beep( FL_BEEP_ERROR );

  m_ImagerInitialized = false;
}


/** Method to be invoked on successful imager initialization */
void
VideoTemporalCalibrationWizard::ReportSuccessImagerInitializationProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportSuccessImagerInitializationProcessing called...\n");
  m_ImagerInitialized = true;
}

/** Method to be invoked on successful mesh loading */
void
VideoTemporalCalibrationWizard::ReportSuccessMeshLoadedProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportSuccessMeshLoadedProcessing called...\n");
}

/** Method to be invoked on Failure mesh loading */
void
VideoTemporalCalibrationWizard::ReportFailureMeshLoadedProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportFailureMeshLoadedProcessing called...\n");
}


/** Method to be invoked on successful tracker configuration */
void
VideoTemporalCalibrationWizard::ReportSuccessTrackerConfigurationProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportSuccessTrackerConfigurationProcessing called...\n");

  this->RequestInitializeTracker();
}

/** Method to be invoked on Failure tracker configuration */
void
VideoTemporalCalibrationWizard::ReportFailureTrackerConfigurationProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportFailureTrackerConfigurationProcessing called...\n");
}

/** Method to be invoked on Failure tracker initialization */
void
VideoTemporalCalibrationWizard::ReportFailureTrackerInitializationProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportFailureTrackerConfigurationProcessing called...\n");
}


/** Method to be invoked on successful tracker initialization */
void
VideoTemporalCalibrationWizard::ReportSuccessTrackerInitializationProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportSuccessTrackerInitializationProcessing called...\n");

  this->RequestStartTracking();
}

/** Method to be invoked on failure tracker disconnection */
void
VideoTemporalCalibrationWizard::ReportFailureTrackerDisconnectionProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportFailureTrackerDisconnectionProcessing called...\n");
}

/** Method to be invoked on successful tracker disconnection */
void
VideoTemporalCalibrationWizard::ReportSuccessTrackerDisconnectionProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportSuccessTrackerDisconnectionProcessing called...\n");
}

/** Method to be invoked on successful tracker start */
void
VideoTemporalCalibrationWizard::ReportSuccessStartTrackingProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportSuccessStartTrackingProcessing called...\n")
}

/** Method to be invoked on Failure tracker start */
void
VideoTemporalCalibrationWizard
::ReportFailureStartTrackingProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportFailureStartTrackingProcessing called...\n")
}

/** Method to be invoked on Failure tracker stop */
void
VideoTemporalCalibrationWizard
::ReportFailureStopTrackingProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportFailureStopTrackingProcessing called...\n")
}

/** Method to be invoked on Failure imager stop */
void
VideoTemporalCalibrationWizard
::ReportFailureStopImagingProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportFailureStopImagingProcessing called...\n")
}

/** Method to be invoked on successful tracker stop */
void
VideoTemporalCalibrationWizard
::ReportSuccessStopImagingProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportSuccessStopImagingProcessing called...\n")

  m_ToggleRunVideoButton->label("Run");
}

/** Method to be invoked on successful tracker stop */
void
VideoTemporalCalibrationWizard
::ReportSuccessStopTrackingProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportSuccessStopTrackingProcessing called...\n")

  m_VideoRunning = false;
}

/** Method to be invoked on Failure imager tool spatial object loading */
void
VideoTemporalCalibrationWizard
::ReportFailureImagerToolSpatialObjectLoadedProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportFailureImagerToolSpatialObjectLoadedProcessing called...\n");

}

/** -----------------------------------------------------------------
* Stops imaging but keeps the imager connected to the
* communication port
*---------------------------------------------------------------------
*/
void
VideoTemporalCalibrationWizard::StopImagingProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                  "StopImagingProcessing called...\n" )

  // try to stop
  m_ImagerController->RequestStop( );

  //check if succeded
  if( m_ImagerControllerObserver->Error() )
  {
    std::string errorMessage;
    m_ImagerControllerObserver->GetErrorMessage( errorMessage );
    m_ImagerControllerObserver->ClearError();
    fl_alert( errorMessage.c_str() );
    fl_beep( FL_BEEP_ERROR );
    igstkLogMacro2( m_Logger, DEBUG, "Imager stop error\n" )
    m_StateMachine.PushInput( m_FailureInput );
  }
  else
  {
    m_StateMachine.PushInput( m_SuccessInput );
  }

  m_StateMachine.ProcessInputs();
}

void
VideoTemporalCalibrationWizard::OnSocketProgressEvent(itk::Object *source, const itk::EventObject & event)
{
  const igstk::DoubleTypeEvent *evt =
    dynamic_cast< const igstk::DoubleTypeEvent * > (&event);

  if ( evt )
  {
    igstk::EventHelperType::DoubleType progress = evt->Get();

    // Get the value of the progress
    //float progress = reinterpret_cast<igstk::DoubleTypeEvent *>(source)->GetProgress();

    // Update the progress bar and value
    m_OutProgressMeter->value(100 * progress);
    m_OutProgressCounter->value(100 * progress);

    // Show or hide progress bar if necessary
    if(progress < 1.0f && !m_WinProgress->visible())
      {
      m_WinProgress->show();
      this->CenterChildWindowInParentWindow( m_WinProgress );
      }
    else if (progress == 1.0f && m_WinProgress->visible())
      {
      m_WinProgress->hide();
      }

    // Update the screen
    Fl::check();
  }
}

/** -----------------------------------------------------------------
* Load working volume mesh. This method asks for a .msh file
* -----------------------------------------------------------------
*/
void VideoTemporalCalibrationWizard::LoadWorkingVolumeMeshProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG,
           "VideoTemporalCalibrationWizard::LoadWorkingVolumeMeshProcessing called...\n" )

    const char*  fileName = fl_file_chooser("Select the mesh file","*.msh", "");

    if ( !fileName )
    {
       igstkLogMacro2( m_Logger, DEBUG, "No file was selected\n" )
       m_StateMachine.PushInput( m_FailureInput );
       m_StateMachine.ProcessInputs();
       return;
    }

    MeshReaderType::Pointer reader = MeshReaderType::New();
    reader->RequestSetFileName( fileName );
    reader->RequestReadObject();
    MeshObjectObserver::Pointer observer = MeshObjectObserver::New();
    reader->AddObserver( igstk::MeshReader::MeshModifiedEvent(), observer);
    reader->RequestGetOutput();

    if(!observer->GotMeshObject())
    {
       igstkLogMacro2( m_Logger, DEBUG, "Cannot read mesh\n" )
       m_StateMachine.PushInput( m_FailureInput);
       m_StateMachine.ProcessInputs();
       return;
    }

    m_MeshSpatialObject = observer->GetMeshObject();
    if ( m_MeshSpatialObject.IsNull() )
    {
     igstkLogMacro2( m_Logger, DEBUG, "Cannot read mesh\n" )
     m_StateMachine.PushInput( m_FailureInput);
     m_StateMachine.ProcessInputs();
     return;
    }

    igstk::Transform identity;
    identity.SetToIdentity( igstk::TimeStamp::GetLongestPossibleTime() );

    // we want the mesh spatial object to be a child of the tracker
    m_MeshSpatialObject->RequestDetachFromParent();
    m_TrackerController->RequestAddChildSpatialObject( identity, m_MeshSpatialObject );

    // build the mesh representation
    MeshRepresentationType::Pointer m_MeshRepresentation = MeshRepresentationType::New();
    m_MeshRepresentation->RequestSetMeshObject( m_MeshSpatialObject );
    m_MeshRepresentation->SetOpacity(0.3);
    m_MeshRepresentation->SetColor(0, 0, 1);

    // add the mesh representation only to the 3D view
    m_ViewerGroup->m_3DView->RequestAddObject( m_MeshRepresentation );

    // set background color to the views
    m_ViewerGroup->m_VideoView->SetRendererBackgroundColor(0,0,0);
    m_ViewerGroup->m_3DView->SetRendererBackgroundColor(1,1,1);

    // set parallel projection to the camera
    m_ViewerGroup->m_VideoView->SetCameraParallelProjection(true);

    // reset cameras in the different views
    m_ViewerGroup->m_VideoView->RequestResetCamera();
    m_ViewerGroup->m_3DView->RequestResetCamera();

    m_StateMachine.PushInput( m_SuccessInput );
    m_StateMachine.ProcessInputs();
}

/** Method to be invoked on successful tracker tool spatial object loading */
void
VideoTemporalCalibrationWizard::ReportSuccessTrackerToolSpatialObjectLoadedProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportSuccessTrackerToolSpatialObjectLoadedProcessing called...\n");

  this->RequestConfigureTracker();
}

/** Method to be invoked on Failure tracker tool spatial object loading */
void
VideoTemporalCalibrationWizard::ReportFailureTrackerToolSpatialObjectLoadedProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportFailureTrackerToolSpatialObjectLoadedProcessing called...\n");

}

/** -----------------------------------------------------------------
* Load tracker mesh. This method asks for a .msh file
* -----------------------------------------------------------------
*/
void VideoTemporalCalibrationWizard::RequestLoadTrackerToolSpatialObject()
{
  igstkLogMacro2( m_Logger, DEBUG,
             "VideoTemporalCalibrationWizard::RequestLoadTrackerToolSpatialObject called...\n" )
  m_StateMachine.PushInput( m_LoadTrackerToolSpatialObjectInput );
  m_StateMachine.ProcessInputs();
}

/** -----------------------------------------------------------------
* Load tool spatial object mesh for the tracker. This method asks for a file with the
* spatial object mesh in the .msh format (see mesh SpatialObject in ITK)
*  -----------------------------------------------------------------
*/
void VideoTemporalCalibrationWizard::LoadTrackerToolSpatialObjectProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG,
       "VideoTemporalCalibrationWizard::LoadTrackerToolSpatialObjectProcessing called...\n" )
/*

   const char*  fileName =
    fl_file_chooser("Chose a tool spatial object mesh for the tracker!", "*.msh", "");

   if ( !fileName )
    {
     igstkLogMacro2( m_Logger, DEBUG,
       "VideoTemporalCalibrationWizard::LoadTrackerToolSpatialObjectProcessing No file was selected\n" )
     m_StateMachine.PushInput( m_FailureInput );
     m_StateMachine.ProcessInputs();
     return;
    }

   MeshReaderType::Pointer reader = MeshReaderType::New();
   reader->RequestSetFileName( fileName );

   reader->RequestReadObject();

   MeshObjectObserver::Pointer observer = MeshObjectObserver::New();

   reader->AddObserver( igstk::MeshReader::MeshModifiedEvent(), observer);

   reader->RequestGetOutput();

   if( !observer->GotMeshObject() )
   {
       igstkLogMacro2( m_Logger, DEBUG,
         "VideoTemporalCalibrationWizard::LoadTrackerToolSpatialObjectProcessing cannot read mesh\n" )
       m_StateMachine.PushInput( m_FailureInput );
       m_StateMachine.ProcessInputs();
       return;
   }

   m_TrackerToolSpatialObject = observer->GetMeshObject();

   m_TrackerToolRepresentation = MeshRepresentationType::New();
   m_TrackerToolRepresentation->RequestSetMeshObject( m_TrackerToolSpatialObject );
   m_TrackerToolRepresentation->SetOpacity(1.0);
   m_TrackerToolRepresentation->SetColor(1,0,1);
*/

   m_StateMachine.PushInput( m_SuccessInput);
   m_StateMachine.ProcessInputs();
}

/** -----------------------------------------------------------------
* Load tracker mesh. This method asks for a .msh file
* -----------------------------------------------------------------
*/
void VideoTemporalCalibrationWizard::LoadTrackerMeshProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG,
                    "VideoTemporalCalibrationWizard::LoadTrackerMeshProcessing called...\n" )

  const char*  fileName =
    fl_file_chooser("Select the mesh file","*.msh", "");

  if ( fileName != NULL )
  {
     MeshReaderType::Pointer reader = MeshReaderType::New();
     reader->RequestSetFileName( fileName );
     reader->RequestReadObject();

     MeshObjectObserver::Pointer observer = MeshObjectObserver::New();
     reader->AddObserver( igstk::MeshReader::MeshModifiedEvent(), observer);
     reader->RequestGetOutput();

     if(!observer->GotMeshObject())
     {
         igstkLogMacro2( m_Logger, DEBUG, "Cannot read mesh\n" )
         m_StateMachine.PushInput( m_FailureInput);
         m_StateMachine.ProcessInputs();
         return;
     }

     m_MeshSpatialObject = observer->GetMeshObject();
     if ( m_MeshSpatialObject.IsNull() )
     {
       igstkLogMacro2( m_Logger, DEBUG, "Cannot read mesh\n" )
       m_StateMachine.PushInput( m_FailureInput);
       m_StateMachine.ProcessInputs();
       return;
     }

     igstk::Transform identity;
     identity.SetToIdentity( igstk::TimeStamp::GetLongestPossibleTime() );

     // we want the mesh to be a child of the tracker
     m_MeshSpatialObject->RequestDetachFromParent();
     m_TrackerController->RequestAddChildSpatialObject( identity, m_MeshSpatialObject );

     MeshRepresentationType::Pointer m_MeshRepresentation = MeshRepresentationType::New();
     m_MeshRepresentation->RequestSetMeshObject( m_MeshSpatialObject );

     m_MeshRepresentation->SetOpacity(0.3);
     m_MeshRepresentation->SetColor(1,1,0);

     //m_ViewerGroup->m_VideoView->RequestAddObject( m_MeshRepresentation->Copy() );
     m_ViewerGroup->m_3DView->RequestAddObject( m_MeshRepresentation->Copy() );

     m_ViewerGroup->m_VideoView->RequestResetCamera();
     m_ViewerGroup->m_3DView->RequestResetCamera();

     m_StateMachine.PushInput( m_SuccessInput );
     m_StateMachine.ProcessInputs();
     return;
  }
  else
  {
     igstkLogMacro2( m_Logger, DEBUG, "No directory is selected\n" )
     m_StateMachine.PushInput( m_FailureInput );
     m_StateMachine.ProcessInputs();
     return;
  }
}

void VideoTemporalCalibrationWizard::InitializeTrackerProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG,
                    "VideoTemporalCalibrationWizard::InitializeTrackerProcessing called...\n" )

/*
  std::ofstream ofile;
  ofile.open("TrackerControllerStateMachineDiagram.dot");
  this->m_TrackerController->ExportStateMachineDescription( ofile, true );
*/

  if (!m_TrackerConfiguration)
  {
    igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::InitializeTrackerProcessing\
                                     There is no tracker configuration\n" )
    m_StateMachine.PushInput( m_FailureInput );
    m_StateMachine.ProcessInputs();
    return;
  }

  m_TrackerControllerObserver = TrackerControllerObserver::New();
  m_TrackerControllerObserver->SetParent( this );

  m_TrackerController->AddObserver(igstk::TrackerController::InitializeErrorEvent(),
    m_TrackerControllerObserver );

  m_TrackerController->AddObserver(igstk::TrackerStartTrackingEvent(),
    m_TrackerControllerObserver );

  m_TrackerController->AddObserver(igstk::TrackerStartTrackingErrorEvent(),
    m_TrackerControllerObserver );

  m_TrackerController->AddObserver(igstk::TrackerStopTrackingEvent(),
    m_TrackerControllerObserver );

  m_TrackerController->AddObserver(igstk::TrackerStopTrackingErrorEvent(),
    m_TrackerControllerObserver );

  m_TrackerController->AddObserver(igstk::TrackerController::RequestToolEvent(),
    m_TrackerControllerObserver );

  m_TrackerController->AddObserver(igstk::TrackerController::RequestToolErrorEvent(),
    m_TrackerControllerObserver );

  m_TrackerController->AddObserver(igstk::TrackerController::RequestToolsEvent(),
    m_TrackerControllerObserver );

  m_TrackerController->RequestInitialize( m_TrackerConfiguration );

  //check that initialization was successful
  if( m_TrackerControllerObserver->Error() )
  {
    std::string errorMessage;
    m_TrackerControllerObserver->GetErrorMessage( errorMessage );
    m_TrackerControllerObserver->ClearError();
    fl_alert( errorMessage.c_str() );
    fl_beep( FL_BEEP_ERROR );
    igstkLogMacro2( m_Logger, DEBUG, "Tracker Initialization error\n" )
    m_StateMachine.PushInput( m_FailureInput );
    m_StateMachine.ProcessInputs();
    return;
  }

  // ask for non reference tools only
  m_TrackerController->RequestGetNonReferenceToolList();

  m_StateMachine.PushInput( m_SuccessInput );
  m_StateMachine.ProcessInputs();
  return;
}

/** -----------------------------------------------------------------
* Starts tracking provided it is initialized and connected to the
* communication port
*---------------------------------------------------------------------
*/
void VideoTemporalCalibrationWizard::StartTrackingProcessing()
{
    igstkLogMacro2( m_Logger, DEBUG,
                    "VideoTemporalCalibrationWizard::StartTrackingProcessing called...\n" )

    igstk::Transform identity;
    identity.SetToIdentity(igstk::TimeStamp::GetLongestPossibleTime());

    m_TrackerController->RequestSetParentSpatialObject( identity, m_WorldReference );

    if (!m_TrackerConfiguration)
    {
      igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::StartTrackingProcessing \
                                       There is not tracker configuration object\n" )
      m_StateMachine.PushInput( m_FailureInput );
      m_StateMachine.ProcessInputs();
      return;
    }

    m_TrackerController->RequestStartTracking();

    //check that start was successful
    if( m_TrackerControllerObserver->Error() )
    {
      std::string errorMessage;
      m_TrackerControllerObserver->GetErrorMessage( errorMessage );
      m_TrackerControllerObserver->ClearError();
      fl_alert( errorMessage.c_str() );
      fl_beep( FL_BEEP_ERROR );
      igstkLogMacro2( m_Logger, DEBUG, "Tracker start error\n" )
      m_StateMachine.PushInput( m_FailureInput );
      m_StateMachine.ProcessInputs();
      return;
    }

    // create a world reference representation
    m_WorldReferenceRepresentation = AxesRepresentationType::New();
    m_WorldReferenceRepresentation->RequestSetAxesObject( m_WorldReference );
    m_WorldReference->SetSize(50,50,50);

    //imager tool
    m_ImagerToolSpatialObject = EllipsoidType::New();
    m_ImagerToolSpatialObject->SetRadius(30,30,30);
    m_ImagerToolSpatialObject->RequestDetachFromParent();
    m_ImagerToolSpatialObject->RequestSetTransformAndParent( identity, m_ImagerTool );

    m_ImagerToolRepresentation = EllipsoidRepresentationType::New();
    m_ImagerToolRepresentation->RequestSetEllipsoidObject( m_ImagerToolSpatialObject );
    m_ImagerToolRepresentation->SetColor(0, 0, 1);

    m_ViewerGroup->m_3DView->RequestAddObject( m_ImagerToolRepresentation );



    EllipsoidType::Pointer m_TrackerToolSpatialObject2 = EllipsoidType::New();
     m_TrackerToolSpatialObject2->SetRadius(30,30,30);
     m_TrackerToolSpatialObject2->RequestDetachFromParent();
     m_TrackerToolSpatialObject2->RequestSetTransformAndParent( identity, m_TrackerTool );

     EllipsoidRepresentationType::Pointer m_TrackerToolRepresentation2 = EllipsoidRepresentationType::New();
     m_TrackerToolRepresentation2->RequestSetEllipsoidObject( m_TrackerToolSpatialObject2 );
     m_TrackerToolRepresentation2->SetColor(1, 0, 1);

     m_ViewerGroup->m_3DView->RequestAddObject( m_TrackerToolRepresentation2 );

//    // set the tool spatial object to the tracker tool
//    m_TrackerToolSpatialObject->RequestDetachFromParent();
//    m_TrackerToolSpatialObject->RequestSetTransformAndParent(identity, m_TrackerTool);
//
//    // add tool spatial object to the 3D view
//    m_ViewerGroup->m_3DView->RequestAddObject( m_TrackerToolRepresentation );

    // set views' background colors
    m_ViewerGroup->m_VideoView->SetRendererBackgroundColor(1,1,1);
    m_ViewerGroup->m_3DView->SetRendererBackgroundColor(1,1,1);

    // build scene graph
    m_ViewerGroup->m_VideoView->RequestSetTransformAndParent(
      identity, m_WorldReference );

    m_ViewerGroup->m_3DView->RequestSetTransformAndParent(
      identity, m_WorldReference );

    // set up view parameters
    m_ViewerGroup->m_VideoView->SetRefreshRate( VIEW_2D_REFRESH_RATE );
    m_ViewerGroup->m_VideoView->RequestStart();
    m_ViewerGroup->m_VideoWidget->RequestEnableInteractions();

    m_ViewerGroup->m_3DView->SetRefreshRate( VIEW_3D_REFRESH_RATE );
    m_ViewerGroup->m_3DView->RequestStart();

    // add world reference to the views
//    m_ViewerGroup->m_VideoView->RequestAddObject(m_WorldReferenceRepresentation->Copy());
    m_ViewerGroup->m_3DView->RequestAddObject(m_WorldReferenceRepresentation);


    m_ViewerGroup->m_VideoView->RequestResetCamera();
    m_ViewerGroup->m_3DView->RequestResetCamera();

    m_StateMachine.PushInput( m_SuccessInput );
    m_StateMachine.ProcessInputs();
}
/** -----------------------------------------------------------------
* Stops tracking but keeps the tracker connected to the
* communication port
*---------------------------------------------------------------------
*/
void VideoTemporalCalibrationWizard::StopTrackingProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG,
                    "VideoTemporalCalibrationWizard::StopTrackingProcessing called...\n" )

  m_TrackerController->RequestStopTracking( );

  //check that stop was successful
  if( m_TrackerControllerObserver->Error() )
  {
    std::string errorMessage;
    m_TrackerControllerObserver->GetErrorMessage( errorMessage );
    m_TrackerControllerObserver->ClearError();
    fl_alert( errorMessage.c_str() );
    fl_beep( FL_BEEP_ERROR );
    igstkLogMacro2( m_Logger, DEBUG, "Tracker stop error\n" )
    m_StateMachine.PushInput( m_FailureInput );
  }
  else
  {
    m_StateMachine.PushInput( m_SuccessInput );
  }

  m_StateMachine.ProcessInputs();
}

/** -----------------------------------------------------------------
* Disconnects the tracker and closes the communication port
*---------------------------------------------------------------------
*/
void
VideoTemporalCalibrationWizard::DisconnectTrackerProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG,
                    "VideoTemporalCalibrationWizard::DisconnectTrackerProcessing called...\n" )
  // try to disconnect
  m_TrackerController->RequestShutdown( );
  //check if succeded
  if( m_TrackerControllerObserver->Error() )
  {
    std::string errorMessage;
    m_TrackerControllerObserver->GetErrorMessage( errorMessage );
    m_TrackerControllerObserver->ClearError();
    fl_alert( errorMessage.c_str() );
    fl_beep( FL_BEEP_ERROR );
    igstkLogMacro2( m_Logger, DEBUG, "Tracker disconnect error\n" )
    m_StateMachine.PushInput( m_FailureInput );
  }
  else
  {
    m_StateMachine.PushInput( m_SuccessInput );
  }

  m_StateMachine.ProcessInputs();
}

/** Method to be invoked on failure imager disconnection */
void
VideoTemporalCalibrationWizard
::ReportFailureImagerDisconnectionProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportFailureImagerDisconnectionProcessing called...\n");
}

/** Method to be invoked on successful imager disconnection */
void
VideoTemporalCalibrationWizard
::ReportSuccessImagerDisconnectionProcessing()
{
  igstkLogMacro2( m_Logger, DEBUG, "VideoTemporalCalibrationWizard::"
                 "ReportSuccessImagerDisconnectionProcessing called...\n");
}

/** Method to set the parent tp the tracker observer */
void
VideoTemporalCalibrationWizard
::TrackerControllerObserver::SetParent( VideoTemporalCalibrationWizard *p )
{
  m_Parent = p;
}

void
VideoTemporalCalibrationWizard
::TrackerControllerObserver::Execute( const itk::Object *caller,
                                                    const itk::EventObject & event )
{
  const itk::Object * constCaller = caller;
  this->Execute(constCaller, event);
}

void
VideoTemporalCalibrationWizard::TrackerControllerObserver::Execute( itk::Object *caller, const itk::EventObject & event )
{
  const igstk::TrackerController::InitializeErrorEvent *evt1a =
    dynamic_cast< const igstk::TrackerController::InitializeErrorEvent * > (&event);

  const igstk::TrackerStartTrackingErrorEvent *evt1b =
    dynamic_cast< const igstk::TrackerStartTrackingErrorEvent * > (&event);

  const igstk::TrackerStopTrackingErrorEvent *evt1c =
    dynamic_cast< const igstk::TrackerStopTrackingErrorEvent * > (&event);

   const igstk::TrackerController::RequestToolsEvent *evt3 =
    dynamic_cast< const igstk::TrackerController::RequestToolsEvent * > (&event);

  const igstk::TrackerController::RequestToolEvent *evt4 =
    dynamic_cast< const igstk::TrackerController::RequestToolEvent * > (&event);

  if( evt1a )
  {
    m_ErrorOccured = true;
    m_ErrorMessage = evt1a->Get();
  }
  else if ( evt1b )
  {
    m_ErrorOccured = true;
    m_ErrorMessage = evt1b->Get();
  }
  else if ( evt1c )
  {
    m_ErrorOccured = true;
    m_ErrorMessage = evt1c->Get();
  }
  else if ( evt3 )
  {
    igstk::TrackerController::ToolContainerType toolContainer = evt3->Get();

    igstk::TrackerController::ToolContainerType::iterator
                          trackerIter = toolContainer.find(TRACKER_TOOL_NAME);

    if ( trackerIter!=toolContainer.end() )
    {
        m_Parent->m_TrackerTool = (*trackerIter).second;

        std::cout << "found tracker tool: " << TRACKER_TOOL_NAME << std::endl;

        // observe tracker tool not available events
        m_Parent->m_TrackerToolNotAvailableObserver = LoadedObserverType::New();
        m_Parent->m_TrackerToolNotAvailableObserver->SetCallbackFunction( m_Parent,
                                                 &VideoTemporalCalibrationWizard::TrackerToolNotAvailableCallback );

        m_Parent->m_TrackerTool->AddObserver(
         igstk::TrackerToolNotAvailableToBeTrackedEvent(), m_Parent->m_TrackerToolNotAvailableObserver);

        // observe tracker tool available events
        m_Parent->m_TrackerToolAvailableObserver = LoadedObserverType::New();
        m_Parent->m_TrackerToolAvailableObserver->SetCallbackFunction( m_Parent,
                                                 &VideoTemporalCalibrationWizard::TrackerToolAvailableCallback );

        m_Parent->m_TrackerTool->AddObserver(
         igstk::TrackerToolMadeTransitionToTrackedStateEvent(), m_Parent->m_TrackerToolAvailableObserver);

        m_Parent->m_TrackerToolUpdateObserver    = LoadedObserverType::New();
        m_Parent->m_TrackerToolUpdateObserver->SetCallbackFunction( m_Parent,
                               &VideoTemporalCalibrationWizard::TrackerToolUpdateTransformCallback );
    }

    igstk::TrackerController::ToolContainerType::iterator imagerIter = toolContainer.find(IMAGER_TOOL_NAME);

    if ( imagerIter!=toolContainer.end() )
    {
        m_Parent->m_ImagerTool = (*imagerIter).second;

        std::cout << "found imager tool: " << IMAGER_TOOL_NAME << std::endl;

        // observer imager tool not available events
        m_Parent->m_ImagerToolNotAvailableObserver = LoadedObserverType::New();
        m_Parent->m_ImagerToolNotAvailableObserver->SetCallbackFunction( m_Parent,
                                                 &VideoTemporalCalibrationWizard::ImagerToolNotAvailableCallback );
        m_Parent->m_ImagerTool->AddObserver(
         igstk::TrackerToolNotAvailableToBeTrackedEvent(), m_Parent->m_ImagerToolNotAvailableObserver);

        // observer imager tool available events
        m_Parent->m_ImagerToolAvailableObserver = LoadedObserverType::New();
        m_Parent->m_ImagerToolAvailableObserver->SetCallbackFunction( m_Parent,
                                                 &VideoTemporalCalibrationWizard::ImagerToolAvailableCallback );

        m_Parent->m_ImagerTool->AddObserver(
         igstk::TrackerToolMadeTransitionToTrackedStateEvent(), m_Parent->m_ImagerToolAvailableObserver);

        // instantiate it but not add it yet (see toggle collector)
        m_Parent->m_ImagerToolUpdateObserver    = LoadedObserverType::New();
        m_Parent->m_ImagerToolUpdateObserver->SetCallbackFunction( m_Parent,
                               &VideoTemporalCalibrationWizard::ImagerToolUpdateTransformCallback );
    }
    /*
    igstk::TrackerController::ToolContainerType::iterator iter = toolContainer.begin();
    for ( ; iter != toolContainer.end(); iter++ )
    {
      m_Parent->m_ToolVector.push_back( (*iter).second );
      std::cout << "VideoTemporalCalibrationWizard::TrackerControllerObserver::Execute() found tool: " << (*iter).first << std::endl;
    }
    */
  }
  else if ( evt4 )
  {
  }
}

void
VideoTemporalCalibrationWizard
::ImagerControllerObserver::SetParent(
  VideoTemporalCalibrationWizard *p )
{
  m_Parent = p;
}

void
VideoTemporalCalibrationWizard
::ImagerControllerObserver::Execute( const itk::Object *caller,
                                     const itk::EventObject & event )
{
  const itk::Object * constCaller = caller;
  this->Execute(constCaller, event);
}

void
VideoTemporalCalibrationWizard
::TrackerToolAvailableCallback(const itk::EventObject & event )
{
  m_TrackerSemaphore->color(FL_GREEN);
  m_TrackerSemaphore->label("tracking");
  m_ControlGroup->redraw();
  Fl::check();
}

void
VideoTemporalCalibrationWizard
::TrackerToolNotAvailableCallback(const itk::EventObject & event )
{
  m_TrackerSemaphore->color(FL_RED);
  m_TrackerSemaphore->label("not visible");
  m_ControlGroup->redraw();
  Fl::check();
}

void
VideoTemporalCalibrationWizard
::ImagerToolAvailableCallback(const itk::EventObject & event )
{
  m_ImagerSemaphore->color(FL_GREEN);
  m_ImagerSemaphore->label("tracking");
  m_ControlGroup->redraw();
  Fl::check();
}

void
VideoTemporalCalibrationWizard
::ImagerToolNotAvailableCallback(const itk::EventObject & event )
{
  m_ImagerSemaphore->color(FL_RED);
  m_ImagerSemaphore->label("not visible");
  m_ControlGroup->redraw();
  Fl::check();
}

void
VideoTemporalCalibrationWizard
::ImagerControllerObserver::Execute( itk::Object *caller,
                                     const itk::EventObject & event )
{
  const igstk::ImagerController::InitializeFailureEvent *evt1a =
    dynamic_cast< const igstk::ImagerController::InitializeFailureEvent * > (&event);

  const igstk::ImagerController::StartFailureEvent *evt1b =
    dynamic_cast< const igstk::ImagerController::StartFailureEvent * > (&event);

    const igstk::ImagerController::StopFailureEvent *evt1c =
    dynamic_cast< const igstk::ImagerController::StopFailureEvent * > (&event);

  const igstk::ImagerController::RequestImagerEvent *evt2 =
    dynamic_cast< const igstk::ImagerController::RequestImagerEvent * > (&event);

  const igstk::ImagerController::RequestToolsEvent *evt3 =
    dynamic_cast< const igstk::ImagerController::RequestToolsEvent * > (&event);

  const igstk::ImagerController::OpenCommunicationFailureEvent *evt4 =
    dynamic_cast< const igstk::ImagerController::OpenCommunicationFailureEvent * > (&event);

  if( evt1a )
  {
    m_ErrorOccured = true;
     m_ErrorMessage = evt1a->Get();
  }
  else if ( evt1b )
  {
    m_ErrorOccured = true;
    m_ErrorMessage = evt1b->Get();
  }
  else if ( evt1c )
  {
    m_ErrorOccured = true;
    m_ErrorMessage = evt1c->Get();
  }
  else if ( evt2 )
  {
    m_Parent->m_Imager = evt2->Get();
  }
  else if ( evt3 )
  {
    const std::vector<igstk::ImagerTool::Pointer> &tools = evt3->Get();

    igstkLogMacro2( m_Parent->m_Logger, DEBUG,
                    "VideoTemporalNavigator::ImagerControllerObserver found imager tool!\n" )

    m_Parent->m_ImagingSourceImagerTool = tools[0];
  }
  else if ( evt4 )
  {
    m_ErrorOccured = true;
    m_ErrorMessage = evt4->Get();
  }
}

void
VideoTemporalCalibrationWizard
::RequestToggleEnableCollector()
{
  igstkLogMacro2( m_Logger, DEBUG,
   "VideoTemporalCalibrationWizard::RequestToggleEnableCollector called...\n" )

  if (m_CollectorEnabled)
  {
    this->m_ToggleEnableCollectorButton->color((Fl_Color)55);

    if ( m_TrackerTool.IsNotNull() )
    {
    m_TrackerTool->RemoveObserver( m_TrackerToolUpdateObserverID );
    }

    if ( m_ImagerTool.IsNotNull() )
    {
    m_ImagerTool->RemoveObserver( m_ImagerToolUpdateObserverID );
    }

    std::string   samplePoints;
    samplePoints = "samplePointsForTemporalCalibration"
      + itksys::SystemTools::GetCurrentDateTime( "_%Y_%m_%d_%H_%M_%S" ) + ".txt";
    WriteSampledPoints(samplePoints.c_str());
  }
  else
  {
    this->m_ToggleEnableCollectorButton->color(FL_GREEN);

    if ( m_TrackerTool.IsNotNull() )
    {
      m_TrackerToolUpdateObserverID = m_TrackerTool->AddObserver(
                    igstk::TrackerToolTransformUpdateEvent(), m_TrackerToolUpdateObserver);
    }

    if ( m_ImagerTool.IsNotNull() )
    {
      m_ImagerToolUpdateObserverID = m_ImagerTool->AddObserver(
                    igstk::TrackerToolTransformUpdateEvent(), m_ImagerToolUpdateObserver);
    }
  }

  m_CollectorEnabled = !m_CollectorEnabled;
}

void
VideoTemporalCalibrationWizard
::RequestToggleEnableVideo()
{
  igstkLogMacro2( m_Logger, DEBUG,
   "VideoTemporalCalibrationWizard::RequestToggleEnableVideo called...\n" )

  if (m_VideoEnabled)
  {
    this->m_ToggleEnableVideoButton->color((Fl_Color)55);
    this->DisableVideo();
  }
  else
  {
    this->m_ToggleEnableVideoButton->color(FL_GREEN);
    this->EnableVideo();
  }

  m_VideoEnabled = !m_VideoEnabled;
}

void VideoTemporalCalibrationWizard::EnableVideo()
{
  // we put here stuff that we want to do once
  // fixme: set a proper state machine action
  if (m_VideoFrame.IsNull())
  {
     // setup the video frame spatial object
     m_VideoFrame = VideoFrameSpatialObjectType::New();
     m_VideoFrame->SetWidth(640);
     m_VideoFrame->SetHeight(480);
     m_VideoFrame->SetPixelSizeX(0.4); // in mm
     m_VideoFrame->SetPixelSizeY(0.4); // in mm
     m_VideoFrame->SetNumberOfScalarComponents(3);
     m_VideoFrame->Initialize();

     igstk::Transform identity;
     identity.SetToIdentity( igstk::TimeStamp::GetLongestPossibleTime() );

     // set transformation between m_VideoFrame and m_ImagerToolSpatialObject according to
     // VideoTemporal calibration
     m_VideoFrame->RequestSetTransformAndParent( identity, m_WorldReference );

     m_ViewerGroup->m_VideoView->RequestDetachFromParent();
     m_ViewerGroup->m_VideoView->RequestSetTransformAndParent( identity, m_VideoFrame );

    // m_WindowWidthVideo = 286;
     //m_WindowLevelVideo = 108;
     m_WindowWidthVideo = 255;
     m_WindowLevelVideo = 128;

     // create a video frame representation for the video view
     m_VideoFrameRepresentationForVideoView = VideoFrameRepresentationType::New();
     m_VideoFrameRepresentationForVideoView->RequestSetVideoFrameSpatialObject( m_VideoFrame );
     m_VideoFrameRepresentationForVideoView->SetWindowLevel(m_WindowWidthVideo,m_WindowLevelVideo);

//     m_VideoFrameRepresentationFor3DView = VideoFrameRepresentationType::New();
//     m_VideoFrameRepresentationFor3DView->RequestSetVideoFrameSpatialObject( m_VideoFrame );
//     m_VideoFrameRepresentationFor3DView->SetWindowLevel(m_WindowWidthVideo,m_WindowLevelVideo);
  }

  if (m_VideoFrame.IsNotNull())
  {
    m_ViewerGroup->m_VideoView->RequestAddObject( m_VideoFrameRepresentationForVideoView );
    m_ViewerGroup->m_3DView->RequestAddObject( m_VideoFrameRepresentationFor3DView );

    m_ToggleRunVideoButton->activate();
  }

  m_ToggleEnableVideoButton->label("Disable");
  m_ViewerGroup->m_VideoView->RequestResetCamera();
  m_ViewerGroup->m_3DView->RequestResetCamera();
}

void VideoTemporalCalibrationWizard::DisableVideo()
{
  m_ViewerGroup->m_VideoView->RequestRemoveObject( m_VideoFrameRepresentationForVideoView );
  m_ViewerGroup->m_3DView->RequestRemoveObject( m_VideoFrameRepresentationFor3DView );

  m_ViewerGroup->m_VideoView->RequestResetCamera();
  m_ViewerGroup->m_3DView->RequestResetCamera();

  m_ToggleEnableVideoButton->label("Enable");
  m_ToggleRunVideoButton->deactivate();
}

void
VideoTemporalCalibrationWizard
::StopVideo()
{
  this->RequestStopImaging();
}

void
VideoTemporalCalibrationWizard
::StartVideo()
{
  // if we are not initialize
  if (!m_ImagerInitialized)
  {
  this->RequestInitializeImager();
  }

  if (m_ImagerInitialized)
  {
  this->RequestStartImaging();
  }

}


void VideoTemporalCalibrationWizard
::RequestToggleRunVideo()
{
  igstkLogMacro2( m_Logger, DEBUG,
      "VideoTemporalCalibrationWizard::RequestToggleRunVideo called...\n" )

  if (m_VideoRunning)
  {
    this->StopVideo();
  }
  else
  {
    this->StartVideo();
  }
}

void
VideoTemporalCalibrationWizard
::RequestPrepareToQuit()
{
  this->RequestStopTracking();
  this->RequestDisconnectTracker();
}

/** -----------------------------------------------------------------
* Callback function for observer listening to a key-pressed event
*---------------------------------------------------------------------
*/
void
VideoTemporalCalibrationWizard
::HandleKeyPressedCallback( const itk::EventObject & event )
{
  if ( igstk::VideoTemporalCalibrationWizardQuadrantViews::KeyPressedEvent().CheckEvent( &event ) )
  {
    igstk::VideoTemporalCalibrationWizardQuadrantViews::KeyPressedEvent *keyPressedEvent =
      ( igstk::VideoTemporalCalibrationWizardQuadrantViews::KeyPressedEvent *) & event;
    this->HandleKeyPressed( keyPressedEvent->Get() );
  }
}

void
VideoTemporalCalibrationWizard
::HandleKeyPressed (
  igstk::VideoTemporalCalibrationWizardQuadrantViews::KeyboardCommandType keyCommand )
{
  switch ( keyCommand.key )
  {

    case 'r':// reset the cameras in the different views
             m_ViewerGroup->m_VideoView->RequestResetCamera();
             m_ViewerGroup->m_3DView->RequestResetCamera();
             break;

    case 'g':// grab tracker and imager transforms
             if (m_CollectorEnabled)
             {
               m_CollectingProbeSamples = true;
               m_CollectingPointerSamples = true;
             }
             break;
    default:
         return;
  }
}

/** -----------------------------------------------------------------
* Callback function for observer listening to a mouse-pressed event
*---------------------------------------------------------------------
*/
void
VideoTemporalCalibrationWizard
::HandleMousePressedCallback( const itk::EventObject & event )
{
  if ( igstk::VideoTemporalCalibrationWizardQuadrantViews::MousePressedEvent().CheckEvent( &event ) )
  {
    igstk::VideoTemporalCalibrationWizardQuadrantViews::MousePressedEvent *mousePressedEvent =
      ( igstk::VideoTemporalCalibrationWizardQuadrantViews::MousePressedEvent *) & event;
    this->HandleMousePressed( mousePressedEvent->Get() );
  }
}

void
VideoTemporalCalibrationWizard
::HandleMousePressed (
    igstk::VideoTemporalCalibrationWizardQuadrantViews::MouseCommandType mouseCommand )
{
    m_WindowWidthVideo += mouseCommand.dx;

    if (m_WindowWidthVideo < 1)
      m_WindowWidthVideo = 1;

    m_WindowLevelVideo += mouseCommand.dy;

    if ( m_VideoFrameRepresentationForVideoView.IsNotNull() )
      m_VideoFrameRepresentationForVideoView->SetWindowLevel(m_WindowWidthVideo,m_WindowLevelVideo);

    if ( m_VideoFrameRepresentationFor3DView.IsNotNull() )
      m_VideoFrameRepresentationFor3DView->SetWindowLevel(m_WindowWidthVideo,m_WindowLevelVideo);
}


/** -----------------------------------------------------------------
*  Overwrite the file with the sampled points
*---------------------------------------------------------------------
*/
void
VideoTemporalCalibrationWizard
::WriteSampledPoints( const char *filename )
{
  igstk::VideoTemporalCalibrationIO * writer = new igstk::VideoTemporalCalibrationIO;
  writer->SetFileName( filename );
  writer->SetVideoTemporalCalibration( m_VideoTemporalCalibrationSamples );
  writer->RequestWrite();
}

void
VideoTemporalCalibrationWizard
::TrackerToolUpdateTransformCallback( const itk::EventObject & event )
{
//  std::cout << "TrackerToolUpdateTransformCallback entered " << std::endl;

  if ( igstk::TrackerToolTransformUpdateEvent().CheckEvent( & event ) )
  {
    typedef igstk::TransformObserver ObserverType;
    ObserverType::Pointer transformObserver = ObserverType::New();
    transformObserver->ObserveTransformEventsFrom( m_TrackerTool );
    transformObserver->Clear();

    m_TrackerTool->RequestComputeTransformTo( m_WorldReference );

    if ( transformObserver->GotTransform() )
    {
//        std::cout << "TrackerToolUpdateTransformCallback transformObserver->GotTransform() passed " << std::endl;

        igstk::Transform transform = transformObserver->GetTransform();

        if ( m_CollectingPointerSamples )
        {
            PointType point = TransformToPoint( transform );
            m_VideoTemporalCalibrationSamples->m_PointerTransforms.push_back( transform );

            std::cout << "pointer: " << point[0] << " " << point[1] << " " << point[2] << " size " << m_VideoTemporalCalibrationSamples->m_PointerTransforms.size() << std::endl;

//            std::stringstream filename;
//            filename << "VideoTemporal" << m_SnapShotCounter << ".png";
//            m_SnapShotCounter++;
//            m_VideoFrameRepresentationForVideoView->SaveScreenShot( filename.str().c_str() );
//            fl_beep( FL_BEEP_MESSAGE );

            m_CollectingPointerSamples = false;
        }
    }
  }
}


void
VideoTemporalCalibrationWizard
::ImagerToolUpdateTransformCallback(const itk::EventObject & event )
{
  if ( igstk::TrackerToolTransformUpdateEvent().CheckEvent( &event ) )
  {
    typedef igstk::TransformObserver ObserverType;
    ObserverType::Pointer transformObserver = ObserverType::New();
    transformObserver->ObserveTransformEventsFrom( m_ImagerTool );
    transformObserver->Clear();

    m_ImagerTool->RequestComputeTransformTo( m_WorldReference );

    if ( transformObserver->GotTransform() )
    {
       igstk::Transform transform = transformObserver->GetTransform();

       if ( m_CollectingProbeSamples )
       {
           PointType point = TransformToPoint( transform );
           m_VideoTemporalCalibrationSamples->m_ProbeTransforms.push_back( transform );
           std::cout << "probe:" << point[0] << " " << point[1] << " " << point[2] << std::endl;

           std::stringstream filename;
           filename << "VideoTemporal" << m_SnapShotCounter << ".png";
           m_SnapShotCounter++;
           m_VideoFrameRepresentationForVideoView->SaveScreenShot( filename.str().c_str() );
           fl_beep( FL_BEEP_MESSAGE );

           m_CollectingProbeSamples = false;
       }
    }
  }
}

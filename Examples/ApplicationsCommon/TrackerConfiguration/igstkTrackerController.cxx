/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkTrackerController.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "igstkTrackerController.h"

#include "igstkTrackerConfiguration.h"

#include "itksys/SystemTools.hxx"
#include "itksys/Directory.hxx"

#include "igstkPolarisTracker.h"
#include "igstkAuroraTracker.h"
//#include "igstkFlockOfBirdsTracker.h"

#ifdef IGSTKSandbox_USE_MicronTracker
#include "igstkMicronTracker.h"
#endif


namespace igstk
{ 


TrackerController::TrackerController() : m_StateMachine( this )
{
            //create error observer
  this->m_ErrorObserver = ErrorObserver::New();

           //create observer for the tracker update status events
  this->m_TrackerUpdateStatusObserver = 
    TrackerUpdateObserver::New();
  this->m_TrackerUpdateStatusObserver->SetParent( this );

        //define the state machine's states 
  igstkAddStateMacro( Idle );
  igstkAddStateMacro( AttemptingToInitialize );
  igstkAddStateMacro( AttemptingToInitializePolarisVicra );
  igstkAddStateMacro( AttemptingToInitializePolarisHybrid );
  igstkAddStateMacro( AttemptingToInitializeAurora );
  igstkAddStateMacro( AttemptingToInitializeMicron );
  igstkAddStateMacro( AttemptingToInitializeMedSafe );  
  igstkAddStateMacro( Initialized );
  igstkAddStateMacro( AttemptingToStartTracking );
  igstkAddStateMacro( Tracking );
  igstkAddStateMacro( AttemptingToCloseCommunication );
  igstkAddStateMacro( AttemptingToStopTracking );

                   //define the state machine's inputs
  igstkAddInputMacro( TrackerInitialize );
  igstkAddInputMacro( PolarisVicraInitialize );
  igstkAddInputMacro( PolarisHybridInitialize );
  igstkAddInputMacro( AuroraInitialize );
  igstkAddInputMacro( MicronInitialize );
  igstkAddInputMacro( MedSafeInitialize );
  igstkAddInputMacro( StartTracking );
  igstkAddInputMacro( StopTracking );
  igstkAddInputMacro( Failed  );
  igstkAddInputMacro( Succeeded  );
  igstkAddInputMacro( CloseCommunication );
  igstkAddInputMacro( GetTools  );
  igstkAddInputMacro( GetTool  );
  igstkAddInputMacro( GetReferenceTool  );

            //define the state machine's transitions

                         //transitions from Idle state
  igstkAddTransitionMacro( Idle,
                           TrackerInitialize,
                           AttemptingToInitialize,
                           TrackerInitialize );

  igstkAddTransitionMacro( Idle,
                           PolarisVicraInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Idle,
                           PolarisHybridInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( Idle,
                           AuroraInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( Idle,
                           MicronInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( Idle,
                           MedSafeInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( Idle,
                           StartTracking,
                           Idle,
                           ReportInvalidRequest);

  igstkAddTransitionMacro( Idle,
                           StopTracking,
                           Idle,
                           ReportStopTrackingSuccess );

  igstkAddTransitionMacro( Idle,
                           Failed,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Idle,
                           Succeeded,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( Idle,
                           CloseCommunication,
                           Idle,
                           ReportCloseCommunicationSuccess );
  
  igstkAddTransitionMacro( Idle,
                           GetTools,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Idle,
                           GetTool,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Idle,
                           GetReferenceTool,
                           Idle,
                           ReportInvalidRequest );

                  //transitions from AttemptingToInitialize state
  igstkAddTransitionMacro( AttemptingToInitialize,
                           Failed,
                           Idle,
                           ReportInitializationFailure );                          

  igstkAddTransitionMacro( AttemptingToInitialize,
                           PolarisVicraInitialize,
                           AttemptingToInitializePolarisVicra,
                           PolarisVicraInitialize );
  
  igstkAddTransitionMacro( AttemptingToInitialize,
                           PolarisHybridInitialize,
                           AttemptingToInitializePolarisHybrid,
                           PolarisHybridInitialize );
 
  igstkAddTransitionMacro( AttemptingToInitialize,
                           AuroraInitialize,
                           AttemptingToInitializeAurora,
                           AuroraInitialize );

  igstkAddTransitionMacro( AttemptingToInitialize,
                           MicronInitialize,
                           AttemptingToInitializeMicron,
                           MicronInitialize );

  igstkAddTransitionMacro( AttemptingToInitialize,
                           MedSafeInitialize,
                           AttemptingToInitializeMedSafe,
                           MedSafeInitialize );

  igstkAddTransitionMacro( AttemptingToInitialize,
                           TrackerInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitialize,
                           StartTracking,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitialize,
                           StopTracking,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitialize,
                           Succeeded,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitialize,
                           CloseCommunication,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitialize,
                           GetTools,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitialize,
                           GetTool,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitialize,
                           GetReferenceTool,
                           Idle,
                           ReportInvalidRequest );

           //transitions from AttemptingToInitializePolarisVicra state
  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           Failed,
                           Idle,
                           ReportInitializationFailure );
  
  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           Succeeded,
                           Initialized,
                           ReportInitializationSuccess );

  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           TrackerInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           PolarisVicraInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           PolarisHybridInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           AuroraInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           MicronInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           MedSafeInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           StartTracking,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           StopTracking,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           CloseCommunication,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           GetTools,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           GetTool,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializePolarisVicra,
                           GetReferenceTool,
                           Idle,
                           ReportInvalidRequest );

           //transitions from AttemptingToInitializePolarisHybrid state
  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           Failed,
                           Idle,
                           ReportInitializationFailure );
  
  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           Succeeded,
                           Initialized,
                           ReportInitializationSuccess );

  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           TrackerInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           PolarisVicraInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           PolarisHybridInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           AuroraInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           MicronInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           MedSafeInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           StartTracking,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           StopTracking,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           CloseCommunication,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           GetTools,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           GetTool,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializePolarisHybrid,
                           GetReferenceTool,
                           Idle,
                           ReportInvalidRequest );

          //transitions from AttemptingToInitializeAurora state
  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           Failed,
                           Idle,
                           ReportInitializationFailure );
  
  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           Succeeded,
                           Initialized,
                           ReportInitializationSuccess );

  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           TrackerInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           PolarisVicraInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           PolarisHybridInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           AuroraInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           MicronInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           MedSafeInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           StartTracking,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           StopTracking,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           CloseCommunication,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           GetTools,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           GetTool,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeAurora,
                           GetReferenceTool,
                           Idle,
                           ReportInvalidRequest );

          //transitions from AttemptingToInitializeMicron state
  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           Failed,
                           Idle,
                           ReportInitializationFailure );
  
  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           Succeeded,
                           Initialized,
                           ReportInitializationSuccess );

  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           TrackerInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           PolarisVicraInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           PolarisHybridInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           AuroraInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           MicronInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           MedSafeInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           StartTracking,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           StopTracking,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           CloseCommunication,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           GetTools,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           GetTool,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeMicron,
                           GetReferenceTool,
                           Idle,
                           ReportInvalidRequest);

          //transitions from AttemptingToInitializeMedSafe state
  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           Failed,
                           Idle,
                           ReportInitializationFailure );
  
  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           Succeeded,
                           Initialized,
                           ReportInitializationSuccess );

  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           TrackerInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           PolarisVicraInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           PolarisHybridInitialize,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           AuroraInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           MicronInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           MedSafeInitialize,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           StartTracking,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           StopTracking,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           CloseCommunication,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           GetTools,
                           Idle,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           GetTool,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToInitializeMedSafe,
                           GetReferenceTool,
                           Idle,
                           ReportInvalidRequest );
          
          //transitions from Initialized state
  igstkAddTransitionMacro( Initialized,
                           GetTools,
                           Initialized,
                           GetTools );

  igstkAddTransitionMacro( Initialized,
                           GetTool,
                           Initialized,
                           GetTool );

  igstkAddTransitionMacro( Initialized,
                           GetReferenceTool,
                           Initialized,
                           GetReferenceTool );

  igstkAddTransitionMacro( Initialized,
                           StartTracking,
                           AttemptingToStartTracking,
                           StartTracking);

  igstkAddTransitionMacro( Initialized,
                           StopTracking,
                           Initialized,
                           ReportStopTrackingSuccess);

  igstkAddTransitionMacro( Initialized,
                           TrackerInitialize,
                           AttemptingToInitialize,
                           TrackerInitialize );

  igstkAddTransitionMacro( Initialized,
                           CloseCommunication,
                           AttemptingToCloseCommunication,
                           CloseCommunication );

  igstkAddTransitionMacro( Initialized,
                           PolarisVicraInitialize,
                           Initialized,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Initialized,
                           PolarisHybridInitialize,
                           Initialized,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( Initialized,
                           AuroraInitialize,
                           Initialized,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( Initialized,
                           MicronInitialize,
                           Initialized,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( Initialized,
                           MedSafeInitialize,
                           Initialized,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Initialized,
                           Failed,
                           Initialized,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Initialized,
                           Succeeded,
                           Initialized,
                           ReportInvalidRequest );

         //transitions from AttemptingToStartTracking state
  igstkAddTransitionMacro( AttemptingToStartTracking,
                           Succeeded,
                           Tracking,
                           ReportStartTrackingSuccess );

  igstkAddTransitionMacro( AttemptingToStartTracking,
                           Failed,
                           Initialized,
                           ReportStartTrackingFailure );

  igstkAddTransitionMacro( AttemptingToStartTracking,
                           TrackerInitialize,
                           AttemptingToStartTracking,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToStartTracking,
                           PolarisVicraInitialize,
                           AttemptingToStartTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStartTracking,
                           PolarisHybridInitialize,
                           AttemptingToStartTracking,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToStartTracking,
                           AuroraInitialize,
                           AttemptingToStartTracking,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToStartTracking,
                           MicronInitialize,
                           AttemptingToStartTracking,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToStartTracking,
                           MedSafeInitialize,
                           AttemptingToStartTracking,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToStartTracking,
                           StartTracking,
                           AttemptingToStartTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStartTracking,
                           StopTracking,
                           AttemptingToStartTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStartTracking,
                           CloseCommunication,
                           AttemptingToStartTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStartTracking,
                           GetTools,
                           AttemptingToStartTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStartTracking,
                           GetTool,
                           AttemptingToStartTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStartTracking,
                           GetReferenceTool,
                           AttemptingToStartTracking,
                           ReportInvalidRequest );

        //transitions from Tracking state
  igstkAddTransitionMacro( Tracking,
                           StopTracking,
                           AttemptingToStopTracking,
                           StopTracking );

  igstkAddTransitionMacro( Tracking,
                           TrackerInitialize,
                           Tracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Tracking,
                           PolarisVicraInitialize,
                           Tracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Tracking,
                           PolarisHybridInitialize,
                           Tracking,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( Tracking,
                           AuroraInitialize,
                           Tracking,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( Tracking,
                           MicronInitialize,
                           Tracking,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( Tracking,
                           MedSafeInitialize,
                           Tracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Tracking,
                           StartTracking,
                           Tracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Tracking,
                           Failed,
                           Tracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Tracking,
                           Succeeded,
                           Tracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Tracking,
                           CloseCommunication,
                           Tracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Tracking,
                           GetTools,
                           Tracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Tracking,
                           GetTool,
                           Tracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( Tracking,
                           GetReferenceTool,
                           Tracking,
                           ReportInvalidRequest );

     //transitions from AttemtingtoStop state
  igstkAddTransitionMacro( AttemptingToStopTracking,
                           Succeeded,
                           Initialized,
                           ReportStopTrackingSuccess );

  igstkAddTransitionMacro( AttemptingToStopTracking,
                           Failed,
                           Tracking,
                           ReportStopTrackingFailure );

  igstkAddTransitionMacro( AttemptingToStopTracking,
                           TrackerInitialize,
                           AttemptingToStopTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStopTracking,
                           PolarisVicraInitialize,
                           AttemptingToStopTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStopTracking,
                           PolarisHybridInitialize,
                           AttemptingToStopTracking,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToStopTracking,
                           AuroraInitialize,
                           AttemptingToStopTracking,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToStopTracking,
                           MicronInitialize,
                           AttemptingToStopTracking,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToStopTracking,
                           MedSafeInitialize,
                           AttemptingToStopTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStopTracking,
                           StartTracking,
                           AttemptingToStopTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStopTracking,
                           StopTracking,
                           AttemptingToStopTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStopTracking,
                           CloseCommunication,
                           AttemptingToStopTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStopTracking,
                           GetTools,
                           AttemptingToStopTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStopTracking,
                           GetTool,
                           AttemptingToStopTracking,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToStopTracking,
                           GetReferenceTool,
                           AttemptingToStopTracking,
                           ReportInvalidRequest );


       //transitions from AttemptingToCloseCommunication state

  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           Succeeded,
                           Idle,
                           ReportCloseCommunicationSuccess );

  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           Failed,
                           Initialized,
                           ReportCloseCommunicationFailure );                          

  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           TrackerInitialize,
                           AttemptingToCloseCommunication,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           PolarisVicraInitialize,
                           AttemptingToCloseCommunication,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           PolarisHybridInitialize,
                           AttemptingToCloseCommunication,
                           ReportInvalidRequest );
 
  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           AuroraInitialize,
                           AttemptingToCloseCommunication,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           MicronInitialize,
                           AttemptingToCloseCommunication,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           MedSafeInitialize,
                           AttemptingToCloseCommunication,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           StartTracking,
                           AttemptingToCloseCommunication,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           StopTracking,
                           AttemptingToCloseCommunication,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           CloseCommunication,
                           AttemptingToCloseCommunication,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           GetTools,
                           AttemptingToCloseCommunication,
                           ReportInvalidRequest);
  
  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           GetTool,
                           AttemptingToCloseCommunication,
                           ReportInvalidRequest);

  igstkAddTransitionMacro( AttemptingToCloseCommunication,
                           GetReferenceTool,
                           AttemptingToCloseCommunication,
                           ReportInvalidRequest );

              //set the initial state of the state machine
  igstkSetInitialStateMacro( Idle );

         // done setting the state machine, ready to run
  this->m_StateMachine.SetReadyToRun();
} 


TrackerController::~TrackerController()  
{
     
}

void
TrackerController::RequestInitialize(
  const TrackerConfiguration *configuration)
{
  igstkLogMacro( DEBUG, 
                 "igstkTrackerController::RequestInitialize called...\n" );
  this->m_TmpTrackerConfiguration = 
    const_cast<TrackerConfiguration *>(configuration);
  igstkPushInputMacro( TrackerInitialize );
  this->m_StateMachine.ProcessInputs();
}

void
TrackerController::RequestStartTracking( )
{
  igstkLogMacro( DEBUG, 
                 "igstkTrackerController::RequestStartTracking called...\n" );
  igstkPushInputMacro( StartTracking );
  this->m_StateMachine.ProcessInputs();
}

void
TrackerController::RequestStopTracking()
{
  igstkLogMacro( DEBUG, 
                 "igstkTrackerController::RequestStopTracking called...\n" );
  igstkPushInputMacro( StopTracking );
  this->m_StateMachine.ProcessInputs();
}

void
TrackerController::RequestShutdown()
{
  igstkLogMacro( DEBUG, 
                 "igstkTrackerController::RequestShutdown called...\n" );
  igstkPushInputMacro( StopTracking );
  igstkPushInputMacro( CloseCommunication );
  this->m_StateMachine.ProcessInputs();
}


void 
TrackerController::RequestGetNonReferenceToolList()
{
  igstkLogMacro( DEBUG, 
                 "igstkTrackerController::RequestGetNonReferenceToolList called...\n" );
  igstkPushInputMacro( GetTools );
  this->m_StateMachine.ProcessInputs();
}


void 
TrackerController::RequestGetTool( const std::string &toolName )
{
  igstkLogMacro( DEBUG, 
                 "igstkTrackerController::RequestGetTool called...\n" );
  this->m_RequestedToolName = toolName;
  igstkPushInputMacro( GetTool );
  this->m_StateMachine.ProcessInputs();
}


void 
TrackerController::RequestGetReferenceTool()
{
  igstkLogMacro( DEBUG, 
                 "igstkTrackerController::RequestGetReferenceTool called...\n" );
  igstkPushInputMacro( GetReferenceTool );
  this->m_StateMachine.ProcessInputs();
}


void 
TrackerController::TrackerInitializeProcessing()
{
  if( this->m_TmpTrackerConfiguration == NULL )
  {
    this->m_ErrorMessage = "Null tracker configuration received.";
    igstkPushInputMacro( Failed );
  }
  else 
  {
    if( dynamic_cast<PolarisVicraTrackerConfiguration *>
      ( this->m_TmpTrackerConfiguration ) )
    {
      this->m_TrackerConfiguration = m_TmpTrackerConfiguration;
      igstkPushInputMacro( PolarisVicraInitialize );
    }
    else if( dynamic_cast<PolarisHybridTrackerConfiguration *>
      ( this->m_TmpTrackerConfiguration ) )
    {
      this->m_TrackerConfiguration = m_TmpTrackerConfiguration;
      igstkPushInputMacro( PolarisHybridInitialize );
    }
    else if( dynamic_cast<AuroraTrackerConfiguration *>
      ( this->m_TmpTrackerConfiguration ) )
    {
      this->m_TrackerConfiguration = m_TmpTrackerConfiguration;
      igstkPushInputMacro( AuroraInitialize );
    }
    #ifdef IGSTKSandbox_USE_MicronTracker
    else if( dynamic_cast<MicronTrackerConfiguration *>
      ( this->m_TmpTrackerConfiguration ) )
    {
      this->m_TrackerConfiguration = m_TmpTrackerConfiguration;
      igstkPushInputMacro( MicronInitialize );
    }
    #endif
    else if( dynamic_cast<MedSafeTrackerConfiguration *>
    ( this->m_TmpTrackerConfiguration ) )
    {
      this->m_TrackerConfiguration = m_TmpTrackerConfiguration;
      igstkPushInputMacro( MedSafeInitialize );
    }
    else
    {
      this->m_ErrorMessage = "Unknown tracker configuration type.";
      igstkPushInputMacro( Failed );
    }
  }
 this->m_StateMachine.ProcessInputs();
}


void 
TrackerController::StartTrackingProcessing()
{  
    unsigned long observerID;
              
    observerID = this->m_Tracker->AddObserver( IGSTKErrorEvent(), 
                                               this->m_ErrorObserver );
    m_Tracker->RequestStartTracking();
    this->m_Tracker->RemoveObserver(observerID);

    if( this->m_ErrorObserver->ErrorOccured() )
    {
      this->m_ErrorObserver->GetErrorMessage( this->m_ErrorMessage );
      this->m_ErrorObserver->ClearError();    
      igstkPushInputMacro( Failed );
    }
    else
    {
      igstkPushInputMacro( Succeeded );
    }
    this->m_StateMachine.ProcessInputs();
}


void 
TrackerController::StopTrackingProcessing()
{
  unsigned long observerID;

  observerID = this->m_Tracker->AddObserver( IGSTKErrorEvent(), 
                                             this->m_ErrorObserver );
  this->m_Tracker->RequestStopTracking();
  this->m_Tracker->RemoveObserver(observerID);

  if( this->m_ErrorObserver->ErrorOccured() )
  {
    this->m_ErrorObserver->GetErrorMessage( this->m_ErrorMessage );
    this->m_ErrorObserver->ClearError();
    igstkPushInputMacro( Failed );
   }
  else
  {
    igstkPushInputMacro( Succeeded );
   }
  this->m_StateMachine.ProcessInputs();
}


void 
TrackerController::CloseCommunicationProcessing()
{
  CloseCommunicationErrorEvent evt;
  unsigned long observerID;

  observerID = this->m_Tracker->AddObserver( IGSTKErrorEvent(), 
                                             this->m_ErrorObserver );  
                   //close communication with tracker
  this->m_Tracker->RequestClose();
  this->m_Tracker->RemoveObserver( observerID );

  if( this->m_ErrorObserver->ErrorOccured() )
  {
    this->m_ErrorObserver->GetErrorMessage( this->m_ErrorMessage );
    this->m_ErrorObserver->ClearError();
    igstkPushInputMacro( Failed );
  }
  else
  {
                   //if serial communication, close COM port
   if( this->m_SerialCommunication.IsNotNull() )
   {
     observerID = this->m_SerialCommunication->AddObserver( ClosePortErrorEvent(),
                                                            this->m_ErrorObserver );
     this->m_SerialCommunication->CloseCommunication();
     this->m_SerialCommunication->RemoveObserver(observerID);
     if( this->m_ErrorObserver->ErrorOccured() )
     {
       this->m_ErrorObserver->GetErrorMessage( this->m_ErrorMessage );
       this->m_ErrorObserver->ClearError();
       igstkPushInputMacro( Failed );
     }
     else
     {
       igstkPushInputMacro( Succeeded );
     }
   }
   else //not serial communication
   {
     igstkPushInputMacro( Succeeded );
   }
  }
  this->m_StateMachine.ProcessInputs();
}


bool 
TrackerController::InitializeSerialCommunication()
{
  SerialCommunicatingTrackerConfiguration *serialTrackerConfiguration =
    dynamic_cast<SerialCommunicatingTrackerConfiguration *>( this->m_TrackerConfiguration );
  
                 //create serial communication
  this->m_SerialCommunication = igstk::SerialCommunication::New();

              //observe errors generated by the serial communication
  unsigned long observerID = 
    this->m_SerialCommunication->AddObserver( OpenPortErrorEvent(),
                                              this->m_ErrorObserver );

  this->m_SerialCommunication->SetPortNumber( serialTrackerConfiguration->GetCOMPort() );
  this->m_SerialCommunication->SetParity( serialTrackerConfiguration->GetParity() );
  this->m_SerialCommunication->SetBaudRate( serialTrackerConfiguration->GetBaudRate() );
  this->m_SerialCommunication->SetDataBits( serialTrackerConfiguration->GetDataBits() );
  this->m_SerialCommunication->SetStopBits( serialTrackerConfiguration->GetStopBits() );
  this->m_SerialCommunication->SetHardwareHandshake( serialTrackerConfiguration->GetHandshake() );

  this->m_SerialCommunication->OpenCommunication();
                     //remove the observer, if an error occured we have already
                     //been notified
  this->m_SerialCommunication->RemoveObserver(observerID);

  if( this->m_ErrorObserver->ErrorOccured() )
  {
    this->m_ErrorObserver->GetErrorMessage( this->m_ErrorMessage );
    this->m_ErrorObserver->ClearError();
    return false;
  }
  return true;
}


PolarisTrackerTool::Pointer
TrackerController::InitializePolarisWirelessTool(
  const PolarisWirelessToolConfiguration *toolConfiguration )
{
  PolarisTrackerTool::Pointer trackerTool = PolarisTrackerTool::New();
    
  trackerTool->RequestSelectWirelessTrackerTool();
  trackerTool->RequestSetSROMFileName( toolConfiguration->GetSROMFile() );
  trackerTool->SetCalibrationTransform( 
    toolConfiguration->GetCalibrationTransform() );
  trackerTool->RequestConfigure();
  return trackerTool;
}


PolarisTrackerTool::Pointer
TrackerController::InitializePolarisWiredTool(
  const PolarisWiredToolConfiguration *toolConfiguration )
{
  PolarisTrackerTool::Pointer trackerTool = PolarisTrackerTool::New();
    
  trackerTool->RequestSelectWiredTrackerTool();
  trackerTool->RequestSetPortNumber( toolConfiguration->GetPortNumber() );
  std::string sromFileName = toolConfiguration->GetSROMFile();
  if( !sromFileName.empty() )
  {
    trackerTool->RequestSetSROMFileName( sromFileName );
  }
  trackerTool->SetCalibrationTransform( 
    toolConfiguration->GetCalibrationTransform() );
  trackerTool->RequestConfigure();
  return trackerTool;
}


void 
TrackerController::PolarisVicraInitializeProcessing()
{
  if( !InitializeSerialCommunication() )
  {
    igstkPushInputMacro( Failed );
  }
  else
  {
                                  //create tracker
    igstk::PolarisTracker::Pointer tracker = igstk::PolarisTracker::New();
    this->m_Tracker = tracker; 
                 //don't need to observe this for errors because the 
                 //configuration class ensures that the frequency is valid
    tracker->RequestSetFrequency( this->m_TrackerConfiguration->GetFrequency() );


                //observe all possible errors generated by the tracker
               //(TrackerOpenErrorEvent, TrackerInitializeErrorEvent)               
    unsigned long observerID = tracker->AddObserver( IGSTKErrorEvent(),
                                                     this->m_ErrorObserver );
    tracker->SetCommunication( this->m_SerialCommunication );
    tracker->RequestOpen();
    tracker->RemoveObserver(observerID);

    if( this->m_ErrorObserver->ErrorOccured() )
    {
      this->m_ErrorObserver->GetErrorMessage( this->m_ErrorMessage );
      this->m_ErrorObserver->ClearError();      
      this->m_SerialCommunication->CloseCommunication();
      igstkPushInputMacro( Failed );
    }
    else   //attach the tools 
    {
      std::map<std::string, TrackerToolConfiguration *> toolConfigurations = 
        this->m_TrackerConfiguration->m_TrackerToolList;
                   //attach tools
      std::map<std::string, TrackerToolConfiguration *>::const_iterator it;
      std::map<std::string, TrackerToolConfiguration *>::const_iterator toolConfigEnd =
        toolConfigurations.end();      
      TrackerTool::Pointer currentTool;
      PolarisWirelessToolConfiguration * currentToolConfig;

      for( it = toolConfigurations.begin(); it!=toolConfigEnd; it++ )
      {
        currentToolConfig = 
          static_cast<PolarisWirelessToolConfiguration *>( it->second );
        currentTool = InitializePolarisWirelessTool( currentToolConfig );
        this->m_Tools.insert(
          std::pair<std::string, TrackerTool::Pointer>( it->first, currentTool ) );
        currentTool->RequestAttachToTracker( tracker );
      }
                      //add the reference if we have one
      TrackerToolConfiguration* referenceToolConfiguration = 
        this->m_TrackerConfiguration->m_ReferenceTool;
      if( referenceToolConfiguration )
      {
        currentToolConfig = 
          static_cast<PolarisWirelessToolConfiguration *>( referenceToolConfiguration );

        currentTool = InitializePolarisWirelessTool( currentToolConfig );
        this->m_ReferenceTool = currentTool;
        currentTool->RequestAttachToTracker( tracker );
        tracker->RequestSetReferenceTool( currentTool );
      }
      igstkPushInputMacro( Succeeded );
    }
  }  
  this->m_StateMachine.ProcessInputs();
}


void 
TrackerController::PolarisHybridInitializeProcessing()
{
  if( !InitializeSerialCommunication() )
  {
    igstkPushInputMacro( Failed );
  }
  else
  {
                                  //create tracker
    igstk::PolarisTracker::Pointer tracker = igstk::PolarisTracker::New();
    this->m_Tracker = tracker; 
                 //don't need to observe this for errors because the 
                 //configuration class ensures that the frequency is valid
    tracker->RequestSetFrequency( this->m_TrackerConfiguration->GetFrequency() );


                //observe all possible errors generated by the tracker
               //(TrackerOpenErrorEvent, TrackerInitializeErrorEvent)               
    unsigned long observerID = tracker->AddObserver( IGSTKErrorEvent(),
                                                     this->m_ErrorObserver );
    tracker->SetCommunication( this->m_SerialCommunication );
    tracker->RequestOpen();
    tracker->RemoveObserver(observerID);

    if( this->m_ErrorObserver->ErrorOccured() )
    {
      this->m_ErrorObserver->GetErrorMessage( this->m_ErrorMessage );
      this->m_ErrorObserver->ClearError();      
      this->m_SerialCommunication->CloseCommunication();
      igstkPushInputMacro( Failed );
    }
    else   //attach the tools 
    {
      std::map<std::string, TrackerToolConfiguration *> toolConfigurations = 
        this->m_TrackerConfiguration->m_TrackerToolList;
                   //attach tools
      std::map<std::string, TrackerToolConfiguration *>::const_iterator it;
      std::map<std::string, TrackerToolConfiguration *>::const_iterator toolConfigEnd =
        toolConfigurations.end();      
      TrackerTool::Pointer trackerTool;
      PolarisWirelessToolConfiguration * wirelessToolConfig;
      PolarisWiredToolConfiguration * wiredToolConfig;

      for( it = toolConfigurations.begin(); it!=toolConfigEnd; it++ )
      {
        if( ( wirelessToolConfig = 
             dynamic_cast<PolarisWirelessToolConfiguration *>( it->second ) ) )
        {
          trackerTool = InitializePolarisWirelessTool( wirelessToolConfig );
        }
        else 
        {
          wiredToolConfig = 
            dynamic_cast<PolarisWiredToolConfiguration *>( it->second );
          trackerTool = InitializePolarisWiredTool( wiredToolConfig );
        }
        this->m_Tools.insert(
          std::pair<std::string, TrackerTool::Pointer>( it->first, trackerTool ) );
        trackerTool->RequestAttachToTracker( tracker );
      }
                      //add the reference if we have one
      TrackerToolConfiguration* referenceToolConfiguration = 
        this->m_TrackerConfiguration->m_ReferenceTool;
      if( referenceToolConfiguration )
      {
        if ( ( wirelessToolConfig = 
          dynamic_cast<PolarisWirelessToolConfiguration *>( referenceToolConfiguration ) ) )
        {
          trackerTool = InitializePolarisWirelessTool( wirelessToolConfig );
        }
        else
        {
          wiredToolConfig = 
              dynamic_cast<PolarisWiredToolConfiguration *>( referenceToolConfiguration );
          trackerTool = InitializePolarisWiredTool( wiredToolConfig );
        }
        this->m_ReferenceTool = trackerTool;
        trackerTool->RequestAttachToTracker( tracker );
        tracker->RequestSetReferenceTool( trackerTool );
      }
      igstkPushInputMacro( Succeeded );
    }
  }  
  this->m_StateMachine.ProcessInputs();
}


AuroraTrackerTool::Pointer 
TrackerController::InitializeAuroraTool(
    const AuroraToolConfiguration *toolConfiguration )
{
  AuroraTrackerTool::Pointer trackerTool = AuroraTrackerTool::New();
  bool is5DOF = toolConfiguration->GetIs5DOF();

  if( is5DOF )
    trackerTool->RequestSelect5DOFTrackerTool();
  else
    trackerTool->RequestSelect6DOFTrackerTool();

  trackerTool->RequestSetPortNumber( toolConfiguration->GetPortNumber() );

  if( is5DOF )
    trackerTool->RequestSetChannelNumber( toolConfiguration->GetChannelNumber() );

  std::string sromFileName = toolConfiguration->GetSROMFile();
  if( !sromFileName.empty() )
  {
    trackerTool->RequestSetSROMFileName( sromFileName );
  }
  trackerTool->SetCalibrationTransform( 
    toolConfiguration->GetCalibrationTransform() );

  trackerTool->RequestConfigure();
  return trackerTool;
}


void 
TrackerController::AuroraInitializeProcessing()
{
  if( !InitializeSerialCommunication() )
  {
    igstkPushInputMacro( Failed );
  }
  else
  {
                                  //create tracker
    igstk::AuroraTracker::Pointer tracker = igstk::AuroraTracker::New();
    this->m_Tracker = tracker; 
                 //don't need to observe this for errors because the 
                 //configuration class ensures that the frequency is valid
    tracker->RequestSetFrequency( this->m_TrackerConfiguration->GetFrequency() );


                //observe all possible errors generated by the tracker
               //(TrackerOpenErrorEvent, TrackerInitializeErrorEvent)               
    unsigned long observerID = tracker->AddObserver( IGSTKErrorEvent(),
                                                     this->m_ErrorObserver );
    tracker->SetCommunication( this->m_SerialCommunication );
    tracker->RequestOpen();
    tracker->RemoveObserver(observerID);

    if( this->m_ErrorObserver->ErrorOccured() )
    {
      this->m_ErrorObserver->GetErrorMessage( this->m_ErrorMessage );
      this->m_ErrorObserver->ClearError();      
      this->m_SerialCommunication->CloseCommunication();
      igstkPushInputMacro( Failed );
    }
    else   //attach the tools 
    {
      std::map<std::string, TrackerToolConfiguration *> toolConfigurations = 
        this->m_TrackerConfiguration->m_TrackerToolList;
                   //attach tools
      std::map<std::string, TrackerToolConfiguration *>::const_iterator it;
      std::map<std::string, TrackerToolConfiguration *>::const_iterator toolConfigEnd =
        toolConfigurations.end();      
      TrackerTool::Pointer currentTool;
      AuroraToolConfiguration * currentToolConfig;

      for( it = toolConfigurations.begin(); it!=toolConfigEnd; it++ )
      {
        currentToolConfig = 
          static_cast<AuroraToolConfiguration *>( it->second );
        currentTool = InitializeAuroraTool( currentToolConfig );
        this->m_Tools.insert(
          std::pair<std::string, TrackerTool::Pointer>( it->first, currentTool ) );
        currentTool->RequestAttachToTracker( tracker );
      }
                      //add the reference if we have one
      TrackerToolConfiguration* referenceToolConfiguration = 
        this->m_TrackerConfiguration->m_ReferenceTool;
      if( referenceToolConfiguration )
      {
        currentToolConfig = 
          static_cast<AuroraToolConfiguration *>( referenceToolConfiguration );

        currentTool = InitializeAuroraTool( currentToolConfig );
        this->m_ReferenceTool = currentTool;
        currentTool->RequestAttachToTracker( tracker );
        tracker->RequestSetReferenceTool( currentTool );
      }
      igstkPushInputMacro( Succeeded );
    }
  }  
  this->m_StateMachine.ProcessInputs();
}


#ifdef IGSTKSandbox_USE_MicronTracker

MicronTrackerTool::Pointer TrackerController::InitializeMicronTool(
    const MicronToolConfiguration *toolConfiguration )
{
  MicronTrackerTool::Pointer trackerTool = MicronTrackerTool::New();
 
  trackerTool->RequestSetMarkerName( toolConfiguration->GetToolName() );
 
  trackerTool->SetCalibrationTransform( 
    toolConfiguration->GetCalibrationTransform() );
  trackerTool->RequestConfigure();
  return trackerTool;
}

#endif


void TrackerController::MicronInitializeProcessing()
{
#ifdef IGSTKSandbox_USE_MicronTracker
                  //create tracker
  igstk::MicronTracker::Pointer tracker = igstk::MicronTracker::New();
  this->m_Tracker = tracker; 
                 //don't need to observe this for errors because the 
                 //configuration class ensures that the frequency is valid
  tracker->RequestSetFrequency( this->m_TrackerConfiguration->GetFrequency() );

  MicronTrackerConfiguration *trackerConfiguration =
    dynamic_cast<MicronTrackerConfiguration *>( this->m_TrackerConfiguration );
  
  tracker->SetCameraCalibrationFilesDirectory( 
    trackerConfiguration->GetCameraCalibrationFileDirectory() );

  tracker->SetInitializationFile(
    trackerConfiguration->GetInitializationFile() );
    
  tracker->SetMarkerTemplatesDirectory( 
    trackerConfiguration->GetTemplatesDirectory() );
    
  unsigned long observerID = tracker->AddObserver( IGSTKErrorEvent(),
                                                   this->m_ErrorObserver );
  tracker->RequestOpen();
  tracker->RemoveObserver(observerID);    
  
  if( this->m_ErrorObserver->ErrorOccured() )
  {
    this->m_ErrorObserver->GetErrorMessage( this->m_ErrorMessage );
    this->m_ErrorObserver->ClearError();
    igstkPushInputMacro( Failed );
  }
  else   //attach the tools and start communication 
  {
    std::map<std::string, TrackerToolConfiguration *> toolConfigurations = 
        this->m_TrackerConfiguration->m_TrackerToolList;
                          //attach tools
    std::map<std::string, TrackerToolConfiguration *>::const_iterator it;
    std::map<std::string, TrackerToolConfiguration *>::const_iterator toolConfigEnd =
      toolConfigurations.end();
    TrackerTool::Pointer trackerTool;
    MicronToolConfiguration * currentToolConfig;

    for(it = toolConfigurations.begin(); it!=toolConfigEnd; it++)
    {
      currentToolConfig = static_cast<MicronToolConfiguration *>(it->second);

      trackerTool = InitializeMicronTool( currentToolConfig );      
      this->m_Tools.insert(
          std::pair<std::string, TrackerTool::Pointer>( it->first, trackerTool ) );
      trackerTool->RequestAttachToTracker( tracker );
    }
                      //add the reference if we have one
    TrackerToolConfiguration* referenceToolConfiguration = 
      this->m_TrackerConfiguration->m_ReferenceTool;
    if( referenceToolConfiguration )
    {
      currentToolConfig = 
        static_cast<MicronToolConfiguration *>( referenceToolConfiguration );

      trackerTool = InitializeMicronTool( currentToolConfig );   
      this->m_ReferenceTool = trackerTool;
      trackerTool->RequestAttachToTracker( tracker );
      tracker->RequestSetReferenceTool( trackerTool );
    }
    igstkPushInputMacro( Succeeded );
  }
  this->m_StateMachine.ProcessInputs();
#else
  igstkPushInputMacro( Failed );
  this->m_StateMachine.ProcessInputs();
#endif
}

/*
FlockOfBirdsTrackerTool::Pointer 
TrackerController::InitializeMedSafeTool(
    const MedSafeToolConfiguration *toolConfiguration )
{
  FlockOfBirdsTrackerTool::Pointer trackerTool = FlockOfBirdsTrackerTool::New();

  trackerTool->RequestSetBirdName( toolConfiguration->GetToolName() );
  trackerTool->SetCalibrationTransform( 
    toolConfiguration->GetCalibrationTransform() );
  trackerTool->RequestConfigure();
  return trackerTool;
}
*/

void 
TrackerController::MedSafeInitializeProcessing()
{
   //see implementation below
}

/*
void 
TrackerController::MedSafeInitializeProcessing()
{
  if( !InitializeSerialCommunication() )
  {
    igstkPushInputMacro( Failed );
  }
  else
  {
    this->m_SerialCommunication->Print(std::cout, 0);

                                  //create tracker
    igstk::FlockOfBirdsTracker::Pointer tracker = igstk::FlockOfBirdsTracker::New();
    this->m_Tracker = tracker; 
                 //don't need to observe this for errors because the 
                 //configuration class ensures that the frequency is valid
    tracker->RequestSetFrequency( this->m_TrackerConfiguration->GetFrequency() );


                //observe all possible errors generated by the tracker
               //(TrackerOpenErrorEvent, TrackerInitializeErrorEvent,
               //TrackerStartTrackingErrorEvent)
    unsigned long observerID = tracker->AddObserver( IGSTKErrorEvent(),
                                                     this->m_ErrorObserver );

    tracker->SetCommunication( this->m_SerialCommunication );

    tracker->Print(std::cout);

    tracker->RequestOpen();
    if( this->m_ErrorObserver->ErrorOccured() )
    {
      this->m_ErrorObserver->GetErrorMessage( this->m_ErrorMessage );
      this->m_ErrorObserver->ClearError();
      tracker->RemoveObserver(observerID);
      this->m_SerialCommunication->CloseCommunication();
      igstkPushInputMacro( Failed );
    }
    else   //attach the tools and start communication 
    {
      std::map<std::string, TrackerToolConfiguration *> toolConfigurations = 
        this->m_TrackerConfiguration->m_TrackerToolList;
                   //attach tools
      std::map<std::string, TrackerToolConfiguration *>::const_iterator it;
      std::map<std::string, TrackerToolConfiguration *>::const_iterator toolConfigEnd =
        toolConfigurations.end();
      TrackerTool::Pointer trackerTool;
      MedSafeToolConfiguration * currentToolConfig;

      for(it = toolConfigurations.begin(); it!=toolConfigEnd; it++)
      {
        currentToolConfig = static_cast<MedSafeToolConfiguration *>(it->second);

        trackerTool = InitializeMedSafeTool( currentToolConfig );
        this->m_Tools.insert(
          std::pair<std::string, TrackerTool::Pointer>( it->first, trackerTool ) );
        trackerTool->RequestAttachToTracker( tracker );
      }
                      //add the reference if we have one
      TrackerToolConfiguration* referenceToolConfiguration = 
        this->m_TrackerConfiguration->m_ReferenceTool;
      if( referenceToolConfiguration )
      {
        currentToolConfig = 
          static_cast<MedSafeToolConfiguration *>( referenceToolConfiguration );

        trackerTool = InitializeMedSafeTool( currentToolConfig );   
        this->m_ReferenceTool = trackerTool;
        trackerTool->RequestAttachToTracker( tracker );
        tracker->RequestSetReferenceTool( trackerTool );
      }
      tracker->RequestStartTracking();
      if( this->m_ErrorObserver->ErrorOccured() )
      {
        this->m_ErrorObserver->GetErrorMessage( this->m_ErrorMessage );
        this->m_ErrorObserver->ClearError();
        tracker->RemoveObserver( observerID );
        tracker->RequestClose();
        this->m_SerialCommunication->CloseCommunication();
        igstkPushInputMacro( Failed );
      }
      else
      {
        tracker->RemoveObserver(observerID);
        igstkPushInputMacro( Succeeded );
      }
      //m_Tracker->RemoveObserver(observerID);
      //igstkPushInputMacro( Succeeded );
    }
  }
  this->m_StateMachine.ProcessInputs();
}
*/


void 
TrackerController::GetToolsProcessing()
{
  igstkLogMacro( DEBUG,
                  "igstk::TrackerController::"
                  "GetToolsProcessing called...\n");
  RequestToolsEvent evt;
  evt.Set( this->m_Tools );
  this->InvokeEvent( evt );
}


void 
TrackerController::GetToolProcessing()
{
  igstkLogMacro( DEBUG,
                  "igstk::TrackerController::"
                  "GetToolProcessing called...\n");  

  RequestToolEvent sevt;
  RequestToolErrorEvent fevt;
   
  std::map<std::string, TrackerTool::Pointer>::iterator it =
    this->m_Tools.find( this->m_RequestedToolName );
 
  this->m_RequestedToolName.clear();

  if( it == this->m_Tools.end() ) 
  {  
    this->InvokeEvent( fevt );
  }
  else
  {
    sevt.Set( it->second );
    this->InvokeEvent( sevt );
  }
}


void 
TrackerController::GetReferenceToolProcessing()
{
  igstkLogMacro( DEBUG,
                  "igstk::TrackerController::"
                  "GetReferenceToolProcessing called...\n");
  RequestToolEvent evt;
  evt.Set( this->m_ReferenceTool );
  this->InvokeEvent( evt );
}


void 
TrackerController::ReportInitializationSuccessProcessing()
{
  igstkLogMacro( DEBUG,
                  "igstk::TrackerController::"
                  "ReportInitializationSuccess called...\n");
  this->m_Tracker->AddObserver( igstk::TrackerUpdateStatusEvent(),
                                this->m_TrackerUpdateStatusObserver );
  this->m_Tracker->AddObserver( igstk::TrackerUpdateStatusErrorEvent(),
                                this->m_TrackerUpdateStatusObserver );  
  this->InvokeEvent( InitializeEvent() );
}


void
TrackerController::ReportInitializationFailureProcessing()
{
  igstkLogMacro( DEBUG,
                  "igstk::TrackerController::"
                  "ReportInitializationFailureProcessing called...\n");
  InitializeErrorEvent evt; 
  evt.Set( this->m_ErrorMessage );
  this->InvokeEvent( evt );
}


void 
TrackerController::ReportStartTrackingSuccessProcessing()
{
  igstkLogMacro( DEBUG,
                  "igstk::TrackerController::"
                  "ReportStartTrackingSuccessProcessing called...\n");
  this->InvokeEvent( TrackerStartTrackingEvent() );
}


void 
TrackerController::ReportStartTrackingFailureProcessing()
{
  igstkLogMacro( DEBUG,
                  "igstk::TrackerController::"
                  "ReportStartTrackingFailureProcessing called...\n");
  TrackerStartTrackingErrorEvent evt; 
  evt.Set( this->m_ErrorMessage );
  this->InvokeEvent( evt );
}


void 
TrackerController::ReportStopTrackingSuccessProcessing()
{
  igstkLogMacro( DEBUG,
                  "igstk::TrackerController::"
                  "ReportStopTrackingSuccessProcessing called...\n");
  this->InvokeEvent( TrackerStopTrackingEvent() );
}


void 
TrackerController::ReportStopTrackingFailureProcessing()
{
  igstkLogMacro( DEBUG,
                  "igstk::TrackerController::"
                  "ReportStopTrackingFailureProcessing called...\n");
  TrackerStopTrackingErrorEvent evt; 
  evt.Set( this->m_ErrorMessage );
  this->InvokeEvent( evt );
}


void 
TrackerController::ReportCloseCommunicationFailureProcessing()
{
  igstkLogMacro( DEBUG,
                  "igstk::TrackerController::"
                  "ReportCloseCommunicationFailureProcessing called...\n");
  CloseCommunicationErrorEvent evt; 
  evt.Set( this->m_ErrorMessage );
  this->InvokeEvent( evt );
}


void 
TrackerController::ReportCloseCommunicationSuccessProcessing()
{
  igstkLogMacro( DEBUG,
                  "igstk::TrackerController::"
                  "ReportCloseCommunicationSuccessProcessing called...\n");
  this->InvokeEvent( CloseCommunicationEvent() );
}


void 
TrackerController::ReportInvalidRequestProcessing()
{
  igstkLogMacro( DEBUG, "igstk::TrackerController::"
                 "ReportInvalidRequestProcessing called...\n");
  this->InvokeEvent(InvalidRequestErrorEvent());
}


TrackerController::ErrorObserver::ErrorObserver() : m_ErrorOccured( false )
{                              //serial communication errors
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( OpenPortErrorEvent().GetEventName(),
                                       "Error opening com port." ) );
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( ClosePortErrorEvent().GetEventName(),
                                        "Error closing com port." ) );
                              //tracker errors
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( TrackerOpenErrorEvent().GetEventName(),
                                        "Error opening tracker communication." ) );
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( TrackerInitializeErrorEvent().GetEventName(),
                                        "Error initializing tracker." ) );
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( TrackerStartTrackingErrorEvent().GetEventName(),
                                        "Error starting tracking." ) );
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( TrackerStopTrackingErrorEvent().GetEventName(),
                                        "Error stopping tracking." ) );
  this->m_ErrorEvent2ErrorMessage.insert(
    std::pair<std::string,std::string>( TrackerCloseErrorEvent().GetEventName(),
                                        "Error closing tracker communication." ) );
}

void 
TrackerController::ErrorObserver::Execute( 
  const itk::Object *caller, 
  const itk::EventObject & event ) throw (std::exception)
{
  std::map<std::string,std::string>::iterator it;
  std::string className = event.GetEventName();
  it = this->m_ErrorEvent2ErrorMessage.find(className);

  if( it != this->m_ErrorEvent2ErrorMessage.end() )
  {
    this->m_ErrorOccured = true;
    this->m_ErrorMessage = (*it).second;
  }
  //if the event we got wasn't in the error events map then we
  //silently ignore it
}

void 
TrackerController::ErrorObserver::Execute( 
  itk::Object *caller, 
  const itk::EventObject & event ) throw (std::exception)
{
  const itk::Object * constCaller = caller;
  this->Execute(constCaller, event);
}


void
TrackerController::TrackerUpdateObserver::SetParent( TrackerController *parent )
{
  this->m_parent = parent;
}


void
TrackerController::TrackerUpdateObserver::Execute( 
  const itk::Object *caller, 
  const itk::EventObject & event ) throw ( std::exception )
{
  if( igstk::TrackerUpdateStatusEvent().CheckEvent( &event ) ||
      igstk::TrackerUpdateStatusErrorEvent().CheckEvent( &event ) )
    this->m_parent->InvokeEvent( event );
}


void
TrackerController::TrackerUpdateObserver::Execute( 
  itk::Object *caller, 
  const itk::EventObject & event ) throw ( std::exception )
{
  const itk::Object * constCaller = caller;
  this->Execute(constCaller, event);
}

}//end namespace igstk

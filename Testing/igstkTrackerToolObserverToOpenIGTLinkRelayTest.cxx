/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkTrackerToolObserverToOpenIGTLinkRelayTest.cxx
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
// Warning about: identifier was truncated to '255' characters 
// in the debug information (MVC6.0 Debug)
#pragma warning( disable : 4786 )
#endif

#include <iostream>

#include "igstkRealTimeClock.h"
#include "igstkCircularSimulatedTracker.h"
#include "igstkSimulatedTrackerTool.h"
#include "igstkTrackerToolObserverToOpenIGTLinkRelay.h"

int igstkTrackerToolObserverToOpenIGTLinkRelayTest( int argc, char * argv [] )
{

  if( argc < 6 )
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " hostname portnumber numberOfTransformsToSend angularSpeed(degrees/second) trackerFrequency(Hz)" << std::endl;
    return EXIT_FAILURE;
    }

  igstk::RealTimeClock::Initialize();

  typedef igstk::CircularSimulatedTracker               TrackerType;
  typedef igstk::SimulatedTrackerTool                   TrackerToolType;
  typedef igstk::TrackerToolObserverToOpenIGTLinkRelay  ObserverType;

  TrackerType::Pointer      tracker      = TrackerType::New();
  TrackerToolType::Pointer  trackerTool  = TrackerToolType::New();
  ObserverType::Pointer     toolObserver = ObserverType::New();

  tracker->RequestOpen();

  tracker->SetRadius( 10.0 );
  tracker->SetAngularSpeed( atof( argv[4] ) );
  tracker->RequestSetFrequency( atof( argv[5] ) );

  trackerTool->RequestSetName("Tool_1");
  trackerTool->RequestConfigure();
  trackerTool->RequestAttachToTracker( tracker );

  toolObserver->RequestSetTrackerTool( trackerTool );
  toolObserver->RequestSetHostName( argv[1] );
  toolObserver->RequestSetPort( atoi( argv[2] ) );
  toolObserver->RequestStart();

  tracker->RequestStartTracking();

  const unsigned int numberOfTransformsToSend = atoi( argv[3] );

  for( unsigned int i = 0; i < numberOfTransformsToSend; i++ )
    {
    igstk::PulseGenerator::Sleep(10);
    igstk::PulseGenerator::CheckTimeouts();
    }

  tracker->RequestStopTracking();
  tracker->RequestReset();
  tracker->RequestClose();

  return EXIT_SUCCESS;
}

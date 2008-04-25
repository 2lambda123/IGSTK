/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    PivotCalibrationFLTKWidgetExample.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "PivotCalibrationFLTKWidgetExample.h"
#include <FL/fl_ask.H>

#include "igstkSerialCommunication.h"

//
// settings for the serial communication
//
const igstk::SerialCommunication::ParityType
PivotCalibrationFLTKWidgetExample::PARITY =
  igstk::SerialCommunication::NoParity;

const igstk::SerialCommunication::BaudRateType
PivotCalibrationFLTKWidgetExample::BAUD_RATE =
  igstk::SerialCommunication::BaudRate115200;

const igstk::SerialCommunication::DataBitsType
PivotCalibrationFLTKWidgetExample::DATA_BITS =
  igstk::SerialCommunication::DataBits8;

const igstk::SerialCommunication::StopBitsType
PivotCalibrationFLTKWidgetExample::STOP_BITS =
  igstk::SerialCommunication::StopBits1;

const igstk::SerialCommunication::HandshakeType
PivotCalibrationFLTKWidgetExample::HAND_SHAKE =
  igstk::SerialCommunication::HandshakeOff;


PivotCalibrationFLTKWidgetExample::PivotCalibrationFLTKWidgetExample()
: PivotCalibrationFLTKWidgetExampleUI(), m_initialized(false)
{
            //create error observer
  this->m_errorObserver = TrackingErrorObserver::New();

  //create serial communication
  this->m_serialCommunication = igstk::SerialCommunication::New();

              //observe errors generated by the serial communication
  this->m_serialCommunication->AddObserver( igstk::OpenPortErrorEvent(),
                                            this->m_errorObserver );
  this->m_serialCommunication->AddObserver( igstk::ClosePortErrorEvent(),
                                            this->m_errorObserver );

              //create tracker
  this->m_tracker = igstk::PolarisTracker::New();

                 //observe errors generated by the tracker
  this->m_tracker->AddObserver( igstk::TrackerOpenErrorEvent(),
                                this->m_errorObserver );
  this->m_tracker->AddObserver( igstk::TrackerInitializeErrorEvent(),
                                this->m_errorObserver );
  this->m_tracker->AddObserver( igstk::TrackerStartTrackingErrorEvent(),
                                this->m_errorObserver );
  this->m_tracker->AddObserver( igstk::TrackerStopTrackingErrorEvent(),
                                this->m_errorObserver );
  this->m_tracker->AddObserver( igstk::TrackerUpdateStatusErrorEvent(),
                                this->m_errorObserver );
  this->m_tracker->AddObserver( igstk::TrackerCloseErrorEvent(),
                                this->m_errorObserver );
  this->m_tracker->AddObserver( igstk::TrackerUpdateStatusErrorEvent(),
                                this->m_errorObserver );

}

void
PivotCalibrationFLTKWidgetExample::InitializeTrackingAndCalibration()
{
              //gather the information from the UI
  typedef igstk::SerialCommunication::PortNumberType PortNumberType;
  PortNumberType comPort =
   static_cast<PortNumberType> ( this->m_comPortChoice->value() );
  unsigned int toolPort =
            static_cast<unsigned int>( this->m_toolPortChoice->value() );
  unsigned int delay =
            static_cast<unsigned int>( this->m_delaySlider->value() );
  unsigned int numberOfFrames =
            static_cast<unsigned int>( this->m_numberOfFramesSlider->value() );
  std::string sromFileName = this->m_SROMFileNameTextField->value();

      //check that the user entered valid data, wireless tool requires an
      //SROM file
  if( sromFileName.empty() && toolPort == 0 )
  {
    fl_alert( "Missing SROM file for wireless tool." );
    fl_beep( FL_BEEP_ERROR );
    return;
  }

              //if already initialized, shutdown everything and reinitialize
  if( this->m_initialized )
  {
    this->m_initialized = false;
    this->m_tracker->RequestStopTracking();
    if( this->m_errorObserver->Error() )
    {
      this->m_errorObserver->ClearError();
      return;
    }
    this->m_tracker->RequestClose();
    if( this->m_errorObserver->Error() )
    {
      this->m_errorObserver->ClearError();
      return;
    }
    this->m_serialCommunication->CloseCommunication();
    if( this->m_errorObserver->Error() )
    {
      this->m_errorObserver->ClearError();
      return;
    }
  }

  this->m_serialCommunication->SetPortNumber( comPort );
  this->m_serialCommunication->SetParity( PARITY );
  this->m_serialCommunication->SetBaudRate( BAUD_RATE );
  this->m_serialCommunication->SetDataBits( DATA_BITS );
  this->m_serialCommunication->SetStopBits( STOP_BITS );
  this->m_serialCommunication->SetHardwareHandshake( HAND_SHAKE );


                 // create tracked tool and configure it
  this->m_tool = igstk::PolarisTrackerTool::New();
         //different configuration for wired and wireless tools
         //toolPort == 0 is wireless all others are actual ports
  if( toolPort == 0) 
  {
    this->m_tool->RequestSelectWirelessTrackerTool();
    this->m_tool->RequestSetSROMFileName(sromFileName);
  }
  else  //we have a wired tool
  {
    this->m_tool->RequestSelectWiredTrackerTool();
                 //the physical ports start at 1, inside IGSTK we refer to 
                //them from 0
    this->m_tool->RequestSetPortNumber( toolPort - 1 );            
               //SROM file is optional 
    if ( !sromFileName.empty() )
    {
      this->m_tool->RequestSetSROMFileName(sromFileName);
    }
  }
  this->m_tool->RequestConfigure();

         //open serial communication
  this->m_serialCommunication->OpenCommunication();
  if( this->m_errorObserver->Error() )
  {
    this->m_errorObserver->ClearError();
    return;
  }
  this->m_tracker->SetCommunication( this->m_serialCommunication );

           //open tracker communication
  this->m_tracker->RequestOpen();
  if( this->m_errorObserver->Error() )
  {
    this->m_errorObserver->ClearError();
    return;
  }
                      //attach the tracker tool
  this->m_tool->RequestAttachToTracker( this->m_tracker );
  if( this->m_errorObserver->Error() )
  {
    this->m_errorObserver->ClearError();
    return;
  }
               //start tracking
  this->m_tracker->RequestStartTracking();
  if( this->m_errorObserver->Error() )
  {
    this->m_errorObserver->ClearError();
    return;
  }

  this->m_initialized = true;


  this->m_pivotCalibrationFLTKWidget->RequestSetDelay( delay );

  igstk::Tracker * genericTracker = this->m_tracker.GetPointer();
  igstk::TrackerTool * genericTrackerTool = this->m_tool.GetPointer();

  this->m_pivotCalibrationFLTKWidget->RequestInitialize(numberOfFrames,
                                                        genericTracker,
                                                        genericTrackerTool );
}

void
PivotCalibrationFLTKWidgetExample::TrackingErrorObserver::Execute(
  itk::Object *caller,
  const itk::EventObject & event )
{
  std::map<std::string,std::string>::iterator it;
  std::string className = event.GetEventName();
  it = this->m_ErrorEvent2ErrorMessage.find( className );
  if( it != this->m_ErrorEvent2ErrorMessage.end() )
    {
    this->m_ErrorOccured = true;
    fl_alert( (*it).second.c_str() );
    fl_beep( FL_BEEP_ERROR );
    }
}

void
PivotCalibrationFLTKWidgetExample::TrackingErrorObserver::Execute(
  const itk::Object *caller,
  const itk::EventObject & event )
{
  const itk::Object * constCaller = caller;
  this->Execute( constCaller, event );
}

void
PivotCalibrationFLTKWidgetExample::TrackingErrorObserver::ClearError()
{
  this->m_ErrorOccured = false;
}

bool
PivotCalibrationFLTKWidgetExample::TrackingErrorObserver::Error()
{
  return this->m_ErrorOccured;
}

PivotCalibrationFLTKWidgetExample
::TrackingErrorObserver::TrackingErrorObserver()
: m_ErrorOccured( false )
{
  //serial communication errors
  this->m_ErrorEvent2ErrorMessage.insert(
      std::pair<std::string,std::string>(
      igstk::OpenPortErrorEvent().GetEventName(), "Error opening com port." ) );

  this->m_ErrorEvent2ErrorMessage.insert(
      std::pair<std::string,std::string>(
      igstk::ClosePortErrorEvent().GetEventName(), "Error closing com port." ) );

  //tracker errors
  this->m_ErrorEvent2ErrorMessage.insert(
      std::pair<std::string,std::string>(
      igstk::TrackerOpenErrorEvent().GetEventName(),
      "Error opening tracker communication." ) );

  this->m_ErrorEvent2ErrorMessage.insert(
      std::pair<std::string,std::string>(
      igstk::TrackerInitializeErrorEvent().GetEventName(),
     "Error initializing tracker." ) );

  this->m_ErrorEvent2ErrorMessage.insert(
      std::pair<std::string,std::string>(
      igstk::TrackerStartTrackingErrorEvent().GetEventName(),
     "Error starting tracking." ) );

  this->m_ErrorEvent2ErrorMessage.insert(
      std::pair<std::string,std::string>(
      igstk::TrackerStopTrackingErrorEvent().GetEventName(),
     "Error stopping tracking." ) );

  this->m_ErrorEvent2ErrorMessage.insert(
      std::pair<std::string,std::string>(
      igstk::TrackerCloseErrorEvent().GetEventName(),
     "Error closing tracker communication." ) );

  this->m_ErrorEvent2ErrorMessage.insert(
      std::pair<std::string,std::string>(
      igstk::TrackerUpdateStatusErrorEvent().GetEventName(),
     "Error updating transformations from tracker." ) );

  this->m_ErrorEvent2ErrorMessage.insert(
      std::pair<std::string,std::string>(
      igstk::AttachingTrackerToolToTrackerErrorEvent().GetEventName(),
     "Error attaching tool to tracker." ) );
}

/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkReslicerPlaneSpatialObject.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __igstkReslicerPlaneSpatialObject_cxx
#define __igstkReslicerPlaneSpatialObject_cxx

#include "igstkReslicerPlaneSpatialObject.h"
#include "igstkEvents.h"

#include "vtkPlaneSource.h"
#include "vtkPlane.h"
#include "vtkMath.h"
#include "vtkTransform.h"

namespace igstk
{ 

/** Constructor */
ReslicerPlaneSpatialObject
::ReslicerPlaneSpatialObject():m_StateMachine(this)
{
  //Default reslicing mode
  m_ReslicingMode = Orthogonal;

  //Default orientation type
  m_OrientationType = Axial;

  m_BoundingBoxProviderSpatialObject = NULL;
  m_ToolSpatialObject = NULL; 

  //tool spatial object check flag
  m_ToolSpatialObjectSet  = false;

  //Cursor position 
  m_CursorPosition[0] = 0;
  m_CursorPosition[1] = 0;
  m_CursorPosition[2] = 0;

  //tool position 
  m_ToolPosition[0] = 0;
  m_ToolPosition[1] = 0;
  m_ToolPosition[2] = 0;

  m_CursorPositionSetFlag = false;

  //List of states
  igstkAddStateMacro( Initial );
  igstkAddStateMacro( ReslicingModeSet );
  igstkAddStateMacro( OrientationTypeSet );
  igstkAddStateMacro( BoundingBoxProviderSpatialObjectSet );
  igstkAddStateMacro( AttemptingToGetToolTransformWRTImageCoordinateSystem );
  igstkAddStateMacro( AttemptingToSetCursorPosition );
  igstkAddStateMacro( AttemptingToSetBoundingBoxProviderSpatialObject );

  // List of state machine inputs
  igstkAddInputMacro( SetBoundingBoxProviderSpatialObject );
  igstkAddInputMacro( ValidReslicingMode );
  igstkAddInputMacro( InValidReslicingMode );
  igstkAddInputMacro( ValidOrientationType );
  igstkAddInputMacro( InValidOrientationType );
  igstkAddInputMacro( ValidBoundingBoxProviderSpatialObject );
  igstkAddInputMacro( InValidBoundingBoxProviderSpatialObject );
  igstkAddInputMacro( ValidToolSpatialObject );
  igstkAddInputMacro( InValidToolSpatialObject );  
  igstkAddInputMacro( SetCursorPosition );
  igstkAddInputMacro( ValidCursorPosition );
  igstkAddInputMacro( InValidCursorPosition );
  igstkAddInputMacro( GetToolPosition );
  igstkAddInputMacro( GetReslicingPlaneParameters );
  igstkAddInputMacro( GetToolTransformWRTImageCoordinateSystem );
  igstkAddInputMacro( ToolTransformWRTImageCoordinateSystem );
  igstkAddInputMacro( ComputeReslicePlane );

  // List of state machine transitions

  //From Initial
  igstkAddTransitionMacro( Initial, ValidReslicingMode, 
                           ReslicingModeSet, SetReslicingMode );

  igstkAddTransitionMacro( Initial, InValidReslicingMode, 
                           Initial, ReportInvalidReslicingMode );

  igstkAddTransitionMacro( Initial, ValidOrientationType, 
                           Initial, ReportInvalidRequest);

  igstkAddTransitionMacro( Initial, InValidOrientationType, 
                           Initial, ReportInvalidRequest);  

  igstkAddTransitionMacro( Initial, SetCursorPosition, 
                           Initial, ReportInvalidRequest);

  igstkAddTransitionMacro( Initial, SetBoundingBoxProviderSpatialObject, 
                           Initial, ReportInvalidRequest);

  //From ReslicingModeSet
  igstkAddTransitionMacro( ReslicingModeSet, ValidOrientationType, 
                           OrientationTypeSet, SetOrientationType );

  igstkAddTransitionMacro( ReslicingModeSet, InValidOrientationType, 
                           ReslicingModeSet, ReportInvalidOrientationType);

  igstkAddTransitionMacro( ReslicingModeSet, SetCursorPosition, 
                           ReslicingModeSet, ReportInvalidRequest );  

  //From OrientationTypeSet
  igstkAddTransitionMacro( OrientationTypeSet, SetBoundingBoxProviderSpatialObject,
                           AttemptingToSetBoundingBoxProviderSpatialObject, AttemptSetBoundingBoxProviderSpatialObject );

  igstkAddTransitionMacro( OrientationTypeSet, SetCursorPosition, 
                           OrientationTypeSet, ReportInvalidRequest );


  // From AttemptingToSetBoundingBoxProviderSpatialObject
  igstkAddTransitionMacro( AttemptingToSetBoundingBoxProviderSpatialObject, ValidBoundingBoxProviderSpatialObject,
                           BoundingBoxProviderSpatialObjectSet,  SetBoundingBoxProviderSpatialObject ); 

  igstkAddTransitionMacro( AttemptingToSetBoundingBoxProviderSpatialObject, InValidBoundingBoxProviderSpatialObject,
                           OrientationTypeSet,  ReportInvalidBoundingBoxProviderSpatialObject );


  //From BoundingBoxProviderSpatialObjectSet
  igstkAddTransitionMacro( BoundingBoxProviderSpatialObjectSet, ValidOrientationType,
                           BoundingBoxProviderSpatialObjectSet, SetOrientationType );

  igstkAddTransitionMacro( BoundingBoxProviderSpatialObjectSet, InValidOrientationType,
                           BoundingBoxProviderSpatialObjectSet, ReportInvalidOrientationType );

  igstkAddTransitionMacro( BoundingBoxProviderSpatialObjectSet, ValidReslicingMode, 
                           BoundingBoxProviderSpatialObjectSet, SetReslicingMode );

  igstkAddTransitionMacro( BoundingBoxProviderSpatialObjectSet, InValidReslicingMode, 
                           BoundingBoxProviderSpatialObjectSet, ReportInvalidReslicingMode );

  igstkAddTransitionMacro( BoundingBoxProviderSpatialObjectSet, ValidToolSpatialObject,
                           BoundingBoxProviderSpatialObjectSet, SetToolSpatialObject );

  igstkAddTransitionMacro( BoundingBoxProviderSpatialObjectSet, InValidToolSpatialObject,
                           BoundingBoxProviderSpatialObjectSet, ReportInvalidToolSpatialObject );  

  igstkAddTransitionMacro( BoundingBoxProviderSpatialObjectSet, SetCursorPosition,
                           AttemptingToSetCursorPosition, AttemptSetCursorPosition );  

  igstkAddTransitionMacro( BoundingBoxProviderSpatialObjectSet, ComputeReslicePlane,
                           BoundingBoxProviderSpatialObjectSet, ComputeReslicePlane );

  igstkAddTransitionMacro( BoundingBoxProviderSpatialObjectSet, GetToolPosition,
                           BoundingBoxProviderSpatialObjectSet, ReportToolPosition );

  igstkAddTransitionMacro( BoundingBoxProviderSpatialObjectSet, GetReslicingPlaneParameters,
                           BoundingBoxProviderSpatialObjectSet, ReportReslicingPlaneParameters );

  igstkAddTransitionMacro( BoundingBoxProviderSpatialObjectSet, GetToolTransformWRTImageCoordinateSystem,
                           AttemptingToGetToolTransformWRTImageCoordinateSystem,
                                             RequestGetToolTransformWRTImageCoordinateSystem );

  // From AttemptingToSetCursorPosition
  igstkAddTransitionMacro( AttemptingToSetCursorPosition, ValidCursorPosition,
                           BoundingBoxProviderSpatialObjectSet,  SetCursorPosition ); 

  igstkAddTransitionMacro( AttemptingToSetCursorPosition, InValidCursorPosition,
                           BoundingBoxProviderSpatialObjectSet,  ReportInvalidCursorPosition );

  // From AttemptingToGetToolTransformWRTImageCoordinateSystem
  igstkAddTransitionMacro( AttemptingToGetToolTransformWRTImageCoordinateSystem, ToolTransformWRTImageCoordinateSystem,
                           BoundingBoxProviderSpatialObjectSet, ReceiveToolTransformWRTImageCoordinateSystem );

  igstkSetInitialStateMacro( Initial );
  this->m_StateMachine.SetReadyToRun();
} 

/** Destructor */
ReslicerPlaneSpatialObject
::~ReslicerPlaneSpatialObject()  
{  

}

void 
ReslicerPlaneSpatialObject
::RequestSetReslicingMode( ReslicingMode reslicingMode )
{  
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::RequestSetReslicingMode called...\n");

  m_ReslicingModeToBeSet = reslicingMode;
  m_StateMachine.PushInput( m_ValidReslicingModeInput );

  //FIXME: Check conditions for InValidReslicing mode
  m_StateMachine.ProcessInputs();
}

/** Null Operation for a State Machine Transition */
void 
ReslicerPlaneSpatialObject
::NoProcessing()
{
}

void
ReslicerPlaneSpatialObject
::RequestGetToolPosition() 
{
  igstkLogMacro( DEBUG, "igstk::ReslicerPlaneSpatialObject\
                        ::RequestGetToolPosition called...\n");
 
  igstkPushInputMacro( GetToolPosition );
  m_StateMachine.ProcessInputs();
}

void
ReslicerPlaneSpatialObject
::RequestGetReslicingPlaneParameters() 
{
  igstkLogMacro( DEBUG, "igstk::ReslicerPlaneSpatialObject\
                        ::RequestGetReslicingPlaneParameters called...\n");
 
  igstkPushInputMacro( GetReslicingPlaneParameters );
  m_StateMachine.ProcessInputs();
}

void
ReslicerPlaneSpatialObject
::ReportToolPositionProcessing() 
{
  igstkLogMacro( DEBUG, "igstk::ReslicerPlaneSpatialObject\
                        ::ReportToolPositionProcessing called...\n");

  ToolTipPositionEvent event;
  event.Set( m_ToolPosition );
  this->InvokeEvent( event );
}

void
ReslicerPlaneSpatialObject
::ReportReslicingPlaneParametersProcessing() 
{
  igstkLogMacro( DEBUG, "igstk::ReslicerPlaneSpatialObject\
                        ::ReportReslicingPlaneParametersProcessing called...\n");

  ReslicerPlaneCenterEvent pcEvent;
  pcEvent.Set( m_PlaneCenter );
  this->InvokeEvent( pcEvent );

  ReslicerPlaneNormalEvent pnEvent;
  pnEvent.Set( m_PlaneNormal );
  this->InvokeEvent( pnEvent );

}

void 
ReslicerPlaneSpatialObject
::RequestSetCursorPosition( const double *point )
{  
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::RequestSetCursorPosition called...\n");

  m_CursorPositionToBeSet[0] = point[0];
  m_CursorPositionToBeSet[1] = point[1];
  m_CursorPositionToBeSet[2] = point[2];

  m_StateMachine.PushInput( m_SetCursorPositionInput );

  m_StateMachine.ProcessInputs();
}

void 
ReslicerPlaneSpatialObject
::AttemptSetCursorPositionProcessing()
{

  igstkLogMacro( DEBUG, "igstk::ReslicerPlaneSpatialObject\
                        ::AttemptSetCursorPositionProcessing called...\n");

    bool validPosition = false; 

    switch( m_OrientationType )
      {
      case Axial:
        if( m_CursorPositionToBeSet[2] >= m_ImageBounds[4] && 
            m_CursorPositionToBeSet[2] <= m_ImageBounds[5] )
          {
          validPosition = true;
          }
          break;
      case Sagittal:
        if( m_CursorPositionToBeSet[0] >= m_ImageBounds[0] && 
            m_CursorPositionToBeSet[0] <= m_ImageBounds[1] )
          {
          validPosition = true;
          }
        break;
      case Coronal:
        if( m_CursorPositionToBeSet[1] >= m_ImageBounds[2] && 
            m_CursorPositionToBeSet[1] <= m_ImageBounds[3] )
          {
          validPosition = true;
          }
        break;
      default:
          {
          validPosition = false;
          }
      }

    if( validPosition )
      {
      igstkPushInputMacro( ValidCursorPosition );
      }
    else
      {
      igstkPushInputMacro( InValidCursorPosition );
      }

    m_StateMachine.ProcessInputs();
}

void 
ReslicerPlaneSpatialObject
::SetCursorPositionProcessing()
{
  igstkLogMacro( DEBUG, "igstk::ReslicerPlaneSpatialObject\
                        ::SetCursorPosition called...\n");

  m_CursorPosition[0] = m_CursorPositionToBeSet[0];
  m_CursorPosition[1] = m_CursorPositionToBeSet[1];
  m_CursorPosition[2] = m_CursorPositionToBeSet[2];

  m_ToolPosition[0] = m_CursorPosition[0];
  m_ToolPosition[1] = m_CursorPosition[1];
  m_ToolPosition[2] = m_CursorPosition[2];

  //turn on the flag
  m_CursorPositionSetFlag = true;
}

void 
ReslicerPlaneSpatialObject
::SetReslicingModeProcessing()
{
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::SetReslicingModeProcessing called...\n");
  m_ReslicingMode = m_ReslicingModeToBeSet;
}

void 
ReslicerPlaneSpatialObject
::RequestSetOrientationType( OrientationType orientationType )
{  
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::RequestSetOrientationType called...\n");

  m_OrientationTypeToBeSet = orientationType;

  bool validOrientation = true;

  if( m_ReslicingMode == Orthogonal )
    {
    if( m_OrientationTypeToBeSet == OffAxial ||
         m_OrientationTypeToBeSet == OffCoronal ||
         m_OrientationTypeToBeSet == OffSagittal  ||
         m_OrientationTypeToBeSet == PlaneOrientationWithZAxesNormal ||
         m_OrientationTypeToBeSet == PlaneOrientationWithXAxesNormal ||
         m_OrientationTypeToBeSet == PlaneOrientationWithYAxesNormal )
      {
      validOrientation = false;
      }
    }

  if( m_ReslicingMode == Oblique )
    {
    if( m_OrientationTypeToBeSet  == OffAxial ||
         m_OrientationTypeToBeSet == OffCoronal ||
         m_OrientationTypeToBeSet == OffSagittal  ||
         m_OrientationTypeToBeSet == Axial ||
         m_OrientationTypeToBeSet == Sagittal ||
         m_OrientationTypeToBeSet == Coronal )
      {
      validOrientation = false;
      }
    }

  if( m_ReslicingMode == OffOrthogonal )
    {
    if( m_OrientationTypeToBeSet  == PlaneOrientationWithZAxesNormal ||
         m_OrientationTypeToBeSet == PlaneOrientationWithXAxesNormal ||
         m_OrientationTypeToBeSet == PlaneOrientationWithYAxesNormal  ||
         m_OrientationTypeToBeSet == Axial ||
         m_OrientationTypeToBeSet == Sagittal ||
         m_OrientationTypeToBeSet == Coronal )
      {
      validOrientation = false;
      }
    }
       
  if ( validOrientation )
    {
    m_StateMachine.PushInput( m_ValidOrientationTypeInput );
    }
  else
    {
    m_StateMachine.PushInput( m_InValidOrientationTypeInput );
    }

  m_StateMachine.ProcessInputs();
}

void 
ReslicerPlaneSpatialObject
::SetOrientationTypeProcessing()
{
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::SetOrientationTypeProcessing called...\n");
  m_OrientationType = m_OrientationTypeToBeSet;
}

void
ReslicerPlaneSpatialObject
::RequestSetBoundingBoxProviderSpatialObject( const BoundingBoxProviderSpatialObjectType* spatialObject )
{
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::RequestSetBoundingBoxProviderSpatialObject called...\n");

  m_BoundingBoxProviderSpatialObjectToBeSet = const_cast< BoundingBoxProviderSpatialObjectType* >(spatialObject);

  m_StateMachine.PushInput( m_SetBoundingBoxProviderSpatialObjectInput );

  m_StateMachine.ProcessInputs();
}

void
ReslicerPlaneSpatialObject
::AttemptSetBoundingBoxProviderSpatialObjectProcessing( )
{
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::AttemptSetBoundingBoxProviderSpatialObject called...\n");

  if( !m_BoundingBoxProviderSpatialObjectToBeSet )
    {
    m_StateMachine.PushInput( m_InValidBoundingBoxProviderSpatialObjectInput );
    }
  else
    {
    m_StateMachine.PushInput( m_ValidBoundingBoxProviderSpatialObjectInput );
    }

  m_StateMachine.ProcessInputs();
}

void 
ReslicerPlaneSpatialObject
::SetBoundingBoxProviderSpatialObjectProcessing( )
{  
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::SetBoundingBoxProviderSpatialObjectProcessing called...\n");

  m_BoundingBoxProviderSpatialObject = m_BoundingBoxProviderSpatialObjectToBeSet;

  // get the bounding box from the reference spatial object
  BoundingBoxObserver::Pointer  boundingBoxObserver = BoundingBoxObserver::New();
  boundingBoxObserver->Reset();

  unsigned long boundingBoxObserverID = 
  m_BoundingBoxProviderSpatialObject->AddObserver( BoundingBoxProviderSpatialObjectType::BoundingBoxEvent(), boundingBoxObserver );
  m_BoundingBoxProviderSpatialObject->RequestGetBounds();

  if( !boundingBoxObserver->GotBoundingBox() ) 
    return;

  m_BoundingBox = boundingBoxObserver->GetBoundingBox();

  if ( m_BoundingBox.IsNull() )
    return;

  m_BoundingBoxProviderSpatialObject->RemoveObserver( boundingBoxObserverID );

  const BoundingBoxType::BoundsArrayType &bounds = m_BoundingBox->GetBounds();

  m_ImageBounds[0] = bounds[0];
  m_ImageBounds[1] = bounds[1];
  m_ImageBounds[2] = bounds[2];
  m_ImageBounds[3] = bounds[3];
  m_ImageBounds[4] = bounds[4];
  m_ImageBounds[5] = bounds[5];

  for ( unsigned int i = 0; i <= 4; i += 2 ) // reverse bounds if necessary
  {
    if ( m_ImageBounds[i] > m_ImageBounds[i+1] )
    {
      double t = m_ImageBounds[i+1];
      m_ImageBounds[i+1] = m_ImageBounds[i];
      m_ImageBounds[i] = t;
    }
  }

  // we start in the middle of the bounding box
  m_ToolPosition[0] = 0.5*(m_ImageBounds[0] + m_ImageBounds[1]);
  m_ToolPosition[1] = 0.5*(m_ImageBounds[2] + m_ImageBounds[3]);
  m_ToolPosition[2] = 0.5*(m_ImageBounds[4] + m_ImageBounds[5]);
  
  m_PlaneCenter[0] = m_ToolPosition[0];
  m_PlaneCenter[1] = m_ToolPosition[1];
  m_PlaneCenter[2] = m_ToolPosition[2];

}

void 
ReslicerPlaneSpatialObject
::ReportInvalidBoundingBoxProviderSpatialObjectProcessing( )
{
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::ReportInvalidBoundingBoxProviderSpatialObjectProcessing called...\n");
}

void 
ReslicerPlaneSpatialObject
::ReportInvalidCursorPositionProcessing( )
{  
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::ReportInvalidCursorPositionProcessing called...\n");
}

void 
ReslicerPlaneSpatialObject
::RequestSetToolSpatialObject( const ToolSpatialObjectType * toolSpatialObject )
{  
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::RequestSetToolSpatialObject called...\n");

  m_ToolSpatialObjectToBeSet = const_cast< ToolSpatialObjectType *>(toolSpatialObject);

  if( !m_ToolSpatialObjectToBeSet )
    {
    m_StateMachine.PushInput( m_InValidToolSpatialObjectInput );
    }
  else
    {
    m_StateMachine.PushInput( m_ValidToolSpatialObjectInput );
    }

  m_StateMachine.ProcessInputs();
}

void 
ReslicerPlaneSpatialObject
::SetToolSpatialObjectProcessing( )
{  
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::SetToolSpatialObjectProcessing called...\n");

  m_ToolSpatialObject = m_ToolSpatialObjectToBeSet;
  this->ObserveToolTransformWRTImageCoordinateSystemInput( this->m_ToolSpatialObject );
  m_ToolSpatialObjectSet = true;
}

void 
ReslicerPlaneSpatialObject
::ReportInvalidToolSpatialObjectProcessing( )
{  
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::ReportInvalidToolSpatialObjectProcessing called...\n");
}

void
ReslicerPlaneSpatialObject
::RequestUpdateToolTransformWRTImageCoordinateSystem()
{
  igstkLogMacro( DEBUG,
                 "igstk::ReslicerPlaneSpatialObject::\
                 RequestUpdateToolTransformWRTImageCoordinateSystem called ...\n");
  
  igstkPushInputMacro( GetToolTransformWRTImageCoordinateSystem );
  this->m_StateMachine.ProcessInputs();
}

/** Request to receive the tool transform WRT reference spatial object coordinate system */
void
ReslicerPlaneSpatialObject
::RequestGetToolTransformWRTImageCoordinateSystemProcessing()
{
  igstkLogMacro( DEBUG,
                 "igstk::ReslicerPlaneSpatialObject::\
                 RequestGetToolTransformWRTImageCoordinateSystemProcessing called ...\n");

  typedef igstk::Friends::CoordinateSystemHelper 
                          CoordinateSystemHelperType;

  const CoordinateSystem* ImageSpatialObjectCoordinateSystem = 
    CoordinateSystemHelperType::GetCoordinateSystem( m_BoundingBoxProviderSpatialObject );

  CoordinateSystem * ImageSpatialObjectCoordinateSystemNC =
          const_cast< CoordinateSystem *>(ImageSpatialObjectCoordinateSystem);

  m_ToolSpatialObject->RequestComputeTransformTo( ImageSpatialObjectCoordinateSystemNC ); 
}

/** Receive the tool spatial object transform WRT reference sapatial object
 * coordinate system using a transduction macro */
void
ReslicerPlaneSpatialObject
::ReceiveToolTransformWRTImageCoordinateSystemProcessing()
{
  igstkLogMacro( DEBUG, "ReceiveToolTransformWRTImageCoordinateSystemProcessing " 
                 << this->m_ToolTransformWRTImageCoordinateSystem );

  this->m_ToolTransformWRTImageCoordinateSystem =
    this->m_ToolTransformWRTImageCoordinateSystemInputToBeSet.GetTransform();
}

/** Request compute reslicing plane */
void
ReslicerPlaneSpatialObject
::RequestComputeReslicingPlane( )
{
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::RequestComputeReslicingPlane called...\n");

  igstkPushInputMacro( ComputeReslicePlane );
  m_StateMachine.ProcessInputs();

}

/** Compute reslicing plane */
void
ReslicerPlaneSpatialObject
::ComputeReslicePlaneProcessing()
{
  //Update the tool transform if tool spatial object provided
  if ( m_ToolSpatialObject ) 
    {
    this->RequestUpdateToolTransformWRTImageCoordinateSystem();
    }

  switch( m_ReslicingMode )
    {
    case Orthogonal:
      {
      this->ComputeOrthogonalReslicingPlane();
      break;
      }
    case Oblique:
      {
      this->ComputeObliqueReslicingPlane();
      break;
      }
    case OffOrthogonal:
      {
      this->ComputeOffOrthogonalReslicingPlane();
      break;
      }
    default:
      break;
    }  
} 

/**Compute orthgonal reslicing plane */
void
ReslicerPlaneSpatialObject
::ComputeOrthogonalReslicingPlane( )
{
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::ComputeOrthogonalReslicingPlane called...\n");

  //If a tool spatial object is set (automatic reslicing) , then the 
  //the plane center will be set to the tool postion in 3D space.
  if( m_ToolSpatialObject )
    {
        VectorType   translation;
        translation = m_ToolTransformWRTImageCoordinateSystem.GetTranslation();

        m_ToolPosition[0] = translation[0];
        m_ToolPosition[1] = translation[1];
        m_ToolPosition[2] = translation[2];

        m_PlaneCenter[0] = translation[0];
        m_PlaneCenter[1] = translation[1];
        m_PlaneCenter[2] = translation[2];

        switch( m_OrientationType )
        {
            case Axial:
            {
              m_PlaneNormal[0] = 0.0;
              m_PlaneNormal[1] = 0.0;
              m_PlaneNormal[2] = 1.0;

              m_PlaneCenter[0] = 0.5*(m_ImageBounds[0]+m_ImageBounds[1]);
              m_PlaneCenter[1] = 0.5*(m_ImageBounds[2]+m_ImageBounds[3]);

              break;
            }

            case Sagittal:
            {
              m_PlaneNormal[0] = 1.0;
              m_PlaneNormal[1] = 0.0;
              m_PlaneNormal[2] = 0.0;

              m_PlaneCenter[1] = 0.5*(m_ImageBounds[2]+m_ImageBounds[3]);
              m_PlaneCenter[2] = 0.5*(m_ImageBounds[4]+m_ImageBounds[5]);

              break;
            }

            case Coronal:
            {
              m_PlaneNormal[0] = 0.0;
              m_PlaneNormal[1] = 1.0;
              m_PlaneNormal[2] = 0.0;

              m_PlaneCenter[0] = 0.5*(m_ImageBounds[0]+m_ImageBounds[1]);
              m_PlaneCenter[2] = 0.5*(m_ImageBounds[4]+m_ImageBounds[5]);

              break;
            }

            default:
            {
            std::cerr << "Invalid orientation type " << std::endl;
            break;
            }
       }
    }
  else
    {
        // Otherwise, use the cursor postion and image bounds to set the center
        switch( m_OrientationType )
        {
            case Axial:
            {
              m_PlaneNormal[0] = 0.0;
              m_PlaneNormal[1] = 0.0;
              m_PlaneNormal[2] = 1.0;

              m_PlaneCenter[0] = 0.5*(m_ImageBounds[0] + m_ImageBounds[1]);
              m_PlaneCenter[1] = 0.5*(m_ImageBounds[2] + m_ImageBounds[3]);
              
              if ( m_CursorPositionSetFlag )
              {
              m_PlaneCenter[2] = m_CursorPosition[2];
              m_ToolPosition[2] = m_CursorPosition[2];
              m_CursorPositionSetFlag = false;
              }
              break; 
            }
            case Sagittal:
            {
              m_PlaneNormal[0] = 1.0;
              m_PlaneNormal[1] = 0.0;
              m_PlaneNormal[2] = 0.0;

              m_PlaneCenter[1] = 0.5*(m_ImageBounds[2] + m_ImageBounds[3]);
              m_PlaneCenter[2] = 0.5*(m_ImageBounds[4] + m_ImageBounds[5]);              

              if ( m_CursorPositionSetFlag )
              {
                m_PlaneCenter[0] = m_CursorPosition[0];
                m_ToolPosition[0] = m_CursorPosition[0];
                m_CursorPositionSetFlag = false;
              }
              break;
            }
            case Coronal:
            {
              m_PlaneNormal[0] = 0.0;
              m_PlaneNormal[1] = 1.0;
              m_PlaneNormal[2] = 0.0;

              m_PlaneCenter[0] = 0.5*(m_ImageBounds[0] + m_ImageBounds[1]);
              m_PlaneCenter[2] = 0.5*(m_ImageBounds[4] + m_ImageBounds[5]);

              if ( m_CursorPositionSetFlag )
              {
                  m_PlaneCenter[1] = m_CursorPosition[1];
                  m_ToolPosition[1] = m_CursorPosition[1];
                  m_CursorPositionSetFlag = false;
              }
              break;
            }
            default:
            {
                std::cerr << "Invalid orientaiton" << std::endl;
                break;
            }
      }
  }  
}

/**Compute oblique reslicing plane */
void
ReslicerPlaneSpatialObject
::ComputeObliqueReslicingPlane( )
{
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::ComputeObliqueReslicingPlane called...\n");

  /* Calculate the tool's long axis vector.
  * we make the assumption that the tool's long axis is on the -x axis
  * with the tip in (0,0,0)
  */
  const VersorType& rotation = m_ToolTransformWRTImageCoordinateSystem.GetRotation();
    // auxiliary vecs
  VectorType v1, v2, vn;
  vn.Fill(0.0);
  vn[0] = 1;
  vn = rotation.Transform(vn);

  const VectorType& translation = m_ToolTransformWRTImageCoordinateSystem.GetTranslation();

  m_ToolPosition[0] = translation[0];
  m_ToolPosition[1] = translation[1];
  m_ToolPosition[2] = translation[2];

  m_PlaneCenter[0] = translation[0];
  m_PlaneCenter[1] = translation[1];
  m_PlaneCenter[2] = translation[2];

  // get any normal vector to the tool's long axis
  // fixme: we could make use of the 6DOF by converting the
  // orthonormal base (1,0,0) (0,1,0) (0,0,1) but it's too noisy ...
  v1[0] = -vn[1];
  v1[1] = vn[0];
  v1[2] = 0;

  // get a second normal vector
  v2 = itk::CrossProduct( v1, vn );

  switch( m_OrientationType )
    {
      case PlaneOrientationWithXAxesNormal:
        { 
          m_PlaneNormal = v1;          
          break;
        }

      case PlaneOrientationWithYAxesNormal:
        {
          m_PlaneNormal = v2;              
          break;
        }

      case PlaneOrientationWithZAxesNormal:
        {
          m_PlaneNormal = vn;
          break;
        }

      default:
        {
           std::cerr << "Invalid orientaiton" << std::endl;
           break;
        }
    }
}

/**Compute off-orthgonal reslicing plane */
void
ReslicerPlaneSpatialObject
::ComputeOffOrthogonalReslicingPlane( )
{
  igstkLogMacro( DEBUG,"igstk::ReslicerPlaneSpatialObject\
                       ::ComputeOffOrthogonalReslicingPlane called...\n");

 /* Calculate the tool's long axis vector.
  * we make the assumption that the tool's long axis is on the -x axis
  * with the tip in (0,0,0)
  */
  const VersorType& rotation = m_ToolTransformWRTImageCoordinateSystem.GetRotation();
  
  // auxiliary vecs
  VectorType v1, vn;
  vn.Fill(0.0);
  vn[0] = 1;
  vn = rotation.Transform(vn);

  const VectorType& translation = m_ToolTransformWRTImageCoordinateSystem.GetTranslation();

  m_ToolPosition[0] = translation[0];
  m_ToolPosition[1] = translation[1];
  m_ToolPosition[2] = translation[2];

  m_PlaneCenter[0] = translation[0];
  m_PlaneCenter[1] = translation[1];
  m_PlaneCenter[2] = translation[2];

  switch( m_OrientationType )
    {
      case OffAxial:
      {
        v1.Fill( 0.0 );
        v1[0] = 1;
      
        if ( fabs(v1*vn) < 1e-9 )
        {
        // FIXME: need to handle this situation too
        igstkLogMacro( DEBUG, "The two vectors are parallel \n");
        }

        vn = itk::CrossProduct( v1, vn );
        vn.Normalize();

        m_PlaneCenter[0] = 0.5*(m_ImageBounds[0]+m_ImageBounds[1]);

        m_PlaneNormal = vn;
        break;
      }

      case OffSagittal:
      {
                
        v1.Fill( 0.0 );
        v1[2] = 1;
      
        if ( fabs(v1*vn) < 1e-9 )
        {
        // FIXME: need to handle this situation too
        igstkLogMacro( DEBUG, "The two vectors are parallel \n");
        }

        vn = itk::CrossProduct( v1, vn );
        vn.Normalize();

        m_PlaneNormal = vn;

        m_PlaneCenter[2] = 0.5*(m_ImageBounds[4]+m_ImageBounds[5]);
        
        break;
      }

    case OffCoronal:
      {
        v1.Fill( 0.0 );
        v1[1] = 1;
      
        if ( fabs(v1*vn) < 1e-9 )
        {
        // FIXME: need to handle this situation too
        igstkLogMacro( DEBUG, "The two vectors are parallel \n");
        }

        vn = itk::CrossProduct( v1, vn );
        vn.Normalize();

        m_PlaneNormal = vn;

        m_PlaneCenter[1] = 0.5*(m_ImageBounds[2]+m_ImageBounds[3]);        

        break;
      }      

      default:
        {
           std::cerr << "Invalid orientaiton" << std::endl;
           break;
        }
    }
}

/** Report invalid reslicing mode */
void
ReslicerPlaneSpatialObject
::ReportInvalidReslicingModeProcessing( void )
{
  igstkLogMacro( WARNING, 
    "igstk::ReslicerPlaneSpatialObject::ReportInvalidReslicingModeProcessing called...\n");
}

/** Report invalid orientation type */
void
ReslicerPlaneSpatialObject
::ReportInvalidOrientationTypeProcessing( void )
{
  igstkLogMacro( WARNING, 
    "igstk::ReslicerPlaneSpatialObject::ReportInvalidOrientationTypeProcessing called...\n");
}

/** Report invalid request */
void 
ReslicerPlaneSpatialObject
::ReportInvalidRequestProcessing( void )
{
  igstkLogMacro( DEBUG, 
    "igstk::ReslicerPlaneSpatialObject::ReportInvalidRequestProcessing called...\n");

  this->InvokeEvent( InvalidRequestErrorEvent() );
}

/** Get tool transform WRT Image Coordinate System*/
igstk::Transform
ReslicerPlaneSpatialObject
::GetToolTransform( ) const
{
  return this->m_ToolTransformWRTImageCoordinateSystem;
}

//todo: get tool position as an event
ReslicerPlaneSpatialObject::VectorType
ReslicerPlaneSpatialObject
::GetToolPosition() const
{
  return m_ToolPosition;
}

/** Check if tool spatial object is set to drive the reslicing*/
bool
ReslicerPlaneSpatialObject
::IsToolSpatialObjectSet( ) 
{
  return this->m_ToolSpatialObjectSet;
}

/** Print object information */
void
ReslicerPlaneSpatialObject
::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);
}

} // end namespace igstk


#endif
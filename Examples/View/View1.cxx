/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    View1.cxx
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
//Warning about: identifier was truncated to '255' characters in the debug 
//information (MVC6.0 Debug)
#pragma warning( disable : 4786 )
#endif

// BeginLatex
// 
// This example illustrates how to use the \doxygen{View3D} class to display
// spatial objects.
//
// EndLatex

#include <iostream>
#include "igstkView3D.h"
#include "igstkEvents.h"
#include "igstkEllipsoidObject.h"
#include "igstkEllipsoidObjectRepresentation.h"
#include "igstkVTKLoggerOutput.h"

#include "igstkLogger.h"
#include "itkStdStreamLogOutput.h"

int main( int, char * [] )
{
  igstk::RealTimeClock::Initialize();

  // BeginLatex
  // 
  // First, a 3D view and other useful data types are defined: 
  //
  // EndLatex

  // BeginCodeSnippet
  typedef igstk::View3D            View3DType;
  typedef igstk::Object::LoggerType             LoggerType;
  typedef itk::StdStreamLogOutput  LogOutputType;
  // EndCodeSnippet

  // logger object created for logging mouse activities
  LoggerType::Pointer   logger = LoggerType::New();
  LogOutputType::Pointer logOutput = LogOutputType::New();
  logOutput->SetStream( std::cout );
  logger->AddLogOutput( logOutput );
  logger->SetPriorityLevel( itk::Logger::DEBUG );

  // BeginLatex
  //
  // For debugging purposes, the VTK window output can be redirected to a
  // logger, using \doxygen{VTKLoggerOutput}, as follows:
  //
  // EndLatex

  // BeginCodeSnippet
  igstk::VTKLoggerOutput::Pointer vtkLoggerOutput = 
                                  igstk::VTKLoggerOutput::New();
  vtkLoggerOutput->OverrideVTKWindow();
  vtkLoggerOutput->SetLogger(logger);  
  // EndCodeSnippet

  try
    {
    // BeginLatex
    // 
    // In this example, we would like to display an ellipsoid object.  To carry
    // this out, an ellipsoid spatial object is first instantiated:
    // 
    // EndLatex

    // BeginCodeSnippet
    igstk::EllipsoidObject::Pointer ellipsoid = igstk::EllipsoidObject::New();
    ellipsoid->SetRadius(0.1,0.1,0.1);
    // EndCodeSnippet
  
    // BeginLatex
    // 
    // Next, a representation object is created using
    // the \doxygen{EllipsoidObjectRepresentation} class. The  representation
    // class provides the mechanism to generate a graphical description of the
    // spatial object for visualization in a VTK scene, as follows: 
    //
    //EndLatex 

    // BeginCodeSnippet 
    igstk::EllipsoidObjectRepresentation::Pointer ellipsoidRepresentation = 
                                  igstk::EllipsoidObjectRepresentation::New();
    ellipsoidRepresentation->RequestSetEllipsoidObject( ellipsoid );
    ellipsoidRepresentation->SetColor(0.0,1.0,0.0);
    ellipsoidRepresentation->SetOpacity(1.0);
    // EndCodeSnippet

    // BeginLatex
    // 
    // A geometrical transformation can then be applied to the ellipsoid spatial
    // object as follows:
    // 
    // EndLatex

    // BeginCodeSnippet
    const double validityTimeInMilliseconds = 1e300; // 100 seconds
    igstk::Transform transform;
    igstk::Transform::VectorType translation;
    translation[0] = 0;
    translation[1] = 10;
    translation[2] = 10;
    igstk::Transform::VersorType rotation;
    rotation.Set( 0.0, 0.0, 0.0, 1.0 );
    igstk::Transform::ErrorType errorValue = 10; // 10 millimeters

    transform.SetTranslationAndRotation( 
        translation, rotation, errorValue, validityTimeInMilliseconds );
#ifdef USE_SPATIAL_OBJECT_DEPRECATED  
    ellipsoid->RequestSetTransform( transform );
#endif
    // EndCodeSnippet

    // BeginLatex
    // 
    // Next, the FLTK window and a view object are instantiated:
    //
    // EndLatex

    // BeginCodeSnippet
    Fl_Window * form = new Fl_Window(601,301,"View Test");
    View3DType * view3D = new View3DType(310,10,280,280,"3D View");
    form->end();
    form->show();
    // EndCodeSnippet
    
    view3D->RequestResetCamera();
    view3D->RequestEnableInteractions();
   
    // BeginLatex
    // 
    // The ellipsoid is added to the scene using the \code{RequestAddObject}
    // method as follows:
    //
    // EndLatex

    // BeginCodeSnippet
    view3D->RequestAddObject( ellipsoidRepresentation );
    // EndCodeSnippet

    // BeginLatex
    //
    // The View components are designed for refreshing their representation at
    // regular intervals. The application developer must set the desired
    // refresh rate in Hertz, which should trigger the 
    // internal generation of pulses that makes it possible for the View class
    // to refresh itself as follows:
    // 
    // EndLatex

    // BeginCodeSnippet
    view3D->RequestSetRefreshRate( 30 );
    view3D->RequestStart();
    // EndCodeSnippet


    // BeginLatex
    // 
    // At this point it is now possible to start the event loop that will drive
    // the user interaction of the application. Inside the loop, it is of
    // fundamental importance to invoke the call to the
    // \doxygen{PulseGenerator} method \code{CheckTimeouts()}. This methods
    // ensures that the pulse generator timers in all the autonomous IGSTK
    // classes are checked to see if they should trigger timer events. The same
    // loop should have some form of ``wait'' or ``sleep'' instruction in order
    // to prevent the loop from taking over the CPU time.
    //
    // EndLatex

    // BeginCodeSnippet
    for(unsigned int i=0; i<10; i++)
      {
      Fl::wait( 0.01 );
      igstk::PulseGenerator::CheckTimeouts();
      Fl::check();       // trigger FLTK redraws
      }
    // EndCodeSnippet


    // BeginLatex
    // 
    // Once the event loop finishes, the method \code{RequestStop()} should be 
    // called to stop the refresh process of the View class as follows:
    //
    // EndLatex


    // BeginCodeSnippet
    view3D->RequestStop();
    // EndCodeSnippet

    delete view3D;
    delete form;
    }
  catch(...)
    {
    std::cerr << "Exception catched !!" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

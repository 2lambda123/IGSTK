/*=========================================================================

Program:   Image Guided Surgery Software Toolkit
Module:    igstkQTWidget.h
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __igstkQTWidget_h
#define __igstkQTWidget_h

#ifdef _MSC_VER
#pragma warning ( disable : 4018 )
//Warning about: identifier was truncated to '255' characters in the debug
//information (MVC6.0 Debug)
#pragma warning( disable : 4284 )
#endif

// QT dared to define macro called DEBUG!!
#define QT_NO_DEBUG 1

// VTK 
#include "vtkInteractorStyle.h"
#include "vtkRenderer.h"

// ITK headers
#include "itkCommand.h"
#include "itkLogger.h"

// IGSTK headers
#include "igstkMacros.h"
#include "igstkStateMachine.h"
#include "igstkViewNew.h"
#include "igstkViewProxy.h"

#include "QVTKWidget.h"

namespace igstk {

/** \class QTWidget
 * 
 * \brief Widget class to develop QT based GUI application 
 */
class QTWidget : public QVTKWidget
{

public:
    
  typedef QTWidget          Self;
  typedef QVTKWidget        Superclass;
  typedef ViewNew           ViewType; 

  igstkTypeMacro( QTWidget, QVTKWidget );
  
  /** Declarations needed for the State Machine */
  igstkStateMachineMacro();

  /** Print the object information in a stream. */
  void Print( std::ostream& os, ::itk::Indent indent=0) const;

  /** Set up variables, types and methods related to the Logger */
  igstkLoggerMacro()

  /** Constructor */
#if QT_VERSION < 0x040000
    //! constructor for Qt 3
    QTWidget(QWidget* parent = NULL, const char* name = NULL, Qt::WFlags f = 0);
#else
    //! constructor for Qt 4
    QTWidget(QWidget* parent = NULL, Qt::WFlags f = 0);
#endif

  /** Destructor */
  virtual ~QTWidget( void );
  
  /** Update the display in order to render the new content of the scene */
  void Update();
 
  /** set the view */
  void SetView( ViewType::Pointer view );

  /** Initialize method */
  void Initialize();

  /** Enable method */
  void Enable();

  /** Render method */
  void Render();

  typedef ViewProxy< QTWidget > ProxyType;

  friend class ViewProxy< QTWidget >;

protected:
  
  /** Print the object information in a stream. */
  virtual void PrintSelf( std::ostream& os, itk::Indent indent ) const; 
 
 /** Overload the mouse move event to send transform events
    when the mouse is moved while the left button is down */
  void mouseMoveEvent(QMouseEvent* e);

private:

  /** Set VTK renderer */
  void SetVTKRenderer( vtkRenderer * renderer )
    {
    this->m_VTKRenderer = renderer;
    }

  /** Set VTK render window interactor */
  void SetVTKRenderWindowInteractor( vtkRenderWindowInteractor * interactor )
    {
    this->m_VTKRenderWindowInteractor = interactor;
    }

private:
  bool                    m_InteractionHandling;
  ViewType::Pointer       m_View;

  ProxyType               m_ProxyView;

  vtkRenderer           * m_VTKRenderer;

  vtkRenderWindowInteractor           * m_VTKRenderWindowInteractor;

};

std::ostream& operator<<(std::ostream& os, const QTWidget& o);

} // end namespace igstk

#endif

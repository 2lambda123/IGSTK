/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    VideoFrameGrabberAndViewerWebcamWinView.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// generated by Fast Light User Interface Designer (fluid) version 1.0107

#ifndef __VideoFrameGrabberAndViewerWebcamWinView_h
#define __VideoFrameGrabberAndViewerWebcamWinView_h

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Tabs.H>

#include "igstkFLTKWidget.h"
#include "igstkView3D.h"
#include "igstkEvents.h"
#include "igstkCTImageSpatialObject.h"

namespace igstk {

/** \class VideoView
*
* \brief Composite FLTK class defines the standard view window.
*
*/

class VideoFrameGrabberAndViewerWebcamWinView : public Fl_Group {
public:

  VideoFrameGrabberAndViewerWebcamWinView(int X, int Y, int W, int H, const char *L = 0);

  virtual ~VideoFrameGrabberAndViewerWebcamWinView(void);

  typedef igstk::View          ViewType;
  typedef igstk::View3D        ViewType3D;

  // Declare View objects
  ViewType3D::Pointer          m_VideoView;

  // Declare FLTKWidgetNew objects
  igstk::FLTKWidget * m_VideoWidget;

private:

  int m_Width;
  int m_Height;
  int m_X;
  int m_Y;
  int m_WW;
  int m_HH;
};

} // end namespace igstk

#endif

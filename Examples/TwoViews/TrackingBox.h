/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    TrackingBox.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISIS Georgetown University. All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __TrackingBox_h_
#define __TrackingBox_h_

#if defined(_MSC_VER)
   //Warning about: identifier was truncated to '255' characters in the debug information (MVC6.0 Debug)
#pragma warning( disable : 4284 )
#endif

#include "igstkMouseTracker.h"
#include "igstkSpatialObject.h"
#include "FL/Fl_Box.h"
#include "igstkView.h"

class TrackingBox : public Fl_Box
{
public:

  typedef igstk::MouseTracker TrackerType;
  typedef igstk::View  ViewType;
  typedef igstk::SpatialObject ObjectType;

  TrackingBox(int x, int y, int w, int h, const char *l=0) : Fl_Box(x,y,w,h,l) 
    {
    m_Tracking = false;
    m_Tracker = TrackerType::New();
    m_Tracker->Initialize();
    };

  ~TrackingBox() 
    {
    m_Tracker->Close();
    };

  void SetTracking(bool val) 
    {
    m_Tracking = val;
    if(m_Tracking)
      {
      m_Tracker->StartTracking();
      m_Tracker->Reset();
      Loop();
      }
    }

  /** Do the looping */
  void Loop()
    {
    while(m_Tracking)
      {
      m_Tracker->UpdateStatus();
      TrackerType::PositionType position;
      m_Tracker->GetPosition( position );

      // We update only if the mouse is inside the box
      if(
        position[0] > this->x() && position[0] < this->x()+this->w()
        && position[1] > this->y() && position[1] < this->y()+this->h()
        )
        {
        // Put everything in the reference frame of the current box
        position[0] -= this->x()+this->w()/2;
        position[1] -= this->y()+this->h()/2;
        double factor = 100;
        m_Object->SetOffset(position[0]/factor,-position[1]/factor,position[2]/factor);
        m_View->Update();
        m_View2->Update();
        }
      Fl::check();
      }
    m_Tracker->StopTracking();
    }

  void SetObjectToTrack(ObjectType* object) {m_Object = object;}
  void SetView(ViewType* view) {m_View = view;}
  void SetView2(ViewType* view) {m_View2 = view;}

protected:

  bool m_Tracking;
  TrackerType::Pointer m_Tracker;
  ObjectType* m_Object;
  ViewType*   m_View;
  ViewType*   m_View2;
};
 
#endif

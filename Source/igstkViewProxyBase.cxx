/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkViewProxyBase.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "igstkViewProxyBase.h"
#include "igstkViewNew.h"

namespace igstk {

vtkRenderer * ViewProxyBase::GetRenderer( ViewNew * view )
{
  return view->GetRenderer();
} 

vtkRenderWindowInteractor * 
ViewProxyBase::GetRenderWindowInteractor( ViewNew * view )
{
  return view->GetRenderWindowInteractor();
} 

::itk::Object::Pointer  
ViewProxyBase::GetReporter( ViewNew * view )
{
  return view->GetReporter();
} 

void
ViewProxyBase::InitializeInteractor( ViewNew * view )
{
  view->RequestInitializeRenderWindowInteractor();
}

void 
ViewProxyBase::SetRenderWindowSize( ViewNew * view, int width, int height )
{
  view->RequestSetRenderWindowSize( width, height );
}

} // end namespace igstk
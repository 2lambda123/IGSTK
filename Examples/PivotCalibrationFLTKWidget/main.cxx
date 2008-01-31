/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    main.cxx
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
#include "igstkRealTimeClock.h"

      
int main(int argc, char *argv[])
{
  PivotCalibrationFLTKWidgetExample app;

  //required by IGSTK for the time stamping used by the tracker
  igstk::RealTimeClock::Initialize();

  app.Show();  
  return Fl::run();
}

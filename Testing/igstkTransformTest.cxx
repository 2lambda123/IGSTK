/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkTransformTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISIS Georgetown University. All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if defined(_MSC_VER)
   //Warning about: identifier was truncated to '255' characters in the debug information (MVC6.0 Debug)
#pragma warning( disable : 4786 )
#endif

#include <iostream>
#include "igstkTransform.h"

int igstkTransformTest( int, char * [] )
{

  try
    {
    typedef igstk::Transform    TransformType;

    TransformType t1;
    
    const double tx = 10.0;
    const double ty = 10.0;
    const double tz = 10.0;

    const double qx =  1.0;
    const double qy =  0.0;
    const double qz =  0.0;
    const double qw =  0.0;

    const double validityPeriod = 10.0; // milliseconds

    t1.SetTranslationAndRotation( tx,ty,tz, qx,qy,qz,qw, validityPeriod );
    }
  catch(...)
    {
    std::cerr << "Exception catched !!!" << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "Test PASSED ! " << std::endl;

  return EXIT_SUCCESS;
}



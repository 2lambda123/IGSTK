/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkSceneTest.cxx
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
#include "igstkScene.h"
#include "igstkEllipsoidObject.h"

int igstkSceneTest( int, char * [] )
{
  typedef igstk::Scene  SceneType;
  SceneType::Pointer scene = SceneType::New();

  typedef igstk::EllipsoidObject  ObjectType;
  ObjectType::Pointer ellipsoid = ObjectType::New();

  // Testing number of object
  std::cout << "Testing GetNumberOfObjects: ";
  if(scene->GetNumberOfObjects() != 0)
    { 
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "[PASSED]" << std::endl;

  // Add Object
  std::cout << "Testing AddObject: ";
  scene->AddObject(ellipsoid); 
  if(scene->GetNumberOfObjects() != 1)
    { 
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "[PASSED]" << std::endl;

  // Remove object
  std::cout << "Testing RemoveObject: ";
  scene->RemoveObject(ellipsoid); 
  if(scene->GetNumberOfObjects() != 0)
    { 
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "[PASSED]" << std::endl;

  /** Returns a list of pointer to the children affiliated to this object.*/ 
  //ObjectListType * GetObjects( unsigned int depth=MaximumDepth,
  //                             char * name=NULL );
  


  return EXIT_SUCCESS;
}



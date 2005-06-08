/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    igstkVTKLoggerOutput.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include"igstkVTKLoggerOutput.h"
#include"vtkObjectFactory.h"
#include"itkIndent.h"
#include <iostream>


namespace igstk
{
vtkCxxRevisionMacro(VTKLoggerOutput, "1.11");
vtkStandardNewMacro(VTKLoggerOutput);

/** Send a string to display. */
void VTKLoggerOutput::DisplayText(const char* t)
{
  if( this->Logger )
  {
    this->Logger->Write(itk::Logger::INFO, t);
  }
}


/** Send a string as an error message to display.
 * The default implementation calls DisplayText() but subclasses
 * could present this message differently. */
void VTKLoggerOutput::DisplayErrorText(const char *t)
{
  if( this->Logger )
  {
    this->Logger->Write(itk::Logger::CRITICAL, t);
  }
}


/** Send a string as a warningmessage to display.
 * The default implementation calls DisplayText() but subclasses
 * could present this message differently. */
void VTKLoggerOutput::DisplayWarningText(const char *t)
{
  if( this->Logger )
  {
    this->Logger->Write(itk::Logger::WARNING, t);
  }
}


/** Send a string as a message to display.
 * The default implementation calls DisplayText() but subclasses
 * could present this message differently. */
void VTKLoggerOutput::DisplayGenericWarningText(const char *t)
{
  if( this->Logger )
  {
    this->Logger->Write(itk::Logger::WARNING, t);
  }
}


/** Send a string as a debug message to display.
 * The default implementation calls DisplayText() but subclasses
 * could present this message differently. */
void VTKLoggerOutput::DisplayDebugText(const char *t)
{
  if( this->Logger )
  {
    this->Logger->Write(itk::Logger::DEBUG, t);
  }
}


void VTKLoggerOutput::PrintSelf(StdOStreamType& os, itk::Indent indent) const
{
  os << indent << "RTTI typeinfo:   " << typeid( *this ).name() << std::endl;
  indent = indent.GetNextIndent();

  if( this->Logger )
  {
//    os << indent << *this->Logger << std::endl;
    os << indent << "Logger is available" << std::endl;
  }
}

void VTKLoggerOutput::Print(StdOStreamType& os)
{
  itk::Indent indent;

  os << indent << this->GetClassName() << " (" << this << ")\n";
  this->PrintSelf(os, indent.GetNextIndent());
  os << indent << "\n";
}

std::ostream& operator<<(std::ostream& os, VTKLoggerOutput& o)
{
  o.Print(os);
  return os;
}
  
} // the end of the namespace igstk



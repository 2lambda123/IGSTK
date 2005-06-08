/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    igstkVTKLoggerOutput.h
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

#ifndef __igstkVTKLoggerOutput_h
#define __igstkVTKLoggerOutput_h

#include<vtkOutputWindow.h>
#include"itkLogger.h"


namespace igstk
{
/** \class VTKLoggerOutput
 * \brief This class is meant for overriding vtk::OutputWindow to redirect messages to itk::Logger.
 *
 * Text messages that the system should display to the user are sent to 
 * this object (or subclasses of this object).
 *
 * \ingroup OSSystemObjects
 */
class VTKLoggerOutput : public vtkOutputWindow 
{
public:
  /** Standard class typedefs. */
  typedef VTKLoggerOutput        Self;
  typedef vtkOutputWindow  Superclass;
  
  /** Run-time type information (and related methods). */
  vtkTypeRevisionMacro(VTKLoggerOutput, vtkOutputWindow);

  typedef itk::Logger*   LoggerType;

  static VTKLoggerOutput* New();

  /** Send a string to display. */
  virtual void DisplayText(const char* t);

  /** Send a string as an error message to display.
   * The default implementation calls DisplayText() but subclasses
   * could present this message differently. */
  virtual void DisplayErrorText(const char *t);

  /** Send a string as a warningmessage to display.
   * The default implementation calls DisplayText() but subclasses
   * could present this message differently. */
  virtual void DisplayWarningText(const char *t);

  /** Send a string as a message to display.
   * The default implementation calls DisplayText() but subclasses
   * could present this message differently. */
  virtual void DisplayGenericWarningText(const char *t);

  /** Send a string as a debug message to display.
   * The default implementation calls DisplayText() but subclasses
   * could present this message differently. */
  virtual void DisplayDebugText(const char *t);

  vtkSetMacro(Logger, LoggerType);

  vtkGetMacro(Logger, LoggerType);

  virtual void OverrideVTKWindow() 
  {
    vtkOutputWindow::SetInstance(this);
  }
 
  typedef std::ostream StdOStreamType;
  void Print(StdOStreamType& os);

protected:
  VTKLoggerOutput() {}
  virtual ~VTKLoggerOutput() {}
  void PrintSelf(StdOStreamType& os, itk::Indent indent) const;

private:
  itk::Logger* Logger;
};
  
std::ostream& operator<<(std::ostream& os, VTKLoggerOutput& o);
} // end namespace igstk

//std::ostream& operator<<(std::ostream& os, const vtkObjectBase& o);
//std::ostream& operator<<(std::ostream& os, const vtkIndent& o);


#endif  // __igstkVTKLoggerOutput_h

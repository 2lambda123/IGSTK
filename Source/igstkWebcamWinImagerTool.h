/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkWebcamWinImagerTool.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __igstkWebcamWinImagerTool_h
#define __igstkWebcamWinImagerTool_h

#include "igstkImagerTool.h"

namespace igstk
{

class WebcamWinImager;

/** \class WebcamWinImagerTool
  * \brief A WebcamWin -specific ImagerTool class.
  *
  * This class provides WebcamWin  -specific functionality
  * for ImagerTools.
  *
  * \ingroup Imager
  *
  */

class WebcamWinImagerTool : public ImagerTool
{
public:

  /** Macro with standard traits declarations. */
  igstkStandardClassTraitsMacro( WebcamWinImagerTool, ImagerTool )

  /** Get Imager tool name */
  igstkGetStringMacro( ImagerToolName );

  /** Set imager tool ImagerTool name */
  void RequestSetImagerToolName( const std::string &);

protected:

  WebcamWinImagerTool();
  virtual ~WebcamWinImagerTool();

  /** Print object information */
  virtual void PrintSelf( std::ostream& os, ::itk::Indent indent ) const;

private:
  WebcamWinImagerTool(const Self&);    //purposely not implemented
  void operator=(const Self&);          //purposely not implemented

  /** States for the State Machine */
  igstkDeclareStateMacro( Idle );
  igstkDeclareStateMacro( ImagerToolNameSpecified );

  /** Inputs to the State Machine */
  igstkDeclareInputMacro( ValidImagerToolName );
  igstkDeclareInputMacro( InValidImagerToolName );

  /** Get boolean variable to check if the imager tool is
   * configured or not */
  virtual bool CheckIfImagerToolIsConfigured() const;

  /** Report Invalid ImagerTool name specified*/
  void ReportInvalidImagerToolNameSpecifiedProcessing( );

  /** Report any invalid request to the logger */
  void ReportInvalidRequestProcessing();

  /** Set ImagerTool name */
  void SetImagerToolNameProcessing();

  std::string     m_ImagerToolName;
  std::string     m_ImagerToolNameToBeSet;

  bool            m_ImagerToolConfigured;

};

} // namespace igstk

#endif  // __igstk_WebcamWinImagerTool_h_

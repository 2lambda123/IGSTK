/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkLogger.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkLogger.h"

namespace itk
{

Logger::Logger()
{
  this->m_PriorityLevel = Logger::NOTSET;
  this->m_LevelForFlushing = Logger::MUSTFLUSH;
  this->m_Clock = RealtimeClock::New();
  this->m_Output = MultipleLogOutput::New();
}

Logger::~Logger()
{
//  this->m_Output->Flush();
}


/** Adds an output stream to the MultipleLogOutput for writing. */
void Logger::AddLogOutput( OutputType* output )
{
  // delegates to MultipleLogOutput
  this->m_Output->AddLogOutput( output ); 
}


void Logger::Write(PriorityLevelType level, std::string const & content)
{
  static std::string m_LevelString[] = { "(MUSTFLUSH) ", "(FATAL) ", "(ERROR) ",
    "(WARNING) ", "(INFO) ", "(DEBUG) ", "(NOTSET) " };

  if( this->m_PriorityLevel >= level )
  {
    this->m_Output->Write(this->GetName() + std::string("  ") + m_LevelString[level] + content, m_Clock->GetTimestamp());
    if( this->m_LevelForFlushing >= level )
    {
      this->m_Output->Flush();
    }
  }
}


void Logger::Flush()
{
  this->m_Output->Flush();
}


/** Print contents of a Logger */
void Logger::PrintSelf(std::ostream &os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  
  os << indent << "Name: " << this->GetName() << std::endl;
  os << indent << "PriorityLevel: " << this->GetPriorityLevel()   << std::endl;
  os << indent << "LevelForFlushing: " << this->GetLevelForFlushing() << std::endl;
}

}


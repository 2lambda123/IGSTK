/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkLoggerTest.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if defined(_MSC_VER)
   //Warning about: identifier was truncated to '255' characters in the debug information (MVC6.0 Debug)
#pragma warning( disable : 4786 )
#endif

#include <iostream>
#include <fstream>
#include "itkStdStreamLogOutput.h"
#include "itkLogger.h"
#include "itkLogMacro.h"



class LogTester
{
public:
  itk::Logger* GetLogger() { return m_Logger; }
  void SetLogger(itk::Logger* logger) { m_Logger = logger; }
  void log() {
    itkLogMacro(itk::Logger::DEBUG, "DEBUG message by itkLogMacro\n");
    itkLogMacro(itk::Logger::INFO, "INFO message by itkLogMacro\n");
    itkLogMacro(itk::Logger::WARNING, "WARNING message by itkLogMacro\n");
    itkLogMacro(itk::Logger::CRITICAL, "CRITICAL message by itkLogMacro\n");
    itkLogMacro(itk::Logger::FATAL, "FATAL message by itkLogMacro\n");
    itkLogMacro(itk::Logger::MUSTFLUSH, "MUSTFLUSH message by itkLogMacro\n");
  }
  static void logStatic(LogTester* tester)
  {
    itkLogMacroStatic(tester, itk::Logger::DEBUG, "DEBUG message by itkLogMacroStatic\n");
    itkLogMacroStatic(tester, itk::Logger::INFO, "INFO message by itkLogMacroStatic\n");
    itkLogMacroStatic(tester, itk::Logger::WARNING, "WARNING message by itkLogMacroStatic\n");
    itkLogMacroStatic(tester, itk::Logger::CRITICAL, "CRITICAL message by itkLogMacroStatic\n");
    itkLogMacroStatic(tester, itk::Logger::FATAL, "FATAL message by itkLogMacroStatic\n");
    itkLogMacroStatic(tester, itk::Logger::MUSTFLUSH, "MUSTFLUSH message by itkLogMacroStatic\n");
  }
private:
  itk::Logger* m_Logger;
};



int itkLoggerTest( int, char * [] )
{
  try
    {

    // Create an ITK StdStreamLogOutputs
    itk::StdStreamLogOutput::Pointer coutput = itk::StdStreamLogOutput::New();
    itk::StdStreamLogOutput::Pointer foutput = itk::StdStreamLogOutput::New();
    coutput->SetStream(std::cout);
    std::ofstream fout("test_logger.txt");
    foutput->SetStream(fout);

    // Create an ITK Logger
    itk::Logger::Pointer logger = itk::Logger::New();

    std::cout << "Testing itk::Logger" << std::endl;

    // Setting the logger
    logger->SetName("org.itk.rootLogger");
    logger->SetPriorityLevel(itk::Logger::INFO);
    logger->SetLevelForFlushing(itk::Logger::CRITICAL);

    std::cout << "  Adding console and file stream LogOutputs" << std::endl;
    logger->AddLogOutput(coutput);
    logger->AddLogOutput(foutput);

    // Printing the logger's member variables
    std::cout << "  Name: " << logger->GetName() << std::endl;
    std::cout << "  Priority Level: " << logger->GetPriorityLevel() << std::endl;
    std::cout << "  Level For Flushing: " << logger->GetLevelForFlushing() << std::endl;

    // Logging by the itkLogMacro from a class with itk::Logger
    std::cout << "  Logging by the itkLogMacro from a class with itk::Logger" << std::endl;
    LogTester tester;
    tester.SetLogger(logger);
    tester.log();
    // Logging by the itkLogMacroStatic from a class with itk::Logger
    std::cout << "  Logging by the itkLogMacroStatic from a class with itk::Logger" << std::endl;
    LogTester::logStatic(&tester);

    // Writing by the logger
    std::cout << "  Writing by itk::Logger" << std::endl;
    logger->Write(itk::Logger::DEBUG, "This is the DEBUG message.\n");
    logger->Write(itk::Logger::INFO, "This is the INFO message.\n");
    logger->Write(itk::Logger::WARNING, "This is the WARNING message.\n");
    logger->Write(itk::Logger::CRITICAL, "This is the CRITICAL message.\n");
    logger->Write(itk::Logger::FATAL, "This is the FATAL message.\n");
    logger->Write(itk::Logger::MUSTFLUSH, "This is the MUSTFLUSH message.\n");
    logger->Flush();
    }
  catch(...)
    {
    std::cerr << "Exception catched !!" << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "[PASSED]" << std::endl;
  return EXIT_SUCCESS;
}



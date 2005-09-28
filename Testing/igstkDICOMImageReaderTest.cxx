/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkDICOMImageReaderTest.cxx
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
#pragma warning ( disable : 4786 )
#endif

#include "igstkDICOMImageReader.h"
#include "igstkImageSpatialObject.h"

#include "itkLogger.h"
#include "itkCommand.h"
#include "itkStdStreamLogOutput.h"

class DICOMImageModalityInfoCallback: public itk::Command
{
public:
  typedef DICOMImageModalityInfoCallback    Self;
  typedef itk::SmartPointer<Self>           Pointer;
  typedef itk::Command                      Superclass;
  itkNewMacro(Self);

  typedef igstk::ImageSpatialObject< 
                                short, 
                                3                                
                                       > ImageSpatialObjectType;
  
 
  typedef igstk::DICOMModalityEvent DICOMModalityEventType;
  
  void Execute(const itk::Object *caller, const itk::EventObject & event)
  {

  }
  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
    if( DICOMModalityEventType().CheckEvent( &event ) )
      {
      const DICOMModalityEventType * modalityEvent = dynamic_cast< const DICOMModalityEventType *>( &event );
      std::cout << "Modality= " << modalityEvent->GetString() << std::endl;
      }
      
  }
protected:
  DICOMImageModalityInfoCallback()   { };

private:
};

class DICOMImagePatientNameInfoCallback: public itk::Command
{
public:
  typedef DICOMImagePatientNameInfoCallback    Self;
  typedef itk::SmartPointer<Self>           Pointer;
  typedef itk::Command                      Superclass;
  itkNewMacro(Self);

  typedef igstk::ImageSpatialObject< 
                                short, 
                                3                                
                                       > ImageSpatialObjectType;
  
 
  typedef igstk::DICOMPatientNameEvent DICOMPatientNameEventType;
  
  void Execute(const itk::Object *caller, const itk::EventObject & event)
  {

  }
  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
    const DICOMPatientNameEventType * patientNameEvent = 
          dynamic_cast < const DICOMPatientNameEventType* > ( &event );    
    std::cerr << "PatientName= " << patientNameEvent->GetString() << std::endl;
      
  }
protected:
  DICOMImagePatientNameInfoCallback()   { };

private:
};



int igstkDICOMImageReaderTest( int argc, char* argv[] )
{

  if(argc < 2)
    {
    std::cerr<<"Usage: "<<argv[0]<<"  DICOMSeries "<<std::endl;
    return EXIT_FAILURE;
    }
  
  typedef itk::Logger              LoggerType;
  typedef itk::StdStreamLogOutput  LogOutputType;
  
  // logger object created for logging mouse activities
  LoggerType::Pointer   logger = LoggerType::New();
  LogOutputType::Pointer logOutput = LogOutputType::New();
  logOutput->SetStream( std::cout );
  logger->AddLogOutput( logOutput );
  logger->SetPriorityLevel( itk::Logger::DEBUG );

  typedef short      PixelType;
  const unsigned int Dimension = 3;

  typedef igstk::ImageSpatialObject< 
                                PixelType, 
                                Dimension 
                                       > ImageSpatialObjectType;
  

  typedef igstk::DICOMImageReader< ImageSpatialObjectType >    ReaderType;

  ReaderType::Pointer   reader = ReaderType::New();

  reader->SetLogger( logger );

  /* Read in a DICOM series */
  std::cout << "Reading the DICOM series : " << argv[1] <<std::endl;

  ReaderType::DirectoryNameType directoryName = argv[1];

  reader->RequestSetDirectory( directoryName );
  reader->RequestReadImage();
  
 /* Add observer to listen to modality info */
  
  DICOMImageModalityInfoCallback::Pointer dimcb = DICOMImageModalityInfoCallback::New();
    
  reader->AddObserver( igstk::DICOMModalityEvent(), dimcb );

  reader->RequestModalityInfo(); 
 
  /* Add observer to listen to patient name  info */
  
  DICOMImagePatientNameInfoCallback::Pointer dipncb = DICOMImagePatientNameInfoCallback::New();

  reader->AddObserver( igstk::DICOMPatientNameEvent(), dipncb );

  reader->RequestPatientNameInfo(); 
 
  return EXIT_SUCCESS;
}


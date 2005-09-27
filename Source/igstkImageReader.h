/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkImageReader.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISIS Georgetown University. All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __igstkImageReader_h
#define __igstkImageReader_h

#include "igstkMacros.h"

#include "itkObject.h"
#include "itkMacro.h"
#include "itkLogger.h"
#include "itkImageSpatialObject.h"



namespace igstk
{
 
namespace Friends 
{

/** class ImageReaderToImageSpatialObject
 * \brief This class is intended to make the connection between the ImageReader
 * and its output, the ImageSpatialObject. With this class it is possible to
 * enforce encapsulation of the Reader and the ImageSpatialObject, and make
 * their GetImage() and SetImage() methods private, so that developers cannot
 * gain access to the ITK or VTK layers of these two classes.
 */
class ImageReaderToImageSpatialObject
{
  public:
    template < class TReader, class TImageSpatialObject >
    static void 
    ConnectImage( const TReader * reader, 
                  TImageSpatialObject * imageSpatialObject )
    {
       imageSpatialObject->SetImage( reader->GetITKImage() );  
    }

}; // end of ImageReaderToImageSpatialObject class

} // end of Friend namespace


  
/** \class ImageReader
 * 
 * \brief This class reads image data stored in files and provide
 * pointers to the image data for use in either a VTK or an ITK
 * pipeline. This class has pixeltype and dimension template parameters
 *
 * \ingroup Object
 */

template < class TImageSpatialObject >
class ImageReader : public itk::Object
{

public:
  /** typedef for LoggerType */
  typedef itk::Logger                    LoggerType;

  /** Typedefs */
  typedef ImageReader                           Self;
  typedef itk::Object                           Superclass;
  typedef itk::SmartPointer<Self>               Pointer;
  typedef itk::SmartPointer<const Self>         ConstPointer;


  /** Some convenient typedefs for input image */
  typedef TImageSpatialObject                         ImageSpatialObjectType;
  typedef typename ImageSpatialObjectType::ImageType  ImageType;
  typedef typename ImageType::ConstPointer            ImagePointer;
  typedef typename ImageType::RegionType              ImageRegionType; 

  /**  Run-time type information (and related methods). */
  igstkTypeMacro( ImageReader, Object );

  /** Method for creation of a reference counted object. */
  igstkNewMacro( Self );



  /** Return the output of the reader as a ImageSpatialObject */
  const ImageSpatialObjectType * GetOutput() const;
  

  /** Declare the ImageReaderToImageSpatialObject class to be a friend 
   *  in order to give it access to the private method GetITKImage(). */
  igstkFriendClassMacro( igstk::Friends::ImageReaderToImageSpatialObject );

protected:

  ImageReader( void );
  ~ImageReader( void ) ;

  /** Print the object information in a stream. */
  void PrintSelf( std::ostream& os, itk::Indent indent ) const;

  /** Declarations needed for the Logger */
  igstkLoggerMacro();

  /** Connect the ITK image to the output ImageSpatialObject */
  void ConnectImage();

private:
  
  typename ImageSpatialObjectType::Pointer   m_ImageSpatialObject;

  // FIXME : This must be replaced with StateMachine logic
  virtual const ImageType * GetITKImage() const { return NULL; }
    
};

} // end namespace igstk

#ifndef IGSTK_MANUAL_INSTANTIATION
#include "igstkImageReader.txx"
#endif

#endif // __igstkImageReader_h


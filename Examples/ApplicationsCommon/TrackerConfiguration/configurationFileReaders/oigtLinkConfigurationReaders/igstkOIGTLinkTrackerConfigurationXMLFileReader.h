#ifndef __igstkOIGTLinkTrackerConfigurationXMLFileReader_h
#define __igstkOIGTLinkTrackerConfigurationXMLFileReader_h

#include <itkXMLFile.h>

namespace igstk
{
/** \class OIGTLinkTrackerConfigurationXMLFileReader
 * 
 *  \brief This class is a reader for xml files containing a 
 *         tracker configuration with open IGT link settings. That is for

 */
class OIGTLinkTrackerConfigurationXMLFileReader : public itk::XMLReaderBase 
{
public:
  /**
   * This is the exception that is thrown if there is a problem with the
   * file format (does not follow the expected format).
   */
  class FileFormatException : public std::exception
  {
  public:
    /**
     * Construct an exception with a specifc message.
     */
    FileFormatException(const std::string &errorMessage) 
    {
      this->errorMessage = errorMessage;
    }

    /**
     * Virtual destructor.
     */
    virtual ~FileFormatException() throw() {}

    /**
     * Get the error message.
     */
     virtual const char* what() const throw() 
     {
       return errorMessage.c_str();
     }
  private:
    std::string errorMessage;
  };

    //standard typedefs
  typedef OIGTLinkTrackerConfigurationXMLFileReader    Self;
  typedef itk::XMLReaderBase                           Superclass;
  typedef itk::SmartPointer<Self>                      Pointer;

           //run-time type information (and related methods)
  itkTypeMacro( OIGTLinkTrackerConfigurationXMLFileReader, itk::XMLReaderBase );

          //method for creation through the object factory
  itkNewMacro( Self );

  /**
   * Check that the given name isn't a zero length string, that the file exists, 
   * and that it isn't a directory.
   */
  virtual int CanReadFile(const char* name);

  /**
   * Method called when a new xml tag start is encountered.
   */
  virtual void StartElement( const char * name, const char **atts ); 

  /**
   * Method called when an xml tag end is encountered.
   */
  virtual void EndElement( const char *name ); 

  /**
   * Method for handling the data inside an xml tag.
   */
  virtual void CharacterDataHandler( const char *inData, int inLength );


  /**
   * Get the map between tool names and ip ports.
   */
  void GetOIGTLinkToolConfigurationData( 
    std::map<std::string, unsigned int> &toolNamesAndPorts );
  
  std::string GetOIGTLinkHostName();   

  bool HaveConfigurationData();

protected:
          //this is the constructor that is called by the factory to 
         //create a new object
  OIGTLinkTrackerConfigurationXMLFileReader() : 
                                 m_ReadingTrackerConfiguration( false ),                             
                                 m_ReadingToolConfiguration( false ),
                                 m_HaveCurrentToolIPPort( false )
                                 {}
  virtual ~OIGTLinkTrackerConfigurationXMLFileReader() {}

  void ProcessToolName() 
    throw ( FileFormatException );

  void ProcessToolIPPort() 
    throw ( FileFormatException );
  
  void ProcessHostName()
    throw ( FileFormatException );

  void ProcessToolData()
    throw ( FileFormatException );

  bool m_ReadingTrackerConfiguration;
  bool m_ReadingToolConfiguration;
  
  std::string m_CurrentTagData;
  std::string m_CurrentToolName;
  unsigned int m_CurrentToolIPPort;
  bool m_HaveCurrentToolIPPort;

  std::map<std::string, unsigned int> m_ToolNamesAndPorts;   
  std::string m_HostName; 

private:
  OIGTLinkTrackerConfigurationXMLFileReader( 
    const OIGTLinkTrackerConfigurationXMLFileReader & other );


  const OIGTLinkTrackerConfigurationXMLFileReader & operator=( 
    const OIGTLinkTrackerConfigurationXMLFileReader & right );
};


}
#endif //__igstkOIGTLinkTrackerConfigurationXMLFileReader_h
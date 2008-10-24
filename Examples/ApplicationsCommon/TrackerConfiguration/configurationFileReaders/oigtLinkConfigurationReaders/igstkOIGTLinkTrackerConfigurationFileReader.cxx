#include <itksys/SystemTools.hxx>
#include "igstkOIGTLinkTrackerConfigurationFileReader.h"
#include "igstkOIGTLinkTrackerConfigurationXMLFileReader.h"

namespace igstk
{ 

OIGTLinkTrackerConfigurationFileReader::OIGTLinkTrackerConfigurationFileReader() : 
  m_StateMachine( this )
{
        //define the state machine's states 
  igstkAddStateMacro( Idle );
  igstkAddStateMacro( AttemptingSetFileName );
  igstkAddStateMacro( AttemptingSetReader );
  igstkAddStateMacro( FileNameSet );
  igstkAddStateMacro( ReaderSet );
  igstkAddStateMacro( AttemptingSetReaderHaveFileName );
  igstkAddStateMacro( AttemptingSetFileNameHaveReader );
  igstkAddStateMacro( Initialized );  
  igstkAddStateMacro( AttemptingRead );
  igstkAddStateMacro( HaveData );

           //define the state machines inputs
  igstkAddInputMacro( SetReader );
  igstkAddInputMacro( SetFileName );
  igstkAddInputMacro( Read );
  igstkAddInputMacro( GetData  );
  igstkAddInputMacro( Success );
  igstkAddInputMacro( Failure );

            //define the state machine's transitions
  igstkAddTransitionMacro( Idle,
                           SetReader,
                           AttemptingSetReader,
                           SetReader );
  igstkAddTransitionMacro( Idle,
                           SetFileName,
                           AttemptingSetFileName,
                           SetFileName );
  igstkAddTransitionMacro( Idle,
                           Read,
                           Idle,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( Idle,
                           GetData,
                           Idle,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( Idle,
                           Success,
                           Idle,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( Idle,
                           Failure,
                           Idle,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingSetFileName,
                           SetReader,
                           AttemptingSetFileName,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetFileName,
                           SetFileName,
                           AttemptingSetFileName,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetFileName,
                           Read,
                           AttemptingSetFileName,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetFileName,
                           GetData,
                           AttemptingSetFileName,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetFileName,
                           Success,
                           FileNameSet,
                           ReportSetFileNameSuccess );
  igstkAddTransitionMacro( AttemptingSetFileName,
                           Failure,
                           Idle,
                           ReportSetFileNameFailure );

  igstkAddTransitionMacro( AttemptingSetReader,
                           SetReader,
                           AttemptingSetReader,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetReader,
                           SetFileName,
                           AttemptingSetReader,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetReader,
                           Read,
                           AttemptingSetReader,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetReader,
                           GetData,
                           AttemptingSetReader,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetReader,
                           Success,
                           ReaderSet,
                           ReportSetReaderSuccess );
  igstkAddTransitionMacro( AttemptingSetReader,
                           Failure,
                           Idle,
                           ReportSetReaderFailure );

  igstkAddTransitionMacro( FileNameSet,
                           SetReader,
                           AttemptingSetReaderHaveFileName,
                           SetReader );
  igstkAddTransitionMacro( FileNameSet,
                           SetFileName,
                           AttemptingSetFileName,
                           SetFileName );
  igstkAddTransitionMacro( FileNameSet,
                           Read,
                           FileNameSet,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( FileNameSet,
                           GetData,
                           FileNameSet,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( FileNameSet,
                           Success,
                           FileNameSet,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( FileNameSet,
                           Failure,
                           FileNameSet,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( ReaderSet,
                           SetReader,
                           AttemptingSetReader,
                           SetReader );
  igstkAddTransitionMacro( ReaderSet,
                           SetFileName,
                           AttemptingSetFileNameHaveReader,
                           SetFileName );
  igstkAddTransitionMacro( ReaderSet,
                           Read,
                           ReaderSet,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( ReaderSet,
                           GetData,
                           ReaderSet,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( ReaderSet,
                           Success,
                           ReaderSet,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( ReaderSet,
                           Failure,
                           ReaderSet,
                           ReportInvalidRequest );

  igstkAddTransitionMacro( AttemptingSetReaderHaveFileName,
                           SetReader,
                           AttemptingSetReaderHaveFileName,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetReaderHaveFileName,
                           SetFileName,
                           AttemptingSetReaderHaveFileName,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetReaderHaveFileName,
                           Read,
                           AttemptingSetReaderHaveFileName,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetReaderHaveFileName,
                           GetData,
                           AttemptingSetReaderHaveFileName,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetReaderHaveFileName,
                           Success,
                           Initialized,
                           ReportSetReaderSuccess );
  igstkAddTransitionMacro( AttemptingSetReaderHaveFileName,
                           Failure,
                           FileNameSet,
                           ReportSetReaderFailure );

  igstkAddTransitionMacro( AttemptingSetFileNameHaveReader,
                           SetReader,
                           AttemptingSetFileNameHaveReader,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetFileNameHaveReader,
                           SetFileName,
                           AttemptingSetFileNameHaveReader,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetFileNameHaveReader,
                           Read,
                           AttemptingSetFileNameHaveReader,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetFileNameHaveReader,
                           GetData,
                           AttemptingSetFileNameHaveReader,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingSetFileNameHaveReader,
                           Success,
                           Initialized,
                           ReportSetFileNameSuccess );
  igstkAddTransitionMacro( AttemptingSetFileNameHaveReader,
                           Failure,
                           ReaderSet,
                           ReportSetFileNameFailure );

  igstkAddTransitionMacro( Initialized,
                           SetReader,
                           AttemptingSetReaderHaveFileName,
                           SetReader );
  igstkAddTransitionMacro( Initialized,
                           SetFileName,
                           AttemptingSetFileNameHaveReader,
                           SetFileName );
  igstkAddTransitionMacro( Initialized,
                           Read,
                           AttemptingRead,
                           Read );
  igstkAddTransitionMacro( Initialized,
                           GetData,
                           Initialized,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( Initialized,
                           Success,
                           Initialized,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( Initialized,
                           Failure,
                           Initialized,
                           ReportInvalidRequest );
  
  igstkAddTransitionMacro( AttemptingRead,
                           SetReader,
                           AttemptingRead,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingRead,
                           SetFileName,
                           AttemptingRead,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingRead,
                           Read,
                           AttemptingRead,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingRead,
                           GetData,
                           AttemptingRead,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( AttemptingRead,
                           Success,
                           HaveData,
                           ReportReadSuccess );
  igstkAddTransitionMacro( AttemptingRead,
                           Failure,
                           Initialized,
                           ReportReadFailure );

  igstkAddTransitionMacro( HaveData,
                           SetReader,
                           AttemptingSetReaderHaveFileName,
                           SetReader );
  igstkAddTransitionMacro( HaveData,
                           SetFileName,
                           AttemptingSetFileNameHaveReader,
                           SetFileName );
  igstkAddTransitionMacro( HaveData,
                           Read,
                           AttemptingRead,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( HaveData,
                           GetData,
                           HaveData,
                           GetData );
  igstkAddTransitionMacro( HaveData,
                           Success,
                           HaveData,
                           ReportInvalidRequest );
  igstkAddTransitionMacro( HaveData,
                           Failure,
                           HaveData,
                           ReportInvalidRequest );  

          //set the initial state of the state machine
  igstkSetInitialStateMacro( Idle );

         // done setting the state machine, ready to run
  this->m_StateMachine.SetReadyToRun();  
}


OIGTLinkTrackerConfigurationFileReader::~OIGTLinkTrackerConfigurationFileReader()
{
}

void 
OIGTLinkTrackerConfigurationFileReader::RequestSetReader( 
               TrackerConfigurationXMLFileReaderBase::Pointer transformReader )
{
  igstkLogMacro( DEBUG, "igstk::TrackerConfigurationFileReader::"
                        "RequestSetReader called...\n" );
  this->m_TmpXMLFileReader = transformReader;

  igstkPushInputMacro( SetReader );
  this->m_StateMachine.ProcessInputs();

}


void 
OIGTLinkTrackerConfigurationFileReader::RequestSetFileName( const std::string &fileName )
{
  igstkLogMacro( DEBUG, "igstk::TrackerConfigurationFileReader::"
                        "RequestSetFileName called...\n" );
  this->m_TmpFileName = fileName;

  igstkPushInputMacro( SetFileName );
  this->m_StateMachine.ProcessInputs();
}


void 
OIGTLinkTrackerConfigurationFileReader::RequestRead()
{
  igstkLogMacro( DEBUG, "igstk::TrackerConfigurationFileReader::"
                        "RequestRead called...\n" );
  
  igstkPushInputMacro( Read );
  this->m_StateMachine.ProcessInputs();
}


void 
OIGTLinkTrackerConfigurationFileReader::RequestGetData()
{
  igstkLogMacro( DEBUG, "igstk::TrackerConfigurationFileReader::"
                        "RequestGetData called...\n" );
  
  igstkPushInputMacro( GetData );
  this->m_StateMachine.ProcessInputs();
}


void 
OIGTLinkTrackerConfigurationFileReader::ReportInvalidRequestProcessing()
{
  igstkLogMacro( DEBUG, "igstk::TrackerConfigurationFileReader::"
                 "ReportInvalidRequestProcessing called...\n" );
  this->InvokeEvent( InvalidRequestErrorEvent() );

}


void 
OIGTLinkTrackerConfigurationFileReader::SetReaderProcessing()
{
  if( this->m_TmpXMLFileReader.IsNull() )
  {
    igstkPushInputMacro( Failure );
  }
  else
  {
    this->m_XMLFileReader = this->m_TmpXMLFileReader;
    this->m_TmpXMLFileReader = NULL;
    igstkPushInputMacro( Success );
  }
  this->m_StateMachine.ProcessInputs();
}


void 
OIGTLinkTrackerConfigurationFileReader::ReportSetReaderSuccessProcessing()
{
  igstkLogMacro( DEBUG,
                 "igstk::TrackerConfigurationFileReader::"
                 "ReportSetReaderSuccessProcessing called...\n");
  this->InvokeEvent( SetReaderSuccessEvent() );
}


void 
OIGTLinkTrackerConfigurationFileReader::ReportSetReaderFailureProcessing()
{
  igstkLogMacro( DEBUG,
                 "igstk::TrackerConfigurationFileReader::"
                 "ReportSetReaderFailureProcessing called...\n");
  this->InvokeEvent( SetReaderFailureEvent() );
}


void 
OIGTLinkTrackerConfigurationFileReader::SetFileNameProcessing()
{
  if( this->m_TmpFileName.empty() )
  {
    igstkPushInputMacro( Failure );
  }
  else
  {
    this->m_FileName = this->m_TmpFileName;
    this->m_TmpFileName.clear();
    igstkPushInputMacro( Success );
  }
  this->m_StateMachine.ProcessInputs();
}


void 
OIGTLinkTrackerConfigurationFileReader::ReportSetFileNameSuccessProcessing()
{
  igstkLogMacro( DEBUG,
                 "igstk::TrackerConfigurationFileReader::"
                 "ReportSetFileNameSuccessProcessing called...\n");
  this->InvokeEvent( SetFileNameSuccessEvent() );
}


void 
OIGTLinkTrackerConfigurationFileReader::ReportSetFileNameFailureProcessing()
{
  igstkLogMacro( DEBUG,
                 "igstk::TrackerConfigurationFileReader::"
                 "ReportSetFileNameFailureProcessing called...\n");
  this->InvokeEvent( SetFileNameFailureEvent() );
}


void 
OIGTLinkTrackerConfigurationFileReader::ReadProcessing()
{
            //check that the file name is valid, the file exists (size greater
            //than zero), and is not a directory
  if(itksys::SystemTools::FileLength( this->m_FileName.c_str() ) == 0 ||
    itksys::SystemTools::FileIsDirectory( this->m_FileName.c_str() ) )
  {
    std::ostringstream msg;
    msg<<"File ("<<this->m_FileName<<") does not exist or is a directory";
    this->m_ReadFailureErrorMessage = msg.str();
    igstkPushInputMacro( Failure );    
  }
  else
  {
    this->m_XMLFileReader->SetFilename( this->m_FileName );
    try {       //read the xml file
      this->m_XMLFileReader->GenerateOutputInformation();
                    //xml reading is successful (no exception generation) even 
                    //when the file is empty, need to check that the data was
                    //actually read
      if( this->m_XMLFileReader->HaveConfigurationData() )
      {
        this->m_OIGTLinkTrackerConfiguration.m_TrackerConfiguration = 
          this->m_XMLFileReader->GetTrackerConfigurationData();
              //we have read the standard igstk tracker congifuration
              //now read the openIGTLink information
        OIGTLinkTrackerConfigurationXMLFileReader::Pointer oigtLinkXmlReader =  
          OIGTLinkTrackerConfigurationXMLFileReader::New();
        oigtLinkXmlReader->SetFilename( this->m_FileName );
        oigtLinkXmlReader->GenerateOutputInformation();
        if( !oigtLinkXmlReader->HaveConfigurationData() )
        {
          this->m_ReadFailureErrorMessage = 
            "XML file does not contain openIGTLink configuration data"; 
          igstkPushInputMacro( Failure )
        }
        else 
        {
          oigtLinkXmlReader->GetOIGTLinkToolConfigurationData( 
            this->m_OIGTLinkTrackerConfiguration.m_ToolNamesAndPorts );
          this->m_OIGTLinkTrackerConfiguration.m_HostName =
            oigtLinkXmlReader->GetOIGTLinkHostName();
          igstkPushInputMacro( Success );    
        }
      }     //we successfuly read the file, it just didn't contain the data, so
            //data loading is considered a failure
      else
      {
        this->m_ReadFailureErrorMessage = 
          "XML file does not contain tracker configuration data"; 
        igstkPushInputMacro( Failure );    
      }
    }
               //an itk exception is thrown if the xml is malformed, and a 
               //FileFormatException is thrown if the data format is incorrect.
               //both are decendants of std::exception 
    catch( std::exception &e ) 
    { 
      this->m_ReadFailureErrorMessage = e.what();
      igstkPushInputMacro( Failure );    
    }
  }
  this->m_StateMachine.ProcessInputs();
}


void 
OIGTLinkTrackerConfigurationFileReader::ReportReadSuccessProcessing()
{
  igstkLogMacro( DEBUG,
                 "igstk::TrackerConfigurationFileReader::"
                 "ReportReadSuccessProcessing called...\n");
  this->InvokeEvent( ReadSuccessEvent() );
}


void 
OIGTLinkTrackerConfigurationFileReader::ReportReadFailureProcessing()
{
  ReadFailureEvent evt;
  evt.Set( this->m_ReadFailureErrorMessage );

  igstkLogMacro( DEBUG,
                 "igstk::TrackerConfigurationFileReader::"
                 "ReportReadFailureProcessing called...\n");
  this->InvokeEvent( evt );
}


void 
OIGTLinkTrackerConfigurationFileReader::GetDataProcessing()
{
  OIGTLinkConfigurationDataEvent evt;
  evt.Set( &this->m_OIGTLinkTrackerConfiguration );

  igstkLogMacro( DEBUG,
                 "igstk::TrackerConfigurationFileReader::"
                 "GetDataProcessing called...\n");
  this->InvokeEvent( evt );
}

void  
OIGTLinkTrackerConfigurationFileReader::PrintSelf( std::ostream& os, 
                                     itk::Indent indent ) const
{
}


}//namespace

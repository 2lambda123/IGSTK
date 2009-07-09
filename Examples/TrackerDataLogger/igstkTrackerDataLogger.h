/*=========================================================================

  Program:   Image Guided Surgery Software Toolkit
  Module:    igstkTrackerDataLogger.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) ISC  Insight Software Consortium.  All rights reserved.
  See IGSTKCopyright.txt or http://www.igstk.org/copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __igstkTrackerDataLogger_h
#define __igstkTrackerDataLogger_h

// BeginLatex
// 
// This example illustrates how to export tracking data to a file.
// The example program supports output to several files at once.
//
// EndLatex
#include "igstkTrackerConfiguration.h"
#include "igstkTrackerController.h"
#include "igstkTransformObserver.h"
#include "igstkTrackerDataLoggerConfigurationFileReader.h" 
#include "igstkTrackerDataLoggerConfigurationXMLFileReader.h"
#include <fstream>

/**
 * \class TrackerDataLogger
 * \brief This class performs output of tracking data to output file. 
 *
 * The class is instantiated with the name of the xml file containing the 
 * tracker. The user can then start and stop the tracking.
 * 
 */
class TrackerDataLogger
{
public:

  /**
   * \class This class was created due to the need for platform independence. 
   * In windows the std::exception class has a constructor which has a string as
   * payload, sadly in linux/unix this constructor does not exist. 
   */
  class ExceptionWithMessage : public std::exception
    {
    public:
    ExceptionWithMessage (const std::string & str) throw()
      {
      this->m_Str = str;
      }
    virtual ~ExceptionWithMessage() throw () {} 

    virtual const char *what() const throw ()
      {
      return this->m_Str.c_str();
      }
    private:
      std::string m_Str;
    };

  TrackerDataLogger( std::string &trackerXMLConfigurationFileName ) 
    throw ( ExceptionWithMessage );

  ~TrackerDataLogger(){ }

  void StartTracking();

  void StopTracking();

  void ExitSuccessfully()
    {
    exit( EXIT_SUCCESS ); 
    }      
    igstkGetMacro( InitialTimeStamp, double );
    igstkGetMacro( TimeLimit, double );
    igstkSetMacro( InitialTimeStamp, double );
    igstkSetMacro( TimeLimit, double );

private:

  /**
  * Observer for the event generated by 
  * TrackerConfigurationLoggerFileReader->RequestGetData() method.
  */
  igstkObserverMacro( TrackerConfiguration, 
    igstk::TrackerDataLoggerConfigurationFileReader::
      ConfigurationDataEvent, 
    igstk::TrackerDataLoggerConfigurationFileReader::
      ConfigurationDataPointerType )

    /**
    * Observer for the TrackerController->RequestInitialize() failure.
    */
    igstkObserverMacro( InitializeError, 
    igstk::TrackerController::InitializeErrorEvent, 
    std::string )


    /**
    * Observer for the TrackerController->RequestStartTracking() failure.
    */
    igstkObserverMacro( StartTrackingError, 
    igstk::TrackerStartTrackingErrorEvent,
    std::string )

    /**
    * Observer for the TrackerController->RequestStopTracking() failure.
    */
    igstkObserverMacro( StopTrackingError, 
    igstk::TrackerStopTrackingErrorEvent,
    std::string )

    /**
    * Observer for the TrackerController->RequestGetNonReferenceToolList() 
    */
    igstkObserverMacro( ToolList, 
    igstk::TrackerController::RequestToolsEvent, 
    igstk::TrackerController::ToolContainerType )

    /**
    * Observer for the TrackerController->RequestGetTool() and  
    * TrackerController->RequestGetReferenceTool()
    */
    igstkObserverMacro( Tool, 
    igstk::TrackerController::RequestToolEvent, 
    igstk::TrackerController::ToolEntryType )

/**
 * This class observes the TrackerToolTransformUpdateEvent for a specific tool.
 * It checks that the event is for the relevant tool and then gets the tool's
 * transform w.r.t. the "world" coordinate system. The transform is then 
 * broadcasted to all destinations.
 */
// BeginLatex
// \code{ToolUpdatedObserver} class observes the
// TrackerToolTransformUpdateEvent for a specific tool. It checks that
// the event is for the relevant tool and then gets the tool's
// transform
// to its parent and sends it via socket.
// EndLatex

  class ToolUpdatedObserver : public ::itk::Command
    {
    public:
      typedef  ToolUpdatedObserver                 Self;
      typedef  ::itk::Command                      Superclass;
      typedef  ::itk::SmartPointer<Self>           Pointer;
      itkNewMacro( Self );

          //typedef ofstream*                            OfstreamPointer;
          
    protected:
// BeginLatex
// In the \code{ToolUpdatedObserver} constructor  we instantiate a 
// PositionMessage, which is updated with the transformation data.
// EndLatex

      ToolUpdatedObserver()
      {
      this->m_TransformObserver = igstk::TransformObserver::New();
// BeginCodeSnippet
      //this->m_PositionMessage = igtl::PositionMessage::New();
// EndCodeSnippet
      }

      ~ToolUpdatedObserver()
      {
      std::vector< ofstream* >::iterator it; 
      for (it = this->m_Ofstreams.begin(); it != this->m_Ofstreams.end(); ++it)
        {
        (*it)->close(); 
        delete (*it);
        }
      this->m_Ofstreams.clear();
      }

    public:

      void Initialize(
        TrackerDataLogger *logger, 
        const std::string toolName, 
        igstk::TrackerTool::Pointer trackerTool,
        igstk::SpatialObject::Pointer world,
        std::vector< std::string > & outputFileNames)
      {
      this->m_Logger = logger;
      this->m_Tool = trackerTool;
      this->m_World = world;
      this->m_TransformObserver->ObserveTransformEventsFrom( this->m_Tool );

      this->m_PositionMessage.assign( toolName.c_str() );

      std::vector< ofstream* >::iterator it; 
      for (it = this->m_Ofstreams.begin(); it != this->m_Ofstreams.end(); ++it)
        {
        (*it)->close();
        delete (*it);
        }
        this->m_Ofstreams.clear();

      // BeginLatex
      // We create a list of output files to write to.
      // Then we open each file for writing. 
      // EndLatex
      // BeginCodeSnippet
      std::vector< std::string >::iterator outputFileNamesIt; 
      for ( outputFileNamesIt = outputFileNames.begin();
            outputFileNamesIt != outputFileNames.end(); ++outputFileNamesIt)
        {
        ofstream *outputFile = new ofstream ( outputFileNamesIt->c_str() , ios::out );
        //outputFile.open();
        
        if ( !outputFile->is_open() )
          {
          for (it = this->m_Ofstreams.begin(); it != this->m_Ofstreams.end(); ++it)
          {
          (*it)->close();
          delete (*it);
          }
        this->m_Ofstreams.clear();
        std::ostringstream msg;
        msg<<" Failed to open file  " <<  (*outputFileNamesIt);
        throw ExceptionWithMessage( msg.str() );
        }

        this->m_Ofstreams.push_back(outputFile);
        }
      // EndCodeSnippet
      }

    void Execute(itk::Object *caller, const itk::EventObject & event)
    {
      const itk::Object * constCaller = caller;
      this->Execute( constCaller, event );
    }

// BeginLatex
// In \emph{ToolUpdatedObserver::Excecute()}, we define the event
// handler to receive a transform, fill the Logger message, and
// send it out.
// EndLatex

    void Execute(const itk::Object *caller, const itk::EventObject & event)
    {

      //if no outputFiles open for writing, just return
      if( m_Ofstreams.empty() )
        {
        return;
        }

      //do something only for the correct tool
      if( this->m_Tool.GetPointer() == caller )
        {               //the tool transform has been updated, get it
        if( dynamic_cast<const
            igstk::TrackerToolTransformUpdateEvent  *>( &event) )
          {                 //request to get the transform 
          this->m_Tool->RequestComputeTransformTo( this->m_World );
          //check that we got it
          if ( this->m_TransformObserver->GotTransform() )
            {
            // BeginCodeSnippet
            char buffer[100];
            
            double timeStamp = igstk::RealTimeClock::GetTimeStamp();
            if (this->m_Logger->GetInitialTimeStamp() < 0.0)
              {
              this->m_Logger->SetInitialTimeStamp( timeStamp );
              }
            
            double timeDifference = timeStamp - this->m_Logger->GetInitialTimeStamp();            
            if (this->m_Logger->GetTimeLimit() > 0.0)
              {
              //std::cerr << " timeDifference " << timeDifference << "\n"; 
              if (timeDifference > this->m_Logger->GetTimeLimit() )
                {
                //std::cerr << " want to stop tracking  \n"; 
                this->m_Logger->StopTracking();
                this->m_Logger->ExitSuccessfully();
                }
              }
                
            igstk::Transform transform =
              this->m_TransformObserver->GetTransform();
            igstk::Transform::VectorType t = transform.GetTranslation();
            igstk::Transform::VersorType r = transform.GetRotation();
            sprintf(buffer, "%f  %f %f %f  %f %f %f %f \n", timeDifference , t[0], t[1], t[2],
                    r.GetX(), r.GetY(), r.GetZ(), r.GetW());

            //std::cerr << buffer;
            
            this->m_PositionMessage.assign(buffer); 

            std::vector< ofstream * >::iterator it; 
            for (it = this->m_Ofstreams.begin(); it != this->m_Ofstreams.end();
                 ++it)
              {
                 (*(*it)) << this->m_PositionMessage; 
              }
            // EndCodeSnippet
            }
          }
        }
    }


    private:
    std::string m_HostName;
    unsigned int m_PortNumber;  
    //we are interested in the tool location relative to the world's 
    //coordinate system
    igstk::SpatialObject::Pointer m_World;
    igstk::TransformObserver::Pointer m_TransformObserver; 
    igstk::TrackerTool::Pointer  m_Tool;
    TrackerDataLogger *m_Logger;

    //send data to these ofstreams 
    std::vector< ofstream* > m_Ofstreams;
    std::string m_PositionMessage; 
    };

  private:

  igstk::TrackerDataLoggerConfigurationFileReader::ConfigurationDataType *
    GetTrackerConfiguration( std::string &configurationFileName) 
    throw ( ExceptionWithMessage );

  igstk::TrackerController::Pointer m_TrackerController;
  double m_InitialTimeStamp;
  double m_TimeLimit; 
      
};
   

#endif //__igstkTrackerDataLogger_h

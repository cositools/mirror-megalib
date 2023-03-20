/*
* MAssembly.cxx
*
*
* Copyright (C) by Andreas Zoglauer.
* All rights reserved.
*
*
* This code implementation is the intellectual property of
* Andreas Zoglauer.
*
* By copying, distributing or modifying the Program (or any work
* based on the Program) you indicate your acceptance of this statement,
* and all its terms.
*
*/


////////////////////////////////////////////////////////////////////////////////
//
// MAssembly
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MAssembly.h"

// Standard libs:
#include <iostream>
#include <sstream>
#include <vector>
#include <csignal>
using namespace std;

// ROOT libs:
#include "TROOT.h"
#include "TCanvas.h"
#include "TView.h"
#include "TGMsgBox.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TApplication.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MAssert.h"
#include "MStreams.h"
#include "MString.h"
#include "MTimer.h"
#include "MFile.h"
#include "MGUIExpoCombinedViewer.h"
#include "MGUIMainFretalon.h"
#include "MReadOutAssembly.h"
#include "MModule.h"
#include "MModuleConverterSimRoa.h"
#include "MModuleLoaderRoa.h"
#include "MModuleTransmitterRealta.h"
#include "MModuleSaver.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MAssembly)
#endif


////////////////////////////////////////////////////////////////////////////////


MAssembly::MAssembly()
{
  // standard constructor
    
  m_Interrupt = false;
  m_UseGui = true;
  
  m_Supervisor = MSupervisor::GetSupervisor();
  
  MString ConfigurationFileName = "~/.pixeldetector.cfg";
  MFile::ExpandFileName(ConfigurationFileName);
  m_Supervisor->SetConfigurationFileName(ConfigurationFileName);
  
  m_Supervisor->AddAvailableModule(new MModuleConverterSimRoa());
  m_Supervisor->AddAvailableModule(new MModuleLoaderRoa());
  m_Supervisor->AddAvailableModule(new MModuleTransmitterRealta());
  m_Supervisor->AddAvailableModule(new MModuleSaver());  

  m_Supervisor->Load();
  
  m_Supervisor->SetUIProgramName("Pixelizer");
  //m_Supervisor->SetUIPicturePath("$(NUCLEARIZER)/resource/icons/Nuclearizer.xpm");
  m_Supervisor->SetUISubTitle("An example program for the Fretalon calibration framework");
  m_Supervisor->SetUILeadAuthor("Andreas Zoglauer");
  //m_Supervisor->SetUICoAuthors("and others");
}



////////////////////////////////////////////////////////////////////////////////


MAssembly::~MAssembly()
{
  // standard destructor
}


////////////////////////////////////////////////////////////////////////////////


bool MAssembly::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: PixelDetector <options>"<<endl;
  Usage<<endl;
  Usage<<"      -c --configuration <filename>.cfg:"<<endl;
  Usage<<"             Use this file as configuration file."<<endl;
  Usage<<"             If no configuration file is give ~/.fretalon.cfg is loaded by default"<<endl;
  Usage<<"      -a --auto:"<<endl;
  Usage<<"             Automatically start analysis without GUI"<<endl;
  Usage<<"      -m --multithreading:"<<endl;
  Usage<<"             0: false (default), else: true"<<endl;
  Usage<<"      -v --verbosity:"<<endl;
  Usage<<"             Verbosity: 0: Quiet, 1: Errors, 2: Warnings, 3: Info"<<endl;
  Usage<<"      -h --help:"<<endl;
  Usage<<"             You know the answer..."<<endl;
  Usage<<endl;
  
  // Store some options temporarily:
  MString Option;
  
  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }
  
  // First check if all options are ok:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    
    // Single argument
    if (Option == "-c" || Option == "--configuration" ||
        Option == "-m" || Option == "--multithreading") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
  }
  
  // Now parse all low level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--configuration" || Option == "-c") {
      m_Supervisor->Load(argv[++i]);
      cout<<"Command-line parser: Use configuration file "<<argv[i]<<endl;
    } else if (Option == "--verbosity" || Option == "-v") {
      g_Verbosity = atoi(argv[++i]);
      cout<<"Command-line parser: Verbosity "<<g_Verbosity<<endl;
    } else if (Option == "--multithreading" || Option == "-m") {
      m_Supervisor->UseMultiThreading((atoi(argv[i]) != 0 ? true : false));
      cout<<"Command-line parser: Using multithreading: "<<(atoi(argv[i]) != 0 ? "yes" : "no")<<endl;
    } else if (Option == "--auto" || Option == "-a") {
      // Parse later
    }
  }
  
  // Now parse all high level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--auto" || Option == "-a") {
      m_UseGui = false;
      gROOT->SetBatch(true);
      m_Supervisor->Analyze();
      m_Supervisor->Exit();
      return false;
    }
  }
  
  if (m_UseGui == true) {
    if (m_Supervisor->LaunchUI() == false) {
      return false; 
    }
  }
  
  return true;
}


// MAssembly: the end...
////////////////////////////////////////////////////////////////////////////////

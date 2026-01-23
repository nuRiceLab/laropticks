#include <vector>
#include <string>
#include <sstream>
#include <iostream>
//#include "TGDMLParse.h"
#include "TSystem.h"
#include "TGeoManager.h"

void dune10kt_checkOvlp(bool nowires = false)
{
  gSystem->Load("libGeom");
  gSystem->Load("libGdml");

  std::vector<std::string> geoms;
    geoms.push_back("dune10kt_v5_refactored_1x2x6_nowires_NoField");
  //  geoms.push_back("pdune_v2_paddles");
  //  geoms.push_back("protodune_v7_74ch");
  //    geoms.push_back("dunevd10kt_3view_30deg_v2_refactored_1x8x6ref");
  //  geoms.push_back("dunevd10kt_3view_v2_refactored_1x8x6ref");
  //  geoms.push_back("dunevd10kt_2view_v1_refactored_1x8x6ref");
  //geoms.push_back("dunevd_v5");
//  geoms.push_back("dune10kt_v1_45deg");
//  geoms.push_back("dune10kt_v1_45deg_workspace");
//  geoms.push_back("dune10kt_v1_3mmpitch");
//  geoms.push_back("dune10kt_v1_3mmpitch_workspace");

  for(unsigned int i=0; i<geoms.size(); i++){
    std::cout << "\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n" << std::endl;

    std::stringstream file;
    file << geoms[i];
    //    if( nowires ) file << "_nowires";
    file << ".gdml";

    std::cout << "Checking " << file.str() << "\n" << std::endl;

    TGeoManager::Import(file.str().c_str());
    gGeoManager->GetTopNode();
    gGeoManager->CheckOverlaps(1e-8);
    gGeoManager->PrintOverlaps();
  }
}

void dune10kt_checkOvlpNowires(bool nowires = true)
{
  gSystem->Load("libGeom");
  gSystem->Load("libGdml");

  std::vector<std::string> geoms;
  //  geoms.push_back("dune10kt_v4_1x2x6");
  //  geoms.push_back("pdune_v2_paddles");
  //  geoms.push_back("protodune_v7_74ch");
  //    geoms.push_back("dunevd10kt_3view_30deg_v2_refactored_1x8x6ref");
  //  geoms.push_back("dunevd10kt_3view_v2_refactored_1x8x6ref");
  //    geoms.push_back("dunevd10kt_2view_v2_refactored_1x8x6ref");
      geoms.push_back("dunevd_v5");
//  geoms.push_back("dune10kt_v1_45deg");
//  geoms.push_back("dune10kt_v1_45deg_workspace");
//  geoms.push_back("dune10kt_v1_3mmpitch");
//  geoms.push_back("dune10kt_v1_3mmpitch_workspace");

  for(unsigned int i=0; i<geoms.size(); i++){
    std::cout << "\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n" << std::endl;

    std::stringstream file;
    file << geoms[i];
    if( nowires ) file << "_nowires";
    file << ".gdml";

    std::cout << "Checking " << file.str() << "\n" << std::endl;

    TGeoManager::Import(file.str().c_str());
    gGeoManager->GetTopNode();
    gGeoManager->CheckOverlaps(1e-5);
    gGeoManager->PrintOverlaps();
  }
}

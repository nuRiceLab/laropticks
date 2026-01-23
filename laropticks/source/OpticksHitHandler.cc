

#include "../include/OpticksHitHandler.h"
#include "SEvt.hh"
#include "G4CXOpticks.hh"
#include "OpticksPhoton.hh"
#include "OpticksGenstep.h"
#include "QSim.hh"

#include "G4RunManager.hh"

namespace phot{

  // Opticks Hit Collection
  // Handles getting hits from opticks to a file
  // Need to implement the backtracer for LArSoft in this class
  OpticksHitHandler * OpticksHitHandler::instance = nullptr;

  void OpticksHitHandler::CollectHits() {

      //Collecting Opticks Photons
      SEvt* sev             = SEvt::Get_EGPU();
      sphoton::Get(sphotons, sev->getHit());
      //auto run= G4RunManager::GetRunManager();
      //G4int eventID=run->GetCurrentEvent()->GetEventID();
      G4int eventID=0;

      for (auto & hit : sphotons){
          OpticksHit ohit= OpticksHit();
          ohit.hit_id=hit.index;
          ohit.sensor_id=hit.identity;
          ohit.x=hit.pos.x;
          ohit.y=hit.pos.y;
          ohit.z=hit.pos.z;
          ohit.polx=hit.pol.x;
          ohit.poly=hit.pol.y;
          ohit.polz=hit.pol.z;
          ohit.momx=hit.mom.x;
          ohit.momy=hit.mom.y;
          ohit.momz=hit.mom.z;
          ohit.time=hit.time;
          ohit.boundary=hit.boundary();
          ohit.wavelength=hit.wavelength;
          hits.push_back(ohit);
      }

      // clear the hits
      sphotons.clear();
      sphotons.shrink_to_fit();
      G4CXOpticks::Get()->reset(eventID);
      QSim::Get()->reset(eventID);
  }

  void OpticksHitHandler::SaveHits(){
      /*
      auto run= G4RunManager::GetRunManager();
      G4int eventID=run->GetCurrentEvent()->GetEventID();
      G4AnalysisManager * analysisManager= G4AnalysisManager::Instance();
      for (auto it : hits){
          analysisManager->FillNtupleIColumn(1,0,eventID);
          analysisManager->FillNtupleIColumn(1,1,it.hit_id);
          analysisManager->FillNtupleIColumn(1,2,it.sensor_id);
          analysisManager->FillNtupleDColumn(1,3,it.x);
          analysisManager->FillNtupleDColumn(1,4,it.y);
          analysisManager->FillNtupleDColumn(1,5,it.z);
          analysisManager->FillNtupleDColumn(1,6,it.time);
          analysisManager->FillNtupleDColumn(1,7,it.wavelength);
          analysisManager->AddNtupleRow(1);
      }
      */
     // Handle Hits Here
     hits.clear();
     hits.shrink_to_fit();
     //G4CXOpticks::Get()->reset(eventID);
     //QSim::Get()->reset(eventID);
  }
}

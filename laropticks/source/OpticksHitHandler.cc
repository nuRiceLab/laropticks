
#include "laropticks/include/OpticksHitHandler.h"

namespace phot{

  // Opticks Hit Collection
  // Handles getting hits from opticks to a file
  // Need to implement the backtracer for LArSoft in this class
  OpticksHitHandler * OpticksHitHandler::instance = nullptr;

  void OpticksHitHandler::CollectHits(int eventID,std::map<int, sim::OBTRHelper> obtrHelpers) {

      //Collecting Opticks Photons
	  std::cout << "Collecting Hits from GPU ...." << std::endl;

	  // --- Get Hits ----
      SEvt* sev             = SEvt::Get_EGPU();
      sphoton::Get(sphotons, sev->getHit());
      //auto run= G4RunManager::GetRunManager();
      //G4int eventID=run->GetCurrentEvent()->GetEventID();
	  //std::cout << "Generating OpDetBacktrackerRecord ...." << std::endl;
	  ///std::map<int,sim::OpDetBacktrackerRecord> btmap;

	  /*
      fTouchableHistory = new G4TouchableHistory();
      auto nav = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
	  nav->LocateGlobalPointAndUpdateTouchable(G4ThreeVector(edep.MidPointX(),edep.MidPointY(),edep.MidPointZ()), fTouchableHistory);
	  fTouchableHistory->GetVolume()->GetLogicalVolume()->GetName();
	  */
	  feventID=eventID;
      for (auto & hit : sphotons) {
          OpticksHit ohit= OpticksHit();
          ohit.hit_id=hit.iindex();
          ohit.parent_id=hit.get_PId();
          ohit.sensor_id=hit.get_identity();
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
		  double pos[3]={ohit.x,ohit.y,ohit.z};
		  obtrHelpers.at(ohit.sensor_id).AddScintillationPhotonsToMap(ohit.parent_id, ohit.time, 1,pos , ohit.wavelength);
      }
	  //
	  SaveHits();
      // clear the hits
      sphotons.clear();
      sphotons.shrink_to_fit();
      G4CXOpticks::Get()->reset(eventID);
      QSim::Get()->reset(eventID);

	  //
  }

  void OpticksHitHandler::SaveHits(){

      G4AnalysisManager * analysisManager= G4AnalysisManager::Instance();
      std::cout << "OpticksHitHandler::SaveHits" << std::endl;
      for (auto it : hits){
          analysisManager->FillNtupleIColumn(0,0,feventID);
          analysisManager->FillNtupleIColumn(0,1,it.parent_id);
          analysisManager->FillNtupleIColumn(0,2,it.hit_id);
          analysisManager->FillNtupleIColumn(0,3,it.sensor_id);
          analysisManager->FillNtupleDColumn(0,4,it.x);
          analysisManager->FillNtupleDColumn(0,5,it.y);
          analysisManager->FillNtupleDColumn(0,6,it.z);
          analysisManager->FillNtupleDColumn(0,7,it.time);
          analysisManager->FillNtupleDColumn(0,8,it.wavelength);
          analysisManager->AddNtupleRow(0);
      }

     // Handle Hits Here
     hits.clear();
     hits.shrink_to_fit();
     //G4CXOpticks::Get()->reset(feventID);
     //QSim::Get()->reset(feventID);
  }
	// BackTracker goes here
  std::vector<sim::OpDetBacktrackerRecord> * OpticksHitHandler::GetOpDetBacktrackerRecords(std::map<int,sim::OpDetBacktrackerRecord> *fOpBTMap,int TrackId,double edep)
  {
	return {};
  }

}

/**
 *  File: include/OpticksHitHandler.h
 *  Author: Ilker Parmaksiz
 *  Experiment: DUNE
 *  Institution: Rice University
 *  Date: 1/9/26
 *  Description: Allows photon hits to be transffered from GPU to CPU.
**/
#pragma once

#ifndef OPTICKS_OPTICKSHITHANDLER_HH
#define OPTICKS_OPTICKSHITHANDLER_HH


#include "lardataobj/Simulation/SimEnergyDeposit.h"
#include "lardataobj/Simulation/OpDetBacktrackerRecord.h"

// Opticks headers
#include "SEvt.hh"
#include "G4CXOpticks.hh"
#include "OpticksPhoton.hh"
#include "OpticksGenstep.h"
#include "QSim.hh"
#include "G4TouchableHistory.hh"

// Geant4
#include "G4Threading.hh"
#include "G4AutoLock.hh"
#include "G4ThreeVector.hh"
#include "G4RunManager.hh"

// Cetlib
#include "cetlib_except/exception.h"

// laropticks
#include "laropticks/include/AnalysisManagerHelper.h"

namespace laropticks {

  class OpticksHitHandler {
    public:

        static OpticksHitHandler* getInstance(){
            G4AutoLock lock(&mtx);
            if(instance== nullptr){
                instance = new OpticksHitHandler();
            }
            return instance;
        };





        void CollectHits(int eventID,std::map<int, sim::OBTRHelper> obtrHelpers);

        void AddHits();
        void SaveHits();
        void SaveVisibilities();

        void AddPhotons(int value);
        void initSensorCounts(std::map<G4String, G4int>  &DetectorIds);
		void setVoxelID(int &id);
    private:
        OpticksHitHandler(){};
        static OpticksHitHandler * instance;
        static G4Mutex mtx;
        std::vector<sphoton> sphotons;
        std::vector<laropticks::OpticksHit> hits;
		G4TouchableHistory *fTouchableHistory;
		int feventID;
        int PhotonCount=0;
        std::map<int,int> fSensorCounts;
		int fVoxelID=0;
  };

  inline void OpticksHitHandler::AddPhotons(int value)
  {
      PhotonCount+=value;
  }

  inline void OpticksHitHandler::setVoxelID(int &id)
  {
      fVoxelID=id;
  }

   inline void OpticksHitHandler::initSensorCounts(std::map<G4String, G4int>  &DetectorIds)
   {
       for (auto &it : DetectorIds)
            fSensorCounts.insert(std::pair<int,int>(it.second,0)); // Initialize
   }
}

#endif //OPTICKS_OPTICKSHITHANDLER_HH

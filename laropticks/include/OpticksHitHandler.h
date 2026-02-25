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
#include "g4root.hh"
#include "G4Threading.hh"
#include "G4AutoLock.hh"
#include "G4ThreeVector.hh"
#include "G4RunManager.hh"

// Cetlib
#include "cetlib_except/exception.h"


namespace phot{

  class OpticksHitHandler {
    public:

        static OpticksHitHandler* getInstance(){
            G4AutoLock lock(&mtx);
            if(instance== nullptr){
                instance = new OpticksHitHandler();
            }
            return instance;
        };

        struct OpticksHit{
            int hit_id;
            int parent_id;
            int sensor_id;
            double time;
            double x,y,z;
            double momx,momy,momz;
            double polx,poly,polz;
            double wavelength;
            double boundary;
        };



        void CollectHits(int eventID,std::map<int, sim::OBTRHelper> obtrHelpers);

        void AddHits();
        void SaveHits();

		std::vector<sim::OpDetBacktrackerRecord> * GetOpDetBacktrackerRecords(std::map<int,sim::OpDetBacktrackerRecord> *fOpBTMap,int TrackId,double edep);

    private:
        OpticksHitHandler(){};
        static OpticksHitHandler * instance;
        static G4Mutex mtx;
        std::vector<sphoton> sphotons;
        std::vector<OpticksHit> hits;
		G4TouchableHistory *fTouchableHistory;
		int feventID;

  };

}

#endif //OPTICKS_OPTICKSHITHANDLER_HH

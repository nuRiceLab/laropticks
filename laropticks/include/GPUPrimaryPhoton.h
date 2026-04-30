/**
 *  File: include/GPUPrimaryPhoton.h
 *  Author: Ilker Parmaksiz
 *  Experiment: DUNE
 *  Institution: Rice University
 *  Date: 3/18/26
 *  Description: GPUPrimaryPhoton simulation: simulates photons within GPU by reading photons produced by largeant4
 */

#ifndef LAROPTICKS_GPUPRIMARYPHOTON_H
#define LAROPTICKS_GPUPRIMARYPHOTON_H
// CUDA
#include <curand_kernel.h>
// Opticks
#include "sphoton.h"
#include "OpticksPhoton.h"
// LArSoft
#include "nusimdata/SimulationBase/MCParticle.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
//laropticks
#include "laropticks/include/MySensorIdentifier.h"
#include "laropticks/include/OpticksHitHandler.h"
#include "laropticks/include/AnalysisManagerHelper.h"

// G4
#include "G4SystemOfUnits.hh"
namespace laropticks {

    class GPUPrimaryPhoton
    {
        public:
            static GPUPrimaryPhoton* getInstance(){

                if(instance== nullptr){
                    instance = new GPUPrimaryPhoton();
                }
                return instance;
            };

            void reset();
            void setEventID(int &id);
            std::vector<sphoton> GetSPhotons();
            void setPhotons(std::vector<sphoton> sphotons);
            void Batcher();
            void CollectPhotonInfo(std::vector<simb::MCParticle> const* phtlist, bool &fsave_pht);
            void Simulate();
            void setObtrHelpers(std::map<int, sim::OBTRHelper> &obtrHs);
			void setVoxelID(int &id);

        private:
            GPUPrimaryPhoton(){};
            static GPUPrimaryPhoton * instance;
            std::vector<sphoton> photons;
            int eventID;
            std::map<int, sim::OBTRHelper> obtrHelpers;
			int fVoxelID;

    };

    // Set Voxel ID for simulation
    inline void GPUPrimaryPhoton::setVoxelID(int &id){
        fVoxelID = id;
    }
    // Set Event ID for simulation
    inline void GPUPrimaryPhoton::setEventID(int &id){
        eventID = id;
    }
    // Pass Backtracking info
    inline void GPUPrimaryPhoton::setObtrHelpers(std::map<int, sim::OBTRHelper> &obtrHs){
        obtrHelpers = obtrHs;
    }
}


#endif //LAROPTICKS_GPUPRIMARYPHOTON_H
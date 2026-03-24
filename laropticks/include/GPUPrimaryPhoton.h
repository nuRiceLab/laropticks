/**
 *  File: include/OpticksInterface.h
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
#include "storch.h"
#include "storchtype.h"
// LArSoft
#include "nusimdata/SimulationBase/MCParticle.h"

//laropticks
#include "laropticks/include/MySensorIdentifier.h"
#include "laropticks/include/OpticksHitHandler.h"
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
            void setEventID(int id);
            std::vector<sphoton> GetSPhotons();
            void setPhotons(std::vector<sphoton> sphotons);
            void CollectPhotons(std::vector<simb::MCParticle> const* phtlist,unsigned int seed);
            void Simulate();
            void setObtrHelpers(std::map<int, sim::OBTRHelper> &obtrHs);
        private:
            GPUPrimaryPhoton(){};
            static GPUPrimaryPhoton * instance;
            std::vector<sphoton> photons;
            int eventID;
            std::map<int, sim::OBTRHelper> obtrHelpers;
    };

    // Set Event ID for simulation
    inline void GPUPrimaryPhoton::setEventID(int id){
        eventID = id;
    }
    // Pass Backtracking info
    inline void GPUPrimaryPhoton::setObtrHelpers(std::map<int, sim::OBTRHelper> &obtrHs){
        obtrHelpers = obtrHs;
    }
}


#endif //LAROPTICKS_GPUPRIMARYPHOTON_H
/**
 *  File: include/OpticksInterface.h
 *  Author: Ilker Parmaksiz
 *  Experiment: DUNE
 *  Institution: Rice University
 *  Date: 3/18/26
 *  Description: Interface Library connects opticks to LArSoft/larsim
 */
#include "sphoton.h"
//#include "storch.h"
#ifndef LAROPTICKS_GPUPRIMARYPHOTON_H
#define LAROPTICKS_GPUPRIMARYPHOTON_H

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
            std::vector<sphoton> GetSPhotons();
            void CollectPhotons(const sphoton& pht);
        private:
            GPUPrimaryPhoton(){};
            static GPUPrimaryPhoton * instance;
            std::vector<sphoton> photons;
    };
}


#endif //LAROPTICKS_GPUPRIMARYPHOTON_H
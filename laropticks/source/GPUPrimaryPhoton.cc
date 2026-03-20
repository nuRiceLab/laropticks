#include "laropticks/include/GPUPrimaryPhoton.h"

namespace laropticks {
      GPUPrimaryPhoton * GPUPrimaryPhoton::instance = nullptr;

       void GPUPrimaryPhoton::CollectPhotons(const sphoton &pht){
            photons.push_back(pht);
      }

      std::vector<sphoton>GPUPrimaryPhoton::GetSPhotons(){
            return photons;
      }
      void GPUPrimaryPhoton::reset(){
            photons.clear();
            photons.shrink_to_fit();
      }
}
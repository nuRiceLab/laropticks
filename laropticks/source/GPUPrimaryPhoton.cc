#include "laropticks/include/GPUPrimaryPhoton.h"

namespace laropticks {
      GPUPrimaryPhoton * GPUPrimaryPhoton::instance = nullptr;

       void GPUPrimaryPhoton::CollectPhotons(std::vector<simb::MCParticle> const* phtlist,unsigned int seed){
  			 // Prepare the photons
            for (size_t ip=0; ip<phtlist->size(); ip++){
                  sphoton spht;
                  auto pht = phtlist->at(ip);
                  spht.pos=make_float3(pht.Vx(0),pht.Vy(0),pht.Vz(0));
                  spht.mom=make_float3(pht.Px(0),pht.Py(0),pht.Pz(0));
                  spht.mom=make_float3(pht.Polarization()[0],pht.Polarization()[1],pht.Polarization()[2]);
                  spht.wavelength=1240/(pht.E()/1000000); // nm
                  spht.time=pht.T();
           		  spht.ParentId=-1;
                  photons.push_back(spht);
            }
      }

      std::vector<sphoton>GPUPrimaryPhoton::GetSPhotons(){
            return photons;
      }

      void GPUPrimaryPhoton::reset(){
            photons.clear();
            photons.shrink_to_fit();
      }


	void GPUPrimaryPhoton::setPhotons(std::vector<sphoton> sphotons){

		size_t num_floats = sphotons.size()*17;
       	float* data = reinterpret_cast<float*>(sphotons.data());
       	NP* photons = NP::MakeFromValues<float>(data, num_floats);
       	photons->reshape({ static_cast<int64_t>(sphotons.size()), 17});
       	SEvt::SetInputPhoton(photons);
	}
	// initiate simulation
	void GPUPrimaryPhoton::Simulate(){
        //mf::LogTrace("GPUPrimaryPhoton::Simulate") << "Initiation of PrimaryPhoton Simulation Within GPU";
       	G4CXOpticks * g4xc=G4CXOpticks::Get();
       	//Event id needed in here
		OpticksHitHandler * OpticksHits = OpticksHitHandler::getInstance();

		auto sphotons = GetSPhotons();

       	int CollectedPhotons=sphotons.size();
       	int maxPhoton=SEventConfig::MaxPhoton();
       	// Simulate in batch
       	if(CollectedPhotons>=maxPhoton) {
       		std::cout << "Simulating in Batch Mode ...." << std::endl;
       		//Simulate();
       		setPhotons(sphotons);
       	};
		g4xc->simulate(eventID,0);
       	cudaDeviceSynchronize();

		if(SEvt::GetNumHit(0)>0){
       		OpticksHits->CollectHits(eventID,obtrHelpers);
       	}else std::cout << "GPUPrimaryPhoton::Simulate: No Hits" << std::endl;

		//Event id needed here
       	g4xc->reset(eventID);
	}
}
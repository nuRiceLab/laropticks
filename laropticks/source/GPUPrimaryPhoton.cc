#include "laropticks/include/GPUPrimaryPhoton.h"

namespace laropticks {
      GPUPrimaryPhoton * GPUPrimaryPhoton::instance = nullptr;

       void GPUPrimaryPhoton::CollectPhotons(std::vector<simb::MCParticle> const* phtlist,unsigned int seed){

            curandStatePhilox4_32_10 rng;
            curand_init(seed,0,0,&rng);
            // Prepare the photons
            for (size_t ip=0; ip<phtlist->size(); ip++){
                  sphoton spht;
            	  quad6 qs;
				  storch tr;
                  qtorch fqt ;
				  fqt.t = tr;
				  fqt.q = gs;
                  auto pht = fParticleList->at(ip);
                  fqt.q.pos=make_float3(pht.Vx(0),pht.Vy(0),pht.Vz(0));
                  fqt.q.mom=make_float3(pht.Px(0),pht.Py(0),pht.Pz(0));
                  fqt.q.mom=make_float3(pht.Polarization()[0],pht.Polarization()[1],pht.Polarization()[2]);
                  fqt.q.wavelength=1240/(pht.E()/1000000); // nm
                  fqt.q.time=pht.T();
                  fqt.q.type=T_POINT;
                  fqt.q.numphoton=1;
                  storch::generate(photon, rng,fqt.q,pht.TrackId(),-1);
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

	  void GPUPrimaryPhoton::PreparePhotons(){
		auto sphotons = GetSPhotons();

		int CollectedPhotons=GetSPhotons.size();
     	int maxPhoton=SEventConfig::MaxPhoton();
      	// Simulate in batch
        if(CollectedPhotons>=maxPhoton) {
		    std::cout << "Simulating in Batch Mode ...." << std::endl;
			//Simulate();
			SetPhotons(sphotons);
	    };


	}
	void GPUPrimaryPhoton::SetPhotons(std::vector<sphoton> sphotons){

		size_t num_floats = sphotons.size()*17;
       	float* data = reinterpret_cast<float*>(sphotons.data());
       	NP* photons = NP::MakeFromValues<float>(data, num_floats);
       	photons->reshape({ static_cast<int64_t>(sphotons.size()), 17});
       	SEvt::SetPhotons(photons);
	}
	// initiate simulation
	void GPUPrimaryPhoton::Simulate(){
        //mf::LogTrace("GPUPrimaryPhoton::Simulate") << "Initiation of PrimaryPhoton Simulation Within GPU";
       	G4CXOpticks * g4xc=G4CXOpticks::Get();
       	//Event id needed in here
		OpticksHitHandler * OpticksHits = OpticksHitHandler::getInstance();

		g4xc->simulate(eventID,0);
       	cudaDeviceSynchronize();

		if(SEvt::GetNumHit(0)>0){
       		OpticksHits->CollectHits(eventID,obtrHelpers);
       	}else std::cout << "GPUPrimaryPhoton::Simulate: No Hits" << std::endl;
       	//Event id needed here
       	g4xc->reset(eventID);
	}
}
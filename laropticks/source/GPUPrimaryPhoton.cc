#include "laropticks/include/GPUPrimaryPhoton.h"

namespace laropticks
{
      GPUPrimaryPhoton * GPUPrimaryPhoton::instance = nullptr;

       void GPUPrimaryPhoton::CollectPhotonInfo(std::vector<simb::MCParticle> const* phtlist){
  			 // Prepare the photons
            for (size_t ip=0; ip<phtlist->size(); ip++)
			{
                  sphoton spht;
                  auto pht = phtlist->at(ip);
                  spht.pos=make_float3(pht.Vx(0),pht.Vy(0),pht.Vz(0));
                  spht.mom=make_float3(pht.Px(0),pht.Py(0),pht.Pz(0));
                  spht.mom=make_float3(pht.Polarization()[0],pht.Polarization()[1],pht.Polarization()[2]);
                  spht.wavelength=1240/(pht.E()/1000000); // nm
				  std::cout << "Wavelength " << spht.wavelength << std::endl;
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


	void GPUPrimaryPhoton::setPhotons(std::vector<sphoton> sphotons)
	{
		std::cout << " [GPUPrimaryPhoton::setPhotons] Setting Photons ...." << std::endl;
		size_t num_floats = sphotons.size()*17;
       	float* data = reinterpret_cast<float*>(sphotons.data());
       	NP* photons = NP::MakeFromValues<float>(data, num_floats);
       	photons->reshape({ static_cast<int64_t>(sphotons.size()), 17});
       	SEvt::SetInputPhoton(photons);
	}
	void GPUPrimaryPhoton::Batcher()
	{
       	std::cout << " [GPUPrimaryPhoton::Batcher] Deciding if Batching Needed ...." << std::endl;

       	auto sphotons = GetSPhotons();

       	long unsigned int CollectedPhotons=sphotons.size();
       	long unsigned int maxPhoton=SEventConfig::MaxPhoton();
       	// Simulate in batch
       	if(CollectedPhotons>=maxPhoton)
		{
       		std::cout << "Simulating in Batch Mode ...." << std::endl;
       		//Simulate();
       		for (std::size_t i =0 ; i < CollectedPhotons; i+=maxPhoton)
			{
				  std::size_t end =std::min(i+maxPhoton,sphotons.size());
       			std::vector<sphoton> batch(
	   			std::make_move_iterator(sphotons.begin() + i),
	   			std::make_move_iterator(sphotons.begin() + end));
				setPhotons(batch);
				Simulate();
			}
		}else
		{
			std::cout << "Simulating Photons at Once ...." << std::endl;
			setPhotons(sphotons);
       		Simulate();
		}
	}

	// initiate simulation
	void GPUPrimaryPhoton::Simulate()
	{

        //mf::LogTrace("GPUPrimaryPhoton::Simulate") << "Initiation of PrimaryPhoton Simulation Within GPU";
       	G4CXOpticks * g4xc=G4CXOpticks::Get();
       	//Event id needed in here
		OpticksHitHandler * OpticksHits = OpticksHitHandler::getInstance();
       	std::cout << " [GPUPrimaryPhoton::Simulate] Simulating Photons Within GPU for EventID " << eventID << " ...."  << std::endl;
		g4xc->simulate(eventID,0);
       	cudaDeviceSynchronize();

		if(SEvt::GetNumHit(0)>0)
		{
       		OpticksHits->CollectHits(eventID,obtrHelpers);
       	}else std::cout << "GPUPrimaryPhoton::Simulate: No Hits" << std::endl;

		//Event id needed here
       	g4xc->reset(eventID);
	}

}
#include "laropticks/include/GPUPrimaryPhoton.h"

namespace laropticks
{
      GPUPrimaryPhoton * GPUPrimaryPhoton::instance = nullptr;

       void GPUPrimaryPhoton::CollectPhotonInfo(std::vector<simb::MCParticle> const* phtlist,bool &fsave_pht){
  			 // Prepare the photons
			double energy=0,wavelength=0;
			G4ThreeVector pol,mom,momunit;
			G4LorentzVector pos;
            for (size_t ip=0; ip<phtlist->size(); ip++)
			{
                  sphoton spht;
				  spht.zero();
			      spht.zero_flags();
    			  spht.set_flag(TORCH);
                  auto pht = phtlist->at(ip);
				  pos=G4LorentzVector (pht.Position(0).X(), pht.Position(0).Y(), pht.Position(0).Z(), pht.Position(0).T());
				  mom=G4ThreeVector(pht.Momentum(0).X(),pht.Momentum(0).Y(),pht.Momentum(0).Z());
				  pol=G4ThreeVector(pht.Polarization().X(),pht.Polarization().Y(),pht.Polarization().Z());
				  momunit=mom.unit();

                  spht.pos=make_float3(pos.x()*10,pos.y()*10,pos.z()*10); //cm --> mm
                  spht.mom=make_float3(momunit.x(),momunit.y(),momunit.z());
                  spht.pol=make_float3(pol.x(),pol.y(),pol.z());
                  energy=1e9*pht.E(); // eV
            	  wavelength=1240/energy;
				  spht.wavelength=wavelength; // nm
            	  spht.time=pos.t();
				  //std::cout << "position " << pht.Vx(0) << " " <<pht.Vy(0)<< " " <<pht.Vz(0) << "Time " << spht.time << " Wavelength " <<spht.wavelength << std::endl;
				  //std::cout << "mom " << mom.X() << " " <<mom.Y()<< " " <<mom.Z()<< std::endl;
				  //std::cout << "pol " << pht.Polarization()[0] << " " << pht.Polarization()[1]<< " " <<pht.Polarization()[2]<< std::endl;

           		  spht.ParentId=0;
				  if(fsave_pht){
				  		AnalysisManagerHelper * AnaMngr = AnalysisManagerHelper::getInstance();
						AnaMngr->FillPhotonGenTree(eventID,pos,momunit,pol ,wavelength,energy);
				  }

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
       		std::cout << "[GPUPrimaryPhoton::Batcher] Simulating in Batch Mode ...." << std::endl;

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
			std::cout << "[GPUPrimaryPhoton::Batcher] Simulating Photons at Once ...." << std::endl;
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
       	std::cout << " [GPUPrimaryPhoton::Simulate]: Simulating Photons Within GPU for EventID " << eventID << " ...."  << std::endl;
		std::cout << " [GPUPrimaryPhoton::Simulate]: Photons Collected = " << GetSPhotons().size() <<std::endl;
		g4xc->simulate(eventID,0);
       	cudaDeviceSynchronize();


		if(SEvt::GetNumHit(0)>0){
			OpticksHits->setVoxelID(fVoxelID);
			OpticksHits->AddPhotons(GetSPhotons().size()); // Get Photon Count;
			OpticksHits->CollectHits(eventID,obtrHelpers);

		}
       	else std::cout << "[GPUPrimaryPhoton::Simulate]: No Hits" << std::endl;

		//Event id needed here
       	g4xc->reset(eventID);
	}

}
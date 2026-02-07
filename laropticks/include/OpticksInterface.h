/**
 *  File: include/OpticksInterface.h
 *  Author: Ilker Parmaksiz
 *  Experiment: DUNE
 *  Institution: Rice University
 *  Date: 1/9/26
 *  Description: Interface Library connects opticks to LArSoft/larsim
 */
#pragma once

#ifndef OPTICKSINTERFACE_H
#define OPTICKSINTERFACE_H


#include "lardataobj/Simulation/SimEnergyDeposit.h"
#include "fhiclcpp/ParameterSet.h"

#include "laropticks/include/MySensorIdentifier.h"
#include "laropticks/include/OpticksHitHandler.h"
#include "larsim/PhotonPropagation/OpticalPropagationTools/IOpticalPropagation.h"
#include "nusimdata/SimulationBase/MCParticle.h"


#include "G4Track.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4TouchableHistory.hh"

namespace fhicl {
    class ParameterSet;
}
namespace phot {

class MySensorIdentifier;
class OpticksHitHandler;

    class OpticksInterface : public IOpticalPropagation  {

    public:

      // Construct with fcl parameters
      OpticksInterface(fhicl::ParameterSet const& config);

      // Default destructor
      ~OpticksInterface();

      void init();
      void initPhotonDetectors();
      void initFileManager();
      void CollectPhotons(G4Track *track,sim::SimEnergyDeposit edep);
      void GetHitsFromGPU();
      void Simulate();

	  std::string getBuildDir();
	  // Initialize fast simulation
	  void beginJob() override;

	  // Execute simulation on a single art::Event
	  UPVecBTR executeEvent(VecSED const& edeps) override;

	  void SetParticleList(std::vector<simb::MCParticle> const* plist) override;
	  //simb::MCParticle * FindParticle(int TrackID);
	  // Finalize execution
	  void endJob() override;
      void InitializeTools(CLHEP::HepRandomEngine& poisson, CLHEP::HepRandomEngine& scint_time) override;

	  private:
      std::string GDMLPath;
      MySensorIdentifier * OpticksSensorIdentifier;
      OpticksHitHandler* OpticksHits;
      std::map<G4String, G4int>  DetectorIds;
	  std::vector<simb::MCParticle> const * fParticleList ;
      std::unordered_map<int, const simb::MCParticle> fParticleMap;
	  std::vector<G4Step*> fsteps;
	  std::vector<G4StepPoint*> fstepPoints ;
	  std::vector<G4Track*> ftracks;
	  std::vector<G4TouchableHistory*> fTouchableHistories;
	  G4VPhysicalVolume* World;
  };
}

#endif //OPTICKSINTERFACE_H

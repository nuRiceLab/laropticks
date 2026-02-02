/**
 *  File: include/Opticks_tool.h
 *  Author: Ilker Parmaksiz
 *  Experiment: DUNE
 *  Institution: Rice University
 *  Date: 1/9/26
 *  Description: Photon Propagation on GPU by Opticks Library
 */
#pragma once

#ifndef OPTICKSINTERFACE_H
#define OPTICKSINTERFACE_H


#include "lardataobj/Simulation/SimEnergyDeposit.h"
#include "fhiclcpp/ParameterSet.h"

#include "laropticks/include/MySensorIdentifier.h"
#include "laropticks/include/OpticksHitHandler.h"
#include "larsim/PhotonPropagation/OpticalPropagationTools/IOpticalPropagation.h"




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
      void CollectPhotons();
      void GetHitsFromGPU();
      void Simulate();
      std::map<G4String, G4int> GetPhotonDetectors();

	  // Initialize fast simulation
	  void beginJob() override;

	  // Execute simulation on a single art::Event
	  UPVecBTR executeEvent(VecSED const& edeps) override;

	  // Finalize execution
	  void endJob() override;
          void InitializeTools(CLHEP::HepRandomEngine& poisson, CLHEP::HepRandomEngine& scint_time) override;		
	  private:
      std::string GDMLPath;
      MySensorIdentifier * OpticksSensorIdentifier;
      OpticksHitHandler* OpticksHits;
      std::map<G4String, G4int>  DetectorIds;
  };
}

#endif //OPTICKSINTERFACE_H

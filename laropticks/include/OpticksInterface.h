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



// laropticks headers
#include "laropticks/include/MySensorIdentifier.h"
#include "laropticks/include/OpticksHitHandler.h"
#include "laropticks/include/AnalysisManagerHelper.h"

// LArSoft Headers
#include "larsim/PhotonPropagation/OpticalPropagationTools/IOpticalPropagation.h"
#include "larcore/CoreUtils/ServiceUtil.h"
#include "larcore/Geometry/Geometry.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "lardataobj/Simulation/SimEnergyDeposit.h"
#include "fhiclcpp/ParameterSet.h"
#include "larcorealg/Geometry/BoxBoundedGeo.h"
#include "larcorealg/Geometry/OpDetGeo.h"
#include "larcoreobj/SimpleTypesAndConstants/geo_vectors.h"
#include "lardataobj/Simulation/OpDetBacktrackerRecord.h"
#include "lardataobj/Simulation/OpDetBacktrackerRecord.h"
#include "lardataobj/Simulation/SimEnergyDeposit.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

// Opticks Headers
#include "G4CXOpticks.hh"
#include "U4SensorIdentifier.h"
#include "SEvt.hh"
#include "U4.hh"
#include "SEventConfig.hh"
#include "OPTICKS_LOG.hh"
#include "fhiclcpp/ParameterSet.h"

// Geant4 Headers
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4VPhysicalVolume.hh"
#include "G4Material.hh"
#include "g4root.hh"
#include "G4TransportationManager.hh"
#include "G4DynamicParticle.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "G4TrackStatus.hh"
#include "G4TransportationManager.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4MaterialPropertiesIndex.hh"
#include "G4TouchableHandle.hh"
#include "G4Exception.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4TouchableHistory.hh"

// Xercesc
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/dom/DOM.hpp"

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


	  // Initialize fast simulation
	  void beginJob() override;

	  // Execute simulation on a single art::Event
	  UPVecBTR executeEvent(VecSED const& edeps) override;
	  void initTracks();
	  void SetParticleList(std::vector<simb::MCParticle> const* plist) override;
	  //simb::MCParticle * FindParticle(int TrackID);
	  // Finalize execution
	  void endJob() override;
      void InitializeTools(CLHEP::HepRandomEngine& poisson, CLHEP::HepRandomEngine& scint_time) override;
      template <typename T>
      void ReleaseMemory(std::vector<T*> &vec, const std::string& msg) {
			 std::cout << "ReleasingMemory for " <<msg << std::endl;
             for (T* ptr : vec) delete ptr;
			 vec.clear();
			 vec.shrink_to_fit();
	  }

	  private:
      std::string GDMLPath;
      MySensorIdentifier * OpticksSensorIdentifier;
      OpticksHitHandler* OpticksHits;
      std::map<G4String, G4int>  DetectorIds;
	  std::vector<simb::MCParticle> const * fParticleList ;
      std::unordered_map<int, const simb::MCParticle> fParticleMap;
	  //std::vector<G4Step*> fsteps;
	  std::vector<G4StepPoint*> fstepPoints ;
	  std::vector<G4Track*> ftracks;
	  std::vector<G4TouchableHistory*> fTouchableHistories;
	  std::vector<G4DynamicParticle*> fDynamicParticles;
	  G4VPhysicalVolume* World;
	  //std::map<int,sim::OpDetBacktrackerRecord> *fOpDetBacktrackerMap;
	  int trackID;
	  geo::GeometryCore const& fGeom;
	  std::map<int,G4Track*> *Trackmps;
	  std::map<int, sim::OBTRHelper> obtrHelpers;
	  int eventID;
  };
}

#endif //OPTICKSINTERFACE_H

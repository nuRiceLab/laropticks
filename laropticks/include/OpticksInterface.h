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
#include "laropticks/include/GPUPrimaryPhoton.h"

// LArSoft Headers
#include "larcore/CoreUtils/ServiceUtil.h"
#include "larcore/Geometry/Geometry.h"
#include "nusimdata/SimulationBase/MCParticle.h"
#include "lardataobj/Simulation/SimEnergyDeposit.h"
#include "larcorealg/Geometry/BoxBoundedGeo.h"
#include "larcorealg/Geometry/OpDetGeo.h"
#include "larcoreobj/SimpleTypesAndConstants/geo_vectors.h"
#include "lardataobj/Simulation/OpDetBacktrackerRecord.h"
#include "lardataobj/Simulation/OpDetBacktrackerRecord.h"
#include "lardataobj/Simulation/SimEnergyDeposit.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/ParameterSet.h"

// Opticks Headers
#include "G4CXOpticks.hh"
#include "U4SensorIdentifier.h"
#include "SEvt.hh"
#include "U4.hh"
#include "SEventConfig.hh"
#include "OPTICKS_LOG.hh"



// Geant4 Headers
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
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
#include "G4PhysicalVolumeStore.hh"
// Xercesc
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include "xercesc/dom/DOM.hpp"


namespace laropticks {

class MySensorIdentifier;
class OpticksHitHandler;

    class OpticksInterface  {

    public:
      using VecSED = std::vector<sim::SimEnergyDeposit>;
      using UPVecBTR = std::unique_ptr<std::vector<sim::OpDetBacktrackerRecord>>;

      static OpticksInterface* GetInstance(){
    	 if(instance== nullptr){
    		instance = new OpticksInterface();
    	 }
    	 return instance;
      }




      void init();
      void initPhotonDetectors();
      void initFileManager();
      void CollectPhotons(G4Track *track,sim::SimEnergyDeposit edep);
      void GetHitsFromGPU();
      void Simulate();
	  void createG4SkinSurface(std::string VolName, G4OpticalSurface* surface);
	  void createG4BorderSurface(G4VPhysicalVolume *phyv1, std::string v2, G4OpticalSurface* surface);
	  std::string GetVolumeName(const std::string& s);
      UPVecBTR executeEvent(VecSED const& edeps); // Simulating photons produced by scintilation events
      UPVecBTR executeEvent(); // This is for producing visibilities with primary photons


	  // Initialize fast simulation
	  void beginJob() ;

	  void initTracks();
	  void setEventID(int evt);
	  void setWorld(G4VPhysicalVolume* fWorld);
	  void setParticleList(std::vector<simb::MCParticle> const* plist);
	  //simb::MCParticle * FindParticle(int TrackID);
	  // Finalize execution
	  void endJob();
      template <typename T>
      void ReleaseMemory(std::vector<T*> &vec, const std::string& msg) {
			 std::cout << "ReleasingMemory for " <<msg << std::endl;
             for (T* ptr : vec) delete ptr;
			 vec.clear();
			 vec.shrink_to_fit();
	  }

	  private:
    	// Construct with fcl parameters
      OpticksInterface(){};
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
	  int trackID;
	  int eventID;
	  geo::GeometryCore const* fGeom;
	  std::map<int,G4Track*> *Trackmps;
	  std::map<int, sim::OBTRHelper> obtrHelpers;
	  G4PhysicalVolumeStore* phyStore;
	  G4LogicalVolumeStore* lvStore;
      static OpticksInterface* instance;
	  GPUPrimaryPhoton * PhotonGen;
  };

}

#endif //OPTICKSINTERFACE_H

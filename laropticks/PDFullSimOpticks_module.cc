/**
 *  Class: PDFullSimOpticks
 *  Plugin Type: producer
 *  File: laropticks/PDFullSimOpticks_module.cc
 *  Author: Ilker Parmaksiz
 *  Experiment: DUNE
 *  Institution: Rice University
 *  Date: 3/2/2026
 *  Description: Module interface LArSoft with Opticks. Using IonAndScint Module, optical photon information is passed to opticks library.
 *  The full photon propagation is simulated at GPU.
 *  Input: 'sim::SimEnergyDeposit' Output: 'sim::OpDetBacktrackerRecord'
**/

// LArSoft libraries
#include "larcore/CoreUtils/ServiceUtil.h"
#include "larcore/Geometry/Geometry.h"
#include "larcorealg/Geometry/BoxBoundedGeo.h"
#include "larcorealg/Geometry/OpDetGeo.h"
#include "larcoreobj/SimpleTypesAndConstants/geo_vectors.h"
#include "lardataobj/Simulation/OpDetBacktrackerRecord.h"
#include "lardataobj/Simulation/SimEnergyDeposit.h"
#include "lardataobj/Simulation/SimPhotons.h"
#include "larsim/IonizationScintillation/ISTPC.h"
#include "nurandom/RandomUtils/NuRandomService.h"

// Art libraries
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/make_tool.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Comment.h"
#include "fhiclcpp/types/DelegatedParameter.h"
#include "fhiclcpp/types/Name.h"
#include "fhiclcpp/types/OptionalDelegatedParameter.h"
#include "fhiclcpp/types/Sequence.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

// Random numbers
#include "CLHEP/Random/RandPoissonQ.h"

#include "range/v3/view/enumerate.hpp"

#include <cmath>
#include <ctime>
#include <map>
#include <memory>
#include <vector>

// laropticks
#include "laropticks/include/OpticksInterface.h"


namespace laropticks {
  class PDFullSimOpticks : public art::EDProducer {
  public:
      using VecSED = std::vector<sim::SimEnergyDeposit>;
      using UPVecBTR = std::unique_ptr<std::vector<sim::OpDetBacktrackerRecord>>;
    // Define the fhicl configuration
    struct Config {
        using Name = fhicl::Name;
        using Comment = fhicl::Comment;
        using DP = fhicl::DelegatedParameter;
        using ODP = fhicl::OptionalDelegatedParameter;
        const std::vector<int> default_TPCs{};
        fhicl::Atom<art::InputTag> SimulationLabel{Name("SimulationLabel"),
                                                   Comment("SimEnergyDeposit label.")};
    };


    using Parameters = art::EDProducer::Table<Config>;

    explicit PDFullSimOpticks(Parameters const& config);
    void produce(art::Event&) override;
    void beginJob() override;
    void endJob() override;
    UPVecBTR executeEvent(int EventID , VecSED const& edeps,std::vector<simb::MCParticle> const * plist);

  private:
	const art::InputTag fSimTag;
    OpticksInterface* opticks;
  };

  //--------------------Construct PDFullSimOpticks-----------------------------//
  /*!
   * Construct with fhicl parameters if there is any.
   */
  PDFullSimOpticks::PDFullSimOpticks(Parameters const& config) : art::EDProducer{config}
											   ,fSimTag(config().SimulationLabel())
											   ,opticks(nullptr)
{

    mf::LogInfo("PDFullSimOpticks") << "Initializing PDFullSimOpticks." << std::endl;


    // Initialize OpDetBacktrackerRecord
    produces<std::vector<sim::OpDetBacktrackerRecord>>();

    // Initialize Opticks
    opticks=OpticksInterface::GetInstance();
  }


  //......................................................................
  void PDFullSimOpticks::produce(art::Event& event)
  {
     mf::LogTrace("PDFullSimOpticks") << "PDFullSimOpticks Module Producer"
                              << "EventID: " << event.event();

    art::Handle<std::vector<sim::SimEnergyDeposit>> edepHandle;
    if (!event.getByLabel(fSimTag, edepHandle)) {
      mf::LogError("PDFullSimOpticks") << "PDFullSimOpticks Module Cannot getByLabel: " << fSimTag;
      return;
    }

    auto mcHandle = event.getValidHandle<std::vector<simb::MCParticle>>("largeant");



    // Include energy deposits here
    auto result=opticks->executeEvent(event.event(),*(edepHandle.product()),mcHandle);

    event.put(std::move(result));

  }

} // namespace laropticks

//--------------------Begin Job----------------------------------------------//
void laropticks::PDFullSimOpticks::beginJob()
{
  mf::LogTrace("PDFullSimOpticks") << "beginJob" << std::endl;
  opticks->beginJob();
}

//--------------------End Job---------------------------------------------//
void laropticks::PDFullSimOpticks::endJob()
{
  mf::LogTrace("PDFullSimOpticks") << "endJob" << std::endl;
  opticks->endJob();
}


DEFINE_ART_MODULE(laropticks::PDFullSimOpticks)
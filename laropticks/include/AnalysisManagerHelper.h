/**
 *  File: include/AnalysisManagerHelper.h
 *  Author: Ilker Parmaksiz
 *  Experiment: DUNE
 *  Institution: Rice University
 *  Date: 3/18/26
 *  Description: AnalysisManagerHelper: Helps with debugging and validation of PDFastSimOpticks
 */
// LArSoft
#include "messagefacility/MessageLogger/MessageLogger.h"

// Art
#include "art_root_io/TFileService.h"
#include "art_root_io/TFileDirectory.h"

// Geant4
#include "G4Threading.hh"
#include "G4AutoLock.hh"
#include "G4ThreeVector.hh"
#include "G4LorentzVector.hh"
#include "g4root.hh"
#include "sphoton.h"

// ROOT
#include "TTree.h"
// laropticks
#include "laropticks/include/types.h"
#ifndef GDMLOPTICKS_ANALYSISMANAGERHELPER_HH
#define GDMLOPTICKS_ANALYSISMANAGERHELPER_HH

namespace laropticks {

class AnalysisManagerHelper
  {

    public:

        //
        static AnalysisManagerHelper* getInstance()
        {
            G4AutoLock lock(&mtx);
            if(instance== nullptr)
            {
                instance = new AnalysisManagerHelper();
            }
            return instance;
        }

        static void deleteInstance()
        {
            G4AutoLock lock(&mtx);
            if(instance != nullptr) {
                delete instance;
                instance = nullptr;
            }
        }

        G4int GetG4ScintPhotons();
        G4int GetOpticksScintPhotons();
        G4int GetG4CerenkovPhotons();
        G4int GetOpticksCerenkovPhotons();
        G4int GetDuration();
        std::map<G4String,G4int> * GetDetectIds();

        void AddG4ScintPhotons(G4int ph);
        void AddOpticksScintPhotons(G4int ph);
        void AddG4CerenkovPhotons(G4int ph);
        void AddOpticksCerenkovPhotons(G4int ph);
        void SetDuration(G4double dr);
        void SavePhotonInfotoFile();
        void SaveG4HitsToFile();
        void SetDetectIds(std::map<G4String,G4int> * fIDs);
        void SaveVoxelPhotonInfotoFile(int &evtid, sphoton &sp, float &energy);
        void Reset();

        // TFile Related
        void initVoxelTree();
        void initOpticksHitTree();
        void initPhotonGenTree();
        void setFileService(art::TFileService * fs);


        void FillHitTree(laropticks::OpticksHit &hit);
        void FillVoxelTree(laropticks::Visibility &vis);
        void FillPhotonGenTree(int &evtid, G4LorentzVector &pos,G4ThreeVector &mom,G4ThreeVector &pol,double wavelength, double &energy);
        ~AnalysisManagerHelper();
    private:
        AnalysisManagerHelper(){};

        static G4Mutex mtx;
        static AnalysisManagerHelper* instance;
        G4int G4CerenkovPhotons{0};
        G4int OpticksCerenkovPhotons{0};
        G4int G4ScintPhotons{0};
        G4int OpticksScintPhotons{0};
        G4double Duration{0};
        std::map<G4String,G4int> * fDetectIds{nullptr};

        TTree * fVisTTree;
        TTree * fPhotonGenTTree;
        TTree * fOpticksHitTTree;
        laropticks::Visibility fVisibilityBranch;
        laropticks::OpticksHit fOpticksHitBranch;
        laropticks::PhotonGen fPhotonGenBranch;
        art::TFileService *fTFileService;

  };
    inline void AnalysisManagerHelper::setFileService(art::TFileService * fs){
        fTFileService=fs;
    }

}
#endif //GDMLOPTICKS_ANALYSISMANAGERHELPER_HH
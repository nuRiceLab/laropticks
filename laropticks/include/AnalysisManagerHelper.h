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
#include "G4Version.hh"
#if G4VERSION_NUMBER < 1100
#include "g4root.hh"
#else
#include "G4AnalysisManager.hh"
#endif
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

        G4int GetNumPhotons();
        G4double GetDuration();
        std::map<G4String,G4int> * GetDetectIds();

        void AddNumPhotons(G4int ph);
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
        void initPerformanceTimeTree();
        void initIonAndScintGenTree();
        void setFileService(art::TFileService * fs);

        TTree * getOpticksHitTree();
        void FillHitTree(OpticksHit &hit);
        void FillVoxelTree(Visibility &vis);
        void FillPerformanceTree(PerformanceTime *per);
        void FillPhotonGenTree(int &evtid, G4LorentzVector &pos,G4ThreeVector &mom,G4ThreeVector &pol,double wavelength, double &energy);
        void FillEdepTree(int &evtid, G4LorentzVector &pos, int trkid, int pdg, int nphot, int nelect);
        ~AnalysisManagerHelper();
    private:
        AnalysisManagerHelper()
        {
             TTree * fVisTTree=nullptr;
             TTree * fPhotonGenTTree=nullptr;
             TTree * fOpticksHitTTree=nullptr;
             TTree * fSimEdepGenTTree=nullptr;
             TTree * fPerformanceTimeTTree=nullptr;
        };

        static G4Mutex mtx;
        static AnalysisManagerHelper* instance;
        G4int fAmountPhotons{0};
        G4double Duration{0};
        std::map<G4String,G4int> * fDetectIds{nullptr};

        TTree * fVisTTree;
        TTree * fPhotonGenTTree;
        TTree * fOpticksHitTTree;
        TTree * fSimEdepGenTTree;
        TTree * fPerformanceTimeTTree;

        Visibility fVisibilityBranch;
        OpticksHit fOpticksHitBranch;
        PhotonGen fPhotonGenBranch;
        SimEdeps fSimEdepsGenBranch;
        PerformanceTime fPerformanceTimeBranch;

        art::TFileService *fTFileService;
  };

    inline void AnalysisManagerHelper::setFileService(art::TFileService * fs){
        fTFileService=fs;
    }
     inline TTree * AnalysisManagerHelper::getOpticksHitTree(){
        return fOpticksHitTTree;
    }


}
#endif //GDMLOPTICKS_ANALYSISMANAGERHELPER_HH
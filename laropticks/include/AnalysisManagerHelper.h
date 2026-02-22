//
// Created by ilker on 11/5/25.
//


#include "G4Threading.hh"
#include "G4AutoLock.hh"
#include "G4ThreeVector.hh"
#include "g4root.hh"

#ifndef GDMLOPTICKS_ANALYSISMANAGERHELPER_HH
#define GDMLOPTICKS_ANALYSISMANAGERHELPER_HH


class AnalysisManagerHelper
{
    public:
         static AnalysisManagerHelper* getInstance(){
                    G4AutoLock lock(&mtx);
                    if(instance== nullptr){
                        instance = new AnalysisManagerHelper();
                    }
                    return instance;
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

        void Reset();


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


};

#endif //GDMLOPTICKS_ANALYSISMANAGERHELPER_HH
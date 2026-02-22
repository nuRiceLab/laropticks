//
// Created by ilker on 11/5/25.
//
#include "laropticks/include/AnalysisManagerHelper.h"

// Initialize Static Member
AnalysisManagerHelper * AnalysisManagerHelper::instance = nullptr;
G4Mutex AnalysisManagerHelper::mtx;

G4int AnalysisManagerHelper::GetG4ScintPhotons(){
    return G4ScintPhotons;
}


G4int AnalysisManagerHelper::GetOpticksScintPhotons(){
    return OpticksScintPhotons;
}

G4int AnalysisManagerHelper::GetG4CerenkovPhotons(){
    return G4CerenkovPhotons;
}


G4int AnalysisManagerHelper::GetOpticksCerenkovPhotons(){
    return OpticksCerenkovPhotons;
}

G4int AnalysisManagerHelper::GetDuration(){
    return Duration;
}

void AnalysisManagerHelper::AddG4ScintPhotons(G4int ph){
    G4ScintPhotons+=ph;
}
void AnalysisManagerHelper::AddG4CerenkovPhotons(G4int ph){
    G4CerenkovPhotons+=ph;
}

void AnalysisManagerHelper::AddOpticksScintPhotons(G4int ph){
    OpticksScintPhotons+=ph;
}

void AnalysisManagerHelper::AddOpticksCerenkovPhotons(G4int ph){
    OpticksCerenkovPhotons+=ph;
}

void AnalysisManagerHelper::SetDuration(G4double dr){
    Duration=dr;
}

void AnalysisManagerHelper::Reset()
{
    Duration=0;
    G4CerenkovPhotons=0;
    OpticksScintPhotons=0;
    OpticksCerenkovPhotons=0;
    OpticksScintPhotons=0;
    G4CerenkovPhotons=0;
    G4ScintPhotons=0;
}
void AnalysisManagerHelper::SavePhotonInfotoFile()
{
    G4AnalysisManager * AnaMngr = G4AnalysisManager::Instance();
    G4int eventID=0;
    AnaMngr->FillNtupleIColumn(3,0,G4ScintPhotons);
    AnaMngr->FillNtupleIColumn(3,1,G4CerenkovPhotons);
    AnaMngr->FillNtupleIColumn(3,2,OpticksScintPhotons);
    AnaMngr->FillNtupleIColumn(3,3,OpticksCerenkovPhotons);
    AnaMngr->FillNtupleDColumn(3,4,Duration);
    AnaMngr->FillNtupleIColumn(3,5,eventID);
    AnaMngr->AddNtupleRow(3);
}
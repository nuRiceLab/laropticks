//
// Created by ilker on 11/5/25.
//
#include "laropticks/include/AnalysisManagerHelper.h"
namespace laropticks {
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
        AnaMngr->FillNtupleIColumn(2,0,G4ScintPhotons);
        AnaMngr->FillNtupleIColumn(2,1,G4CerenkovPhotons);
        AnaMngr->FillNtupleIColumn(2,2,OpticksScintPhotons);
        AnaMngr->FillNtupleIColumn(2,3,OpticksCerenkovPhotons);
        AnaMngr->FillNtupleDColumn(2,4,Duration);
        AnaMngr->FillNtupleIColumn(2,5,eventID);
        AnaMngr->AddNtupleRow(3);
    }

    void AnalysisManagerHelper::SaveVoxelPhotonInfotoFile(int &evtid, sphoton &sp, float &energy)
    {
        G4AnalysisManager * AnaMngr = G4AnalysisManager::Instance();
        int id=1;
        // Float precision for Opticks
        AnaMngr->FillNtupleIColumn(id,0,evtid);
        AnaMngr->FillNtupleFColumn(id,1,sp.pos.x);
        AnaMngr->FillNtupleFColumn(id,2,sp.pos.y);
        AnaMngr->FillNtupleFColumn(id,3,sp.pos.z);
        AnaMngr->FillNtupleFColumn(id,4,sp.time);
        AnaMngr->FillNtupleFColumn(id,5,sp.pol.x);
        AnaMngr->FillNtupleFColumn(id,6,sp.pol.y);
        AnaMngr->FillNtupleFColumn(id,7,sp.pol.z);
        AnaMngr->FillNtupleFColumn(id,8,sp.mom.x);
        AnaMngr->FillNtupleFColumn(id,9,sp.mom.y);
        AnaMngr->FillNtupleFColumn(id,10,sp.mom.z);
        AnaMngr->FillNtupleFColumn(id,11,sp.wavelength);
        AnaMngr->FillNtupleFColumn(id,12,energy);
        AnaMngr->AddNtupleRow(id);
    }
    void AnalysisManagerHelper::SaveVoxelPhotonInfotoFile(int &evtid, G4LorentzVector &pos,G4ThreeVector &mom,G4ThreeVector &pol,double wavelength, double &energy)
    {
        G4AnalysisManager * AnaMngr = G4AnalysisManager::Instance();
        int id=1;

        AnaMngr->FillNtupleIColumn(id,0,evtid);
        AnaMngr->FillNtupleDColumn(id,1,pos.x());
        AnaMngr->FillNtupleDColumn(id,2,pos.y());
        AnaMngr->FillNtupleDColumn(id,3,pos.z());
        AnaMngr->FillNtupleDColumn(id,4,pos.t());
        AnaMngr->FillNtupleDColumn(id,5,pol.x());
        AnaMngr->FillNtupleDColumn(id,6,pol.y());
        AnaMngr->FillNtupleDColumn(id,7,pol.z());
        AnaMngr->FillNtupleDColumn(id,8,mom.x());
        AnaMngr->FillNtupleDColumn(id,9,mom.y());
        AnaMngr->FillNtupleDColumn(id,10,mom.z());
        AnaMngr->FillNtupleDColumn(id,11,wavelength);
        AnaMngr->FillNtupleDColumn(id,12,energy);
        AnaMngr->AddNtupleRow(id);
    }
}
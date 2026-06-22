
#include "laropticks/include/AnalysisManagerHelper.h"

#include "laropticks/include/types.h"

namespace laropticks {

    // Initialize Static Member
    AnalysisManagerHelper * AnalysisManagerHelper::instance = nullptr;
    G4Mutex AnalysisManagerHelper::mtx;


     AnalysisManagerHelper::~AnalysisManagerHelper()
    {
         if(fDetectIds != nullptr) {
             delete fDetectIds;
             fDetectIds = nullptr;
        }
    }

    G4int AnalysisManagerHelper::GetNumPhotons(){
        return fAmountPhotons;
    }


    G4double AnalysisManagerHelper::GetDuration(){
        return Duration;
    }

    void AnalysisManagerHelper::AddNumPhotons(G4int ph){
        fAmountPhotons+=ph;
    }

    void AnalysisManagerHelper::SetDuration(G4double dr){
        Duration=dr;
    }

    void AnalysisManagerHelper::Reset()
    {
        Duration=0;
        fAmountPhotons=0;
    }

    void AnalysisManagerHelper::SavePhotonInfotoFile()
    {
        G4AnalysisManager * AnaMngr = G4AnalysisManager::Instance();
        int id=2,fEventID=0;
        AnaMngr->FillNtupleIColumn(id,0,fEventID);
        AnaMngr->FillNtupleIColumn(id,1,fAmountPhotons);
        AnaMngr->FillNtupleDColumn(id,4,Duration);
        AnaMngr->AddNtupleRow(id);
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



    void AnalysisManagerHelper::FillPhotonGenTree(int &evtid, G4LorentzVector &pos,G4ThreeVector &mom,G4ThreeVector &pol,double wavelength, double &energy)
    {

         fPhotonGenBranch.evtID=evtid;
         fPhotonGenBranch.x=pos.x();
         fPhotonGenBranch.y=pos.y();
         fPhotonGenBranch.z=pos.z();
         fPhotonGenBranch.t=pos.t();
         fPhotonGenBranch.px=pol.x();
         fPhotonGenBranch.py=pol.y();
         fPhotonGenBranch.pz=pol.z();
         fPhotonGenBranch.mx=mom.x();
         fPhotonGenBranch.my=mom.y();
         fPhotonGenBranch.mz=mom.z();
         fPhotonGenBranch.wavelength=wavelength;
         fPhotonGenBranch.energy=energy;
         fPhotonGenTTree->Fill();
    }


        void AnalysisManagerHelper::FillEdepTree(int &evtid, G4LorentzVector &pos, int trkid, int pdg, int nphot, int nelect)
    {
         fSimEdepsGenBranch.evtID=evtid;
         fSimEdepsGenBranch.x=pos.x();
         fSimEdepsGenBranch.y=pos.y();
         fSimEdepsGenBranch.z=pos.z();
         fSimEdepsGenBranch.t=pos.t();
         fSimEdepsGenBranch.TrackID=trkid;
         fSimEdepsGenBranch.nphot=nphot;
         fSimEdepsGenBranch.PDG=pdg;
         fSimEdepsGenBranch.nelect=nelect;
         fSimEdepGenTTree->Fill();
    }
    void AnalysisManagerHelper::FillHitTree(laropticks::OpticksHit &hit){
        fOpticksHitBranch=hit;
        if (fOpticksHitTTree!= nullptr) fOpticksHitTTree->Fill();
        else std::cout << "Opticks Hit Tree Not Initialized ...." << std::endl;
    }

     void AnalysisManagerHelper::FillVoxelTree(laropticks::Visibility &vis){
        fVisibilityBranch=vis;
        fVisTTree->Fill();
    }
	void AnalysisManagerHelper::FillPerformanceTree(laropticks::PerformanceTime *per){

        fPerformanceTimeBranch=*per;
     	if (fPerformanceTimeTTree!= nullptr) fPerformanceTimeTTree->Fill();
     	else std::cout << "Performance Time Tree Not Initialized ...." << std::endl;
    }

    void AnalysisManagerHelper::initVoxelTree(){

        fVisTTree = fTFileService->make<TTree>("Visibilities", "Visibilities as a function of VoxelID and SensorID");
		fVisTTree->Branch("Voxel", &fVisibilityBranch.id, "Voxel/I");
		fVisTTree->Branch("OptDetID", &fVisibilityBranch.sensorid, "OptDetID/I");
		fVisTTree->Branch("Visibility", &fVisibilityBranch.Visibility, "Visibility/D");
    }


    void AnalysisManagerHelper::initOpticksHitTree(){
        fOpticksHitTTree = fTFileService->make<TTree>("OpticksHits", "Opticks GPU Photon Hits");
        fOpticksHitTTree->Branch("evtID", &fOpticksHitBranch.evtID, "evtID/I");
        fOpticksHitTTree->Branch("hit_id", &fOpticksHitBranch.hit_id, "hit_id/I");
        fOpticksHitTTree->Branch("parent_id", &fOpticksHitBranch.parent_id, "parent_id/I");
        fOpticksHitTTree->Branch("sensor_id", &fOpticksHitBranch.sensor_id, "sensor_id/I");
		fOpticksHitTTree->Branch("x", &fOpticksHitBranch.x, "x/D");
		fOpticksHitTTree->Branch("y", &fOpticksHitBranch.y, "y/D");
		fOpticksHitTTree->Branch("z", &fOpticksHitBranch.z, "z/D");
		fOpticksHitTTree->Branch("t", &fOpticksHitBranch.time, "t/D");
	    fOpticksHitTTree->Branch("px", &fOpticksHitBranch.polx, "px/D");
		fOpticksHitTTree->Branch("py", &fOpticksHitBranch.poly, "py/D");
		fOpticksHitTTree->Branch("pz", &fOpticksHitBranch.polz, "pz/D");
		fOpticksHitTTree->Branch("mx", &fOpticksHitBranch.momx, "mx/D");
		fOpticksHitTTree->Branch("my", &fOpticksHitBranch.momy, "my/D");
		fOpticksHitTTree->Branch("mz", &fOpticksHitBranch.momz, "mz/D");
        fOpticksHitTTree->Branch("wavelength", &fOpticksHitBranch.wavelength, "wavelength/D");
		fOpticksHitTTree->Branch("boundary", &fOpticksHitBranch.boundary, "boundary/D");

    }
    void AnalysisManagerHelper::initPhotonGenTree(){
        fPhotonGenTTree = fTFileService->make<TTree>("photon_gen", "Photons produced during LightSource Modules");
        fPhotonGenTTree->Branch("evtID", &fPhotonGenBranch.evtID, "evtID/I");
		fPhotonGenTTree->Branch("x", &fPhotonGenBranch.x, "x/D");
		fPhotonGenTTree->Branch("y", &fPhotonGenBranch.y, "y/D");
		fPhotonGenTTree->Branch("z", &fPhotonGenBranch.z, "z/D");
		fPhotonGenTTree->Branch("t", &fPhotonGenBranch.t, "t/D");
	    fPhotonGenTTree->Branch("px", &fPhotonGenBranch.px, "px/D");
		fPhotonGenTTree->Branch("py", &fPhotonGenBranch.py, "py/D");
		fPhotonGenTTree->Branch("pz", &fPhotonGenBranch.pz, "pz/D");
		fPhotonGenTTree->Branch("mx", &fPhotonGenBranch.mx, "mx/D");
		fPhotonGenTTree->Branch("my", &fPhotonGenBranch.my, "my/D");
		fPhotonGenTTree->Branch("mz", &fPhotonGenBranch.mz, "mz/D");
		fPhotonGenTTree->Branch("wavelength", &fPhotonGenBranch.wavelength, "wavelength/D");
		fPhotonGenTTree->Branch("energy", &fPhotonGenBranch.energy, "energy/D");
    }

    void AnalysisManagerHelper::initIonAndScintGenTree(){
        fSimEdepGenTTree = fTFileService->make<TTree>("SimEnergyDeposit", "IonAndScint Info Saved");
        fSimEdepGenTTree->Branch("evtID", &fSimEdepsGenBranch.evtID, "evtID/I");
		fSimEdepGenTTree->Branch("TrackID", &fSimEdepsGenBranch.TrackID, "TrackID/I");
		fSimEdepGenTTree->Branch("PDG", &fSimEdepsGenBranch.PDG, "PDG/I");
		fSimEdepGenTTree->Branch("x", &fSimEdepsGenBranch.x, "x/D");
		fSimEdepGenTTree->Branch("y", &fSimEdepsGenBranch.y, "y/D");
		fSimEdepGenTTree->Branch("z", &fSimEdepsGenBranch.z, "z/D");
		fSimEdepGenTTree->Branch("t", &fSimEdepsGenBranch.t, "t/D");
        fSimEdepGenTTree->Branch("nphot", &fSimEdepsGenBranch.nphot, "nphot/I");
        fSimEdepGenTTree->Branch("nelect", &fSimEdepsGenBranch.nelect, "nelect/I");

	 }

    void AnalysisManagerHelper::initPerformanceTimeTree(){
			fPerformanceTimeTTree = fTFileService->make<TTree>("PerformanceTime", "Photon Performance");
			fPerformanceTimeTTree->Branch("evtID", &fPerformanceTimeBranch.evtID, "evtID/I");
			fPerformanceTimeTTree->Branch("PhotonAmount", &fPerformanceTimeBranch.PhotonAmount, "PhotonAmount/I");
			fPerformanceTimeTTree->Branch("time", &fPerformanceTimeBranch.time, "time/D");
	 }


}

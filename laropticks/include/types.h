
/**
 *  File: include/types.h
 *  Author: Ilker Parmaksiz
 *  Experiment: DUNE
 *  Institution: Rice University
 *  Date: 4/27/26.
 *  Description: Variables types that are shared by many classes
 */

#ifndef LAROPTICKS_TYPES_HH
#define LAROPTICKS_TYPES_HH

namespace laropticks {
    // Structs for TTree Generation
    struct OpticksHit{
        int evtID;
        int hit_id;
        int parent_id;
        int sensor_id;
        double time;
        double x,y,z;
        double momx,momy,momz;
        double polx,poly,polz;
        double wavelength;
        double boundary;
    };

    struct Visibility{
        int id, sensorid;
        double Visibility;
    };

    struct PhotonGen{
        int evtID;
        double x;
        double y;
        double z;
        double t;
        double px;
        double py;
        double pz;
        double mx;
        double my;
        double mz;
        double wavelength;
        double energy;
    };
}


#endif //LAROPTICKS_TYPES_HH
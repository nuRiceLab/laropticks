/**
 *  File: include/MySensorIdentifier.h
 *  Author: Ilker Parmaksiz
 *  Experiment: DUNE
 *  Institution: Rice University
 *  Date: 1/9/26
 *  Description: It allows opticks to recognize the optical sensors
**/
#pragma once

#ifndef OPTICKS_MYSENSORIDENTIFIER_H
#define OPTICKS_MYSENSORIDENTIFIER_H
#include <string>
#include <vector>
#include <map>
#include "U4SensorIdentifier.h"
#include "G4String.hh"

namespace phot {

  class MySensorIdentifier : public U4SensorIdentifier{

  public:
      MySensorIdentifier(std::map<G4String, G4int> &ids);
      ~MySensorIdentifier();
      virtual void setLevel(int _level);
      int getGlobalIdentity(const G4VPhysicalVolume*,const G4VPhysicalVolume*) override;
      int getInstanceIdentity(const G4VPhysicalVolume* ) const override ;

  private:
      std::map<G4String,G4int> &fDetectIds;
      int ids=0;
  };
}

#endif //OPTICKS_MYSENSORIDENTIFIER_HH

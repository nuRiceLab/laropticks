# Opticks-Tool for LArSoft.

## ⚙️ Prerequisites
- **CUDA Toolkit 13 Update 2 (Tested)**
- **NVIDIA GPU with OptiX support (9) (Tested)**
- **Geant4 (with GDML support enabled)**
- **Opticks**
- **ROOT (for hit testing)**

---

## 🛠️ Installation

It has to be cloned under srcs folder along other LArSoft packages such as larsim.
It will be automatically detected and compiled during "mrb i"
### 1. Clone Repositories
```bash
git clone https://github.com/nuRiceLab/laropticks
```

### 2. Compile
Compile it with other LArSoft packages as follows
```bash
mrb i -j20 
```

### 3. Example Run
Source some enviroment variables for opticks

```bash
source laropticks/OpticksEnv
```

Within your working directory, test the tool by following
Run a single GENIE event
```bash
lar -n 1 -c prodgenie_nu_dune10kt_1x2x6.fcl -o Genie.root
```

Then load the Geni.root file to simulate optical photons
```bash
lar -n 1 -c standard_opticks_dune10kt.fcl -s Genie.root -o opticks.root
```

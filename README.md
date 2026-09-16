# Opticks Module for LArSoft

## ⚙️ Prerequisites
- **CUDA Toolkit 13 Update 2 (Tested)**
- **NVIDIA GPU with OptiX support (9) (Tested)**
- **Geant4 (with GDML support enabled)**
- **[Opticks](https://github.com/nuRiceLab/RiceOpticks)**
- **ROOT (for hit testing)**

---

## 🐳 Containerized Environment (Alma9 Spack Ready Image)
This repository provides a containerized environment based on AlmaLinux 9, pre-configured with Spack for the RiceOpticks project. It is highly recommended to use this image to easily meet all prerequisites.

**Key Features:**
* **CVMFS Integrated:** Relies on the CernVM File System (CVMFS) for software distribution.
* **DUNE Spack:** Loads Spack directly from the DUNE repository.
* **Pre-activated Environment:** Automatically sets up and activates the `dune-prototype` Spack environment.

### 1. Download and Run the Container
Set up a local cache directory and pull the image using Apptainer. Then, start an interactive shell with NVIDIA GPU support (`--nv`) and the required CVMFS mount (`--bind /cvmfs`):

```bash
mkdir -p cache/.apptainer/tmp
apptainer pull riceopticks.sif oras://ghcr.io/nuricelab/laropticks:spack-latest
apptainer shell --nv --bind /cvmfs riceopticks.sif
```

### 2. Initialize the Environment
Once inside the container, source the setup script to load the DUNE Spack environment and initialize the required variables:

```bash
Apptainer> source /opt/setup.sh
```

Run the command `OpticksExample` at any time to print a reminder of these execution steps.
```bash
OpticksExample
```
---

## 🛠️ Installation

The module has to be cloned under the `srcs` folder alongside other LArSoft packages such as `larsim`. It will be automatically detected and compiled during `mrb i`.

### 1. Clone the Repository
```bash
git clone [https://github.com/nuRiceLab/laropticks](https://github.com/nuRiceLab/laropticks)
```

### 2. Compile
Compile it with your other LArSoft packages as follows:
```bash
mrb i -j20 
```

---

## 🚀 Example Run

Once compiled, source the necessary environment variables for Opticks:
#### (NOTE: Skip this step if you are running inside the container)
```bash
source laropticks/OpticksEnv 
```

Within your working directory, test the tool by generating a single GENIE event:
```bash
lar -n 1 -c prodgenie_nu_dune10kt_1x2x6.fcl -o Genie.root
```

Then, load the `Genie.root` file to simulate optical photons with Opticks:
```bash
lar -n 1 -c standard_opticks_dunefd_1x2x6.fcl -s Genie.root -o opticks.root
```

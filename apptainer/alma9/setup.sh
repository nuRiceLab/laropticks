#!/bin/bash
# RiceOpticks RUNTIME environment. Source this to run Opticks.
# Needs /cvmfs (dune + larsoft) mounted, and an R580+ driver via --nv.
# Load Spack enviroment
GREEN="\e[32m"
RESET="\e[0m"
echo -e "${GREEN}Loading DUNE spack enviroment ... ${RESET}"
source /cvmfs/dune.opensciencegrid.org/spack/v1.2.2/share/spack/setup-env.sh
echo -e "${GREEN}Activating dune-protoype enviroment ... ${RESET}"
spack env activate dune-prototype

echo -e "${GREEN}Loading dunesw ...${RESET}"
spack load dunesw

# This is for workDir and build folders for PDFullSimOpticks Module 
export wrkDir=/opt/LArOpticks
buildfolder=build


export OPTICKS_HOME=/opt/opticks

opticks-(){  [ -r $OPTICKS_HOME/opticks.bash ] && . $OPTICKS_HOME/opticks.bash && opticks-env $* ; }
opticks-

o(){ opticks- ; cd $(opticks-home) ; git status ; }
oo(){ opticks- ; cd $(opticks-home) ; om- ; om-- ;  }
t(){ typeset -f $* ; }

oed(){ vi $BASH_SOURCE && ofu ; }  # edit this file
ofu(){ source $BASH_SOURCE ; }  # source this file

# container CUDA 13 must win over Spack's CUDA
export PATH=/usr/local/cuda/bin:$PATH
export LD_LIBRARY=/usr/local/cuda/lib64:$LD_LIBRARY
export OPTICKS_PREFIX=/usr/local/opticks
export OPTICKS_CUDA_PREFIX=/usr/local/cuda
export OPTICKS_OPTIX_PREFIX=/opt/optix/current
export PYTHONPATH=/opt
export CMAKE_PREFIX_PATH=$OPTICKS_CUDA_PREFIX:$CMAKE_PREFIX_PATH
opticks-check-prefix
opticks-setup > /dev/null



# PDFUllSimOpticks
echo -e "${GREEN}Setting up enviroment variables for PDFullSimOpticks ...${RESET}"



#Exporting enviroment variables
export LarOpticks=$wrkDir/srcs/laropticks
export LarOpticksBuild=$wrkDir/${buildfolder}
export FHICL_FILE_PATH=$LarOpticks/laropticks/fcl:$FHICL_FILE_PATH
export FW_SEARCH_PATH=$LarOpticks/laropticks/GDML:$FW_SEARCH_PATH
export LD_LIBRARY_PATH=$LarOpticksBuild/lib:$LD_LIBRARY_PATH
export CET_PLUGIN_PATH=$LarOpticksBuild/lib:$CET_PLUGIN_PATH

# function to quickly build 
larbuild() {
    mkdir -p  $wrkDir
    mkdir -p  $LarOpticks
    mkdir -p $LarOpticksBuild
    cmake -S $LarOpticks -B $LarOpticksBuild
    cd $LarOpticksBuild && make -j10
}


alias wrk="cd $wrkDir"
alias larbuild=larbuild
alias larclone_develop="git clone -b develop https://github.com/nuRiceLab/laropticks.git $LarOpticks"
alias larclone="git clone https://github.com/nuRiceLab/laropticks.git $LarOpticks"
export LANG=C
export LC_ALL=C

source $LarOpticks/OptickEnv


#### Example Event ###
Example() {

	# Define colors
	RED="\e[31m"
	YELLOW="\e[33m"
	CYAN="\e[36m"


        echo -e "${GREEN}Create a work directory by executing 'mkdir work'${RESET}"
        sleep 1

        echo -e "${CYAN}Create a GENIE event...${RESET}"
        echo -e "${YELLOW}lar -c prodgenie_nu_dune10kt_1x2x6.fcl -o Genie.root${RESET}"
        sleep 1

        echo -e "${CYAN}Simulate photons with Opticks using Genie.root${RESET}"
        echo -e "${YELLOW}lar -c standard_opticks_dunefd_1x2x6.fcl -s Genie.root -o g4_withOpticks.root${RESET}"
        sleep 1

        echo -e "${CYAN}Digitize the results with detsim...${RESET}"
        echo -e "${YELLOW}lar -c standard_detsim_opticks_dune10kt_1x2x6.fcl -s g4_withOpticks.root${RESET}"
        sleep 1
}



alias OpticksExample=Example

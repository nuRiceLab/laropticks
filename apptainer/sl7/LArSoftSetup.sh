#!/bin/bash

COMPILER=e26
DIRECTORY=Release
cd $HOME

USERNAME=`whoami`
Install=$HOME/dep/install
export Sim=OpticalSims/
export Opticks_Dep_install=${Install}
export PATH=$HOME/dep/cuda/bin:${Install}:${Install}/bin:$PATH
export LD_LIBRARY_PATH=${Install}/lib64:$LD_LIBRARY_PATH
export wrkDir=$HOME/${DIRECTORY}/work


# setup_dune
source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh


setup dunesw v10_20_00d00 -q e26:prof
cd ${HOME}
export MRB_CMAKE_ARGS="-DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++)"
if [[ "$1" == "clean" ]]; then
   touch $HOME/${DIRECTORY}
   rm -rf $HOME/${DIRECTORY}
   mkdir $HOME/${DIRECTORY}
   cd $HOME/${DIRECTORY}
   #setup larsoft v10_14_01 -q e26:prof
   setup dunesw v10_20_00d00 -q e26:prof

   mrb newDev -q ${COMPILER}:prof
fi

source $HOME/${DIRECTORY}/localProducts*/setup

source $HOME/.opticks_config_Ricelatest

cd $HOME/${DIRECTORY}/srcs


# checks out the develop versions of the split repositories
LoadAndInstall (){
     # Ensure DIRECTORY is set before proceeding
    local target_dir="$HOME/${DIRECTORY:?DIRECTORY variable is not set}/srcs"

    # Ensure target directory exists and enter it safely
    mkdir -p "$target_dir" || return 1
    cd "$target_dir" || return 1

    if [ -d "$HOME/laropticks" ]; then
        # Overwrite or update existing symlink safely
        ln -snf "$HOME/laropticks" "$target_dir/laropticks"
    elif [ ! -d "laropticks" ]; then
        # Only clone if the destination directory doesn't already exist
        git clone https://github.com/nuRiceLab/laropticks.git
    fi
}


if [[ "$1" == "clean" ]]; then
   LoadAndInstall;
fi
mrb uc
cd $MRB_BUILDDIR
mrbsetenv		# Sets up the build environment

export CC=$(which gcc)
export CXX=$(which g++)
export FC=$(which gfortran)
#Some Shortcuts
alias cm="cd ${MRB_BUILDDIR}/laropticks && make install -j10"
alias cpgdml=" cp $HOME/laropticks/laropticks/GDML/* $MRB_BUILDDIR/laropticks/gdml/"
alias cmk="cmake -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++) .."
alias ccmk="ccmake -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++) .."
alias nvrun="${HOME}/dep/glibc-2.42/install/run-with-glibc-2.42.sh"
alias nvrun2="${HOME}/dep/glibc-2.42/install/run-with-glibc-2.42v2.sh"
alias wrk="cd $wrkDir"
# Load Opticks Enviroments
export CMAKE_PREFIX_PATH=$Opticks_Dep_install:$CMAKE_PREFIX_PATH
source $HOME/laropticks/OptickEnv

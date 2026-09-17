

## Adjust the paths bellow as needed for your env.
## Example Run Command:
# source ApptainerSetup.sh for alma9 container
# source ApptainerSetup.sh sl7 for sl7 container

run_container () {
    local IMAGE=""
    echo "your input is $1 " 
    if [ "$1" == "sl7" ]; then
	echo "Loading sl7 container ..."    
        IMAGE="/cvmfs/singularity.opensciencegrid.org/fermilab/fnal-dev-sl7:latest"
    else
	echo "Loading alma9 container ..."    
        IMAGE="/cvmfs/singularity.opensciencegrid.org/fermilab/fnal-wn-el9:devel"
    fi

    /cvmfs/oasis.opensciencegrid.org/mis/apptainer/current/bin/apptainer shell \
        --home /home/rice/Projects/DUNE_Spack:/DUNE_Spack \
        --nv \
        --shell=/bin/bash \
        -B /run/user,/usr/lib64:/srv/host_lib64,/home/rice/Projects/DUNE_Opticks,/cvmfs \
        --ipc --pid \
        "$IMAGE"
}

run_container $1

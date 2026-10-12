#!/bin/bash
# =============================================================================
#
# SLURM batch job: laropticks GPU optical simulation.
# Runs laropticks/scripts/nersc_laropticks.sh inside the Apptainer container.
#
#   sbatch -A dune_g laropticks/scripts/nersc-gpu.sbatch
#   sbatch -A dune_g -q debug -t 00:30:00 --export=ALL,NEVENTS=2 \
#          laropticks/scripts/nersc-gpu.sbatch                      # smoke test
#   sbatch -A dune_g --array=0-99%20 \
#          --export=ALL,NEVENTS=200,SKIP_CACHE_BOOTSTRAP=1 \
#          laropticks/scripts/nersc-gpu.sbatch                      # production
#
# =============================================================================

#SBATCH --job-name=laropticks-gpu
#SBATCH --constraint=gpu
#SBATCH --qos=shared
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --gpus-per-task=1
#SBATCH --cpus-per-task=32
#SBATCH --time=06:00:00
#SBATCH --output=%x-%A_%a.out
#SBATCH --error=%x-%A_%a.err
##SBATCH --account=dune_g
##SBATCH --mail-type=END,FAIL
##SBATCH --mail-user=you@example.edu

set -u

# --- knobs (override with --export=ALL,VAR=value) ----------------------------
LAROPTICKS_DIR=${LAROPTICKS_DIR:-$HOME/laropticks}
SCRIPTS_DIR=$LAROPTICKS_DIR/laropticks/scripts

# The inner script, run inside the container. Change here if it is renamed.
INNER_SCRIPT=${INNER_SCRIPT:-$SCRIPTS_DIR/nersc_laropticks.sh}

IMAGE=${IMAGE:-/cvmfs/singularity.opensciencegrid.org/dunescience/laropticks:spack-latest}
FCL=${FCL:-$LAROPTICKS_DIR/laropticks/fcl/production/prodgenie_g4_opticks_dunefd_1x2x6.fcl}
NEVENTS=${NEVENTS:-100}
RUNROOT=${RUNROOT:-$PSCRATCH/laropticks-runs}
SKIP_CACHE_BOOTSTRAP=${SKIP_CACHE_BOOTSTRAP:-}
RANDOMIZE=${RANDOMIZE:-1}          # 0 = use the fcl's seeds/run number as-is
BASE_RUN=${BASE_RUN:-20000001}     # run number for array task 0

# Array-aware tag and output directory, so tasks never collide.
if [ -n "${SLURM_ARRAY_TASK_ID:-}" ]; then
    TAG=${SLURM_ARRAY_JOB_ID}_${SLURM_ARRAY_TASK_ID}
    TASK_ID=$SLURM_ARRAY_TASK_ID
else
    TAG=${SLURM_JOB_ID:-manual-$$}
    TASK_ID=0
fi
OUTDIR=${OUTDIR:-$RUNROOT/job_${TAG}}

# Apptainer from the OSG CVMFS distribution.
# current fails, use 1.3.3 
#     export APPTAINER_ROOT=`/cvmfs/.../bin/apptainer`   <-- EXECUTES apptainer
# APPTAINER_ROOT is the install DIRECTORY, so $APPTAINER_ROOT/bin is a dir.
APPTAINER_ROOT=${APPTAINER_ROOT:-/cvmfs/oasis.opensciencegrid.org/mis/apptainer/1.3.3}
export PATH="${APPTAINER_ROOT}/bin:/sbin:/usr/sbin:${PATH}"


# Keep Apptainer scratch off $HOME, which is quota'd and slow.
export APPTAINER_TMPDIR=${APPTAINER_TMPDIR:-$PSCRATCH/apptainer-tmp}
export APPTAINER_CACHEDIR=${APPTAINER_CACHEDIR:-$PSCRATCH/apptainer-cache}
mkdir -p "$APPTAINER_TMPDIR" "$APPTAINER_CACHEDIR" "$OUTDIR"

echo "=============================================================="
echo " laropticks GPU job"
echo "   tag       : $TAG   (task $TASK_ID)   on $(hostname)"
echo "   started   : $(date -Is)"
echo "   image     : $IMAGE"
echo "   inner     : $INNER_SCRIPT"
echo "   fcl       : $FCL"
echo "   events    : $NEVENTS"
echo "   outdir    : $OUTDIR"
echo "   randomize : $RANDOMIZE  (base run $BASE_RUN)"
echo "=============================================================="

# --- preflight: fail here, not 30 s into the container -----------------------
command -v apptainer > /dev/null \
    || { echo "FATAL: apptainer not on PATH (APPTAINER_ROOT=$APPTAINER_ROOT)"; exit 1; }
[ -e "$IMAGE" ]         || { echo "FATAL: image not found: $IMAGE"; exit 1; }
[ -f "$FCL" ]           || { echo "FATAL: fcl not found: $FCL"; exit 1; }
[ -f "$INNER_SCRIPT" ]  || { echo "FATAL: inner script not found: $INNER_SCRIPT"; exit 1; }
[ -f "$SCRIPTS_DIR/opticks-gpu-env.sh" ] \
    || { echo "FATAL: opticks-gpu-env.sh missing under $SCRIPTS_DIR"; exit 1; }

# /cvmfs is autofs: touch each repository so it mounts before the bind.
for repo in oasis.opensciencegrid.org singularity.opensciencegrid.org \
            larsoft.opensciencegrid.org dune.opensciencegrid.org ; do
    if ls "/cvmfs/$repo" > /dev/null 2>&1 ; then echo "  cvmfs ok   : $repo"
    else                                         echo "  cvmfs MISS : $repo" >&2 ; fi
done

nvidia-smi -L || { echo "FATAL: no GPU on this node"; exit 1; }

# --- run ---------------------------------------------------------------------
# srun so the task gets the GPU binding and CPU affinity SLURM allocated.
# /tmp comes into the container by default and is where the geometry cache is
# staged; /pscratch carries the cache tarball and the outputs.
srun --ntasks=1 --cpus-per-task="${SLURM_CPUS_PER_TASK:-32}" \
     apptainer exec --nv \
        --bind /cvmfs \
        --bind "$HOME" \
        --bind /pscratch \
        --env LAROPTICKS_DIR="$LAROPTICKS_DIR" \
        --env FCL="$FCL" \
        --env NEVENTS="$NEVENTS" \
        --env OUTDIR="$OUTDIR" \
        --env TAG="$TAG" \
        --env TASK_ID="$TASK_ID" \
        --env PSCRATCH="$PSCRATCH" \
        --env SKIP_CACHE_BOOTSTRAP="$SKIP_CACHE_BOOTSTRAP" \
        --env RANDOMIZE="$RANDOMIZE" \
        --env BASE_RUN="$BASE_RUN" \
        "$IMAGE" \
        bash "$INNER_SCRIPT"
rc=$?

echo "=============================================================="
echo " job $TAG exit $rc at $(date -Is)"
echo " outputs in $OUTDIR"
ls -lh "$OUTDIR" 2>/dev/null
echo "=============================================================="

# Keep the SLURM logs next to the data rather than wherever sbatch was run.
for ext in out err ; do
    cp -f "${SLURM_SUBMIT_DIR:-.}/${SLURM_JOB_NAME:-laropticks-gpu}-${TAG}.${ext}" \
          "$OUTDIR/" 2>/dev/null
done

exit $rc

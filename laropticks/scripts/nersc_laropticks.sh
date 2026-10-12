#!/bin/bash
# =============================================================================
#
# Runs INSIDE the laropticks Apptainer container on a GPU node.
#
#   apptainer shell --nv --bind /cvmfs --bind $HOME --bind /pscratch <image>
#   bash $HOME/laropticks/laropticks/scripts/nersc_laropticks.sh
#
# Three steps:
#   1. set up the Spack/LArSoft environment   (/opt/setup.sh)
#   2. make a geometry cache available        (stage a tarball, or build one)
#   3. run `lar`                              (GENIE -> G4 -> IonAndScint -> Opticks)
#
# Environment (nersc-gpu.sbatch sets all of these; defaults are for by-hand use):
#   LAROPTICKS_DIR        clone of nuRiceLab/laropticks  (default $HOME/laropticks)
#   FCL                   fcl to run
#   NEVENTS               events to process              (default 100)
#   OUTDIR                writable output directory      (default $PWD)
#   TAG                   label for output filenames     (default "local")
#   TASK_ID               array task index               (default 0)
#   RANDOMIZE             1 = per-task seeds + run no.   (default 1)
#   BASE_RUN              run number for task 0          (default 20000001)
#   OPTICKS_CACHE_TAR     cache tarball                  (default $PSCRATCH/$GEOM.tar)
#   SKIP_CACHE_BOOTSTRAP  non-empty = fail instead of building a missing cache
# =============================================================================

LAROPTICKS_DIR=${LAROPTICKS_DIR:-$HOME/laropticks}
FCL=${FCL:-$LAROPTICKS_DIR/laropticks/fcl/production/prodgenie_g4_opticks_dunefd_1x2x6.fcl}
NEVENTS=${NEVENTS:-100}
OUTDIR=${OUTDIR:-$PWD}
TAG=${TAG:-local}
TASK_ID=${TASK_ID:-0}
RANDOMIZE=${RANDOMIZE:-1}
BASE_RUN=${BASE_RUN:-20000001}

echo "=============================================================="
echo " inside container on $(hostname)"
echo "   tag     : $TAG   (task $TASK_ID)"
echo "   fcl     : $FCL"
echo "   events  : $NEVENTS"
echo "   outdir  : $OUTDIR"
echo "=============================================================="

[ -f "$FCL" ] || { echo "FATAL: fcl not found: $FCL"; exit 1; }
mkdir -p "$OUTDIR" || { echo "FATAL: cannot create $OUTDIR"; exit 1; }
cd "$OUTDIR" || exit 1

# --- 1. Spack / LArSoft environment -----------------------------------------
# Relax errexit/nounset: the setup scripts touch unset vars and can return
# non-zero benignly. The explicit gates below catch real failures.
set +eu
source /opt/setup.sh
set +eu

command -v lar > /dev/null || { echo "FATAL: lar not on PATH -- /opt/setup.sh did not take"; exit 1; }
: "${GEOM:?FATAL: GEOM unset after /opt/setup.sh}"
echo "--- environment"
echo "  lar  : $(command -v lar)"
echo "  GEOM : $GEOM"
nvidia-smi -L || { echo "FATAL: no GPU visible -- was --nv passed?"; exit 1; }

# --- 2. geometry cache -------------------------------------------------------
# The cache is ~269k small files. Writing that to a shared filesystem is slow
# enough to burn an allocation, so it lives as ONE tarball on Scratch and is
# extracted into node-local /tmp (in-memory) at job start -- a few seconds.
export OPTICKS_CACHE_TAR=${OPTICKS_CACHE_TAR:-${PSCRATCH:?PSCRATCH unset}/$GEOM.tar}

if [ ! -f "$OPTICKS_CACHE_TAR" ]; then
    if [ -n "${SKIP_CACHE_BOOTSTRAP:-}" ]; then
        echo "FATAL: no cache tarball at $OPTICKS_CACHE_TAR and bootstrap disabled"
        exit 1
    fi
    echo "--- no cache tarball; bootstrapping one (first run only)"
    # Mode 2 sets NoGPU: it skips CSGOptiX::Create but still runs
    # U4Tree::Create -> initSceneFromTree -> CSGFoundry::save, i.e. the whole
    # cache. Written to node-local /tmp, never to $HOME.
    BOOT=/tmp/$USER/geocache
    rm -rf "$BOOT"; mkdir -p "$BOOT"
    (
        export OPTICKS_INTEGRATION_MODE=2
        export G4CXOpticks__setGeometry_saveGeometry=$BOOT/$GEOM
        export SSim=INFO SScene=INFO CSGFoundry=INFO
        time lar -c "$FCL" -n 1 \
             -o "$OUTDIR/cachebuild.root" -T "$OUTDIR/cachebuild_hist.root"
    )
    [ -d "$BOOT/$GEOM/CSGFoundry" ] \
        || { echo "FATAL: cache bootstrap produced no CSGFoundry"; exit 1; }
    [ -n "$(find "$BOOT/$GEOM" -type d -name meshgroup -print -quit)" ] \
        || { echo "FATAL: no SSim/scene/meshgroup -- the scene save did not finish"; exit 1; }
    NF=$(find "$BOOT/$GEOM" -type f | wc -l)
    [ "$NF" -gt 10000 ] || { echo "FATAL: cache looks truncated ($NF files)"; exit 1; }
    echo "--- cache built: $NF files; packing to $OPTICKS_CACHE_TAR"
    # Temp name then rename, so a concurrent job never reads a half-written tar.
    time tar cf "$OPTICKS_CACHE_TAR.$$" -C "$BOOT" "$GEOM"
    mv -f "$OPTICKS_CACHE_TAR.$$" "$OPTICKS_CACHE_TAR"
    ls -lh "$OPTICKS_CACHE_TAR"
    rm -f "$OUTDIR/cachebuild.root" "$OUTDIR/cachebuild_hist.root"
fi

# Stages the tarball into /tmp, sets CFBASE, unsets the save variable, picks the
# GPU and applies the GL shim. Must be sourced AFTER /opt/setup.sh.
source "$LAROPTICKS_DIR/laropticks/scripts/opticks-gpu-env.sh"

[ -d "${CFBASE:-/nonexistent}/CSGFoundry" ] \
    || { echo "FATAL: geometry cache not usable (CFBASE=${CFBASE:-unset})"; exit 1; }

# --- 2b. per-task randomisation ----------------------------------------------
# Without this every array task runs the same GENIE configuration with the same
# seeds and produces IDENTICAL events -- N copies of one sample. A thin wrapper
# fcl includes the production one and overrides the seed policy and run number.
#
# NB fhicl `#include` resolves against FHICL_FILE_PATH -- it is NOT a filesystem
# path, so `#include "/abs/path/to.fcl"` fails with "search_path ... Can't find
# file". The production fcl's directory therefore goes on FHICL_FILE_PATH and
# the include uses its basename. (`lar -c /abs/path.fcl` for the TOP-LEVEL file
# is fine; only includes go through the search path.)
export FHICL_FILE_PATH="$(dirname "$FCL"):${FHICL_FILE_PATH:-}"

RUN_FCL=$FCL
if [ "$RANDOMIZE" = "1" ]; then
    RUN_NUMBER=$(( BASE_RUN + TASK_ID ))
    RUN_FCL=$OUTDIR/run_${TAG}.fcl
    cat > "$RUN_FCL" <<EOF
# generated by nersc_laropticks.sh for task $TASK_ID -- do not edit
#include "$(basename "$FCL")"

# seed every engine from system entropy, so no two tasks repeat a sequence
services.NuRandomService.policy: "random"

# distinct run number per task, so event IDs stay unique across the campaign
source.firstRun: $RUN_NUMBER
EOF
    echo "--- randomised: run $RUN_NUMBER, NuRandomService policy random"
    echo "    wrapper fcl : $RUN_FCL"
    echo "    include     : $(basename "$FCL")  (from $(dirname "$FCL"))"
else
    echo "--- RANDOMIZE=0: using $FCL seeds and run number as-is"
fi

# --- 3. run ------------------------------------------------------------------
OUTFILE=$OUTDIR/prodgenie_g4_opticks_${TAG}.root
HISTFILE=$OUTDIR/prodgenie_g4_opticks_hist_${TAG}.root

echo "--- running lar on $NEVENTS events"
time lar -c "$RUN_FCL" -n "$NEVENTS" -o "$OUTFILE" -T "$HISTFILE"
rc=$?

echo "=============================================================="
echo " finished : $(date -Is)   rc=$rc"
if [ "$rc" -eq 0 ]; then
    ls -lh "$OUTFILE" "$HISTFILE" 2>/dev/null
else
    echo " FAILED -- see the log above"
fi
echo "=============================================================="
exit $rc

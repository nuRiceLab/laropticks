#!/bin/bash
# opticks-gpu-env.sh
#
# Source this INSIDE the container on a GPU node:
#
#   source /opt/setup.sh
#   source ~/opticks-gpu-env.sh
#
# It stages the geometry cache tarball from Scratch into node-local /tmp
# (in-memory on Perlmutter, so 269k tiny files cost almost nothing), points
# Opticks at it, and stops Opticks from rebuilding the cache.
#
# Container must be started with /pscratch bound:
#   apptainer shell --nv --bind /cvmfs --bind $HOME --bind /pscratch <image>.sif
#
# Overrides, set before sourcing:
#   OPTICKS_CACHE_TAR    tarball to stage        (default $PSCRATCH/$GEOM.tar)
#   OPTICKS_CACHE_STAGE  where to extract        (default /tmp/$USER/geocache)
#   CFBASE               skip staging entirely, use this cache as-is

# ---------------------------------------------------------------------------
# 1. Stage the cache into node-local /tmp
# ---------------------------------------------------------------------------
_stage=${OPTICKS_CACHE_STAGE:-/tmp/$USER/geocache}
_tar=${OPTICKS_CACHE_TAR:-${PSCRATCH:-}/${GEOM:-unset}.tar}

if [ -n "${CFBASE:-}" ]; then
    echo "note: CFBASE preset to $CFBASE, skipping staging"
elif [ -d "$_stage/${GEOM:-unset}/CSGFoundry" ]; then
    export CFBASE=$_stage/$GEOM
    echo "note: cache already staged at $CFBASE (reusing)"
elif [ -f "$_tar" ]; then
    mkdir -p "$_stage"
    echo "note: extracting $_tar -> $_stage"
    time tar xf "$_tar" -C "$_stage"
    export CFBASE=$_stage/$GEOM
else
    echo "ERROR: no cache tarball at $_tar and nothing staged at $_stage" >&2
    echo "       Build one first:  bash ~/opticks-cache-build.sh" >&2
    echo "       (is /pscratch bound into this container?)" >&2
fi

if [ -n "${CFBASE:-}" ] && [ ! -d "$CFBASE/CSGFoundry" ]; then
    echo "ERROR: no CSGFoundry under CFBASE=$CFBASE" >&2
fi

# ---------------------------------------------------------------------------
# 2. Do NOT rebuild the cache.
#    This is the variable that writes 269k files and costs minutes-to-tens-of-
#    minutes per job. Once a cache exists it should only ever be read.
# ---------------------------------------------------------------------------
unset G4CXOpticks__setGeometry_saveGeometry
unset G4CXOpticks__SaveGeometry_DIR

# ---------------------------------------------------------------------------
# 3. GPU mode. 2 is the CPU-only/NoGPU path used to BUILD the cache;
#    1 runs both GPU and CPU simulation.
# ---------------------------------------------------------------------------
export OPTICKS_INTEGRATION_MODE=${OPTICKS_INTEGRATION_MODE_OVERRIDE:-1}

# ---------------------------------------------------------------------------
# 4. GPU selection. OptickEnv hardcodes CUDA_VISIBLE_DEVICES=1, which picks the
#    second GPU -- wrong on a single-GPU node, and wrong on a MIG host, where
#    CUDA needs the instance UUID rather than an ordinal. Derive it instead.
# ---------------------------------------------------------------------------
if nvidia-smi -L 2>/dev/null | grep -q '^  MIG'; then
    export CUDA_VISIBLE_DEVICES=$(nvidia-smi -L | grep -o 'MIG-[0-9a-f-]*' | head -1)
    echo "note: MIG host, selecting instance by UUID"
else
    export CUDA_VISIBLE_DEVICES=${CUDA_VISIBLE_DEVICES:-0}
fi

# ---------------------------------------------------------------------------
# 5. Host GL libraries injected by --nv can be built against a newer glibc than
#    the container has (AlmaLinux 9 ships glibc 2.34; hosts seen with 2.38).
#    Geant4's vis drivers are linked into larg4, so libGL is dlopen'd even for a
#    non-visual job, failing with
#      GLIBC_2.38 not found (required by /.singularity.d/libs/libGLX.so.0)
#    Shadow only the GL stack -- not all of /usr/lib64, which would shadow
#    libxml2/libpng/sqlite and break the Spack stack.
# ---------------------------------------------------------------------------
if ldd /.singularity.d/libs/libGL.so.1 2>&1 | grep -q 'GLIBC_.* not found'; then
    mkdir -p /tmp/glfix
    for _l in libGL.so.1 libGLU.so.1 libGLX.so.0 libGLdispatch.so.0 \
              libEGL.so.1 libOpenGL.so.0 ; do
        [ -e /usr/lib64/$_l ] && ln -sf /usr/lib64/$_l /tmp/glfix/$_l
    done
    export LD_LIBRARY_PATH=/tmp/glfix:$LD_LIBRARY_PATH
    echo "note: host GL libs too new for this container; shadowing via /tmp/glfix"
fi

# ---------------------------------------------------------------------------
# 6. Report
# ---------------------------------------------------------------------------
echo "Opticks GPU environment:"
echo "  GEOM                     : ${GEOM:-<unset>}"
echo "  CFBASE                   : ${CFBASE:-<unset>}"
echo "  cache files              : $(find "${CFBASE:-/nonexistent}" -type f 2>/dev/null | wc -l)"
echo "  OPTICKS_INTEGRATION_MODE : $OPTICKS_INTEGRATION_MODE"
echo "  CUDA_VISIBLE_DEVICES     : $CUDA_VISIBLE_DEVICES"
echo "  OPTICKS_MAX_PHOTON       : ${OPTICKS_MAX_PHOTON:-<unset>}"
echo "  saveGeometry             : ${G4CXOpticks__setGeometry_saveGeometry:-unset (good: cache is READ-only)}"

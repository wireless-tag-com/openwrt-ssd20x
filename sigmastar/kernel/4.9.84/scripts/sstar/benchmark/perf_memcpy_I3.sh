FILTER_BASIC="-e cpu-cycles -e instructions -e cache-references -e cache-misses -e branch-instructions -e branch-misses -e stalled-cycles-frontend -e stalled-cycles-backend -e L1-dcache-loads -e L1-dcache-load-misses -e L1-dcache-stores -e L1-dcache-store-misses -e L1-icache-load-misses -e context-switches -e cpu-migrations -e page-faults -e dTLB-load-misses -e dTLB-store-misses -e iTLB-load-misses -e branch-loads -e branch-load-misses"

VERBOSE=
LOOP_BASE=10000
MEM_TYPE="MIU"
CACHABLE="1"

usage()
{
  echo ''
  echo "Usage: $0 BUFFER_SIZE L2_PMU_SELECT [memcpy scheme] [memory type] [cachable]"
  echo '    BUFFER_SIZE: number of KB for each iteration (total bytes transfer: 64KB * 10000)'
  echo '    L2_PMU_SELECT:  not valid for  I3'
  echo '    [memcpy scheme]: valid option 0|1|2'
  echo '                          0: C runtime memcpy'
  echo '                          1: memcpy.S with NEON'
  echo '                          2: memcpy.S without NEON'
  echo '    [memory type]: valid option MIU|IMI'
  echo '    [cachable]: valid option 0|1'
  echo ''
  echo "EXAMPLE: $0 32 r 0"
  echo '    [CRT] memcpy scheme test with [32]KB buffer for 20000 iterations and use perf PMU for profiling with addtional L2 PMU [drreq/drhit].'
  echo ''
}

if [ "$1" == "" ]
then
    usage
    exit 1
fi

BUFFER_SIZE=$(($1*1024))
LOOP_COUNT=$((64*1024*$LOOP_BASE/$BUFFER_SIZE))
echo 'R/W:' $2 ', BUFFER_SIZE: ' $BUFFER_SIZE ', LOOP_COUNT: ' $LOOP_COUNT ', neon: ' $3 ', mem type: ' $4 ', cachable: ' $5

FILTER_EX="-e LLC-loads -e LLC-load-misses -e LLC-stores -e LLC-store-misses"
FILTER_EX2="-e armv7_cortex_a7/l1d_cache/ -e armv7_cortex_a7/l1d_cache_refill/ -e armv7_cortex_a7/l1d_cache_wb/ -e armv7_cortex_a7/l2d_cache/ -e armv7_cortex_a7/l2d_cache_refill/ -e armv7_cortex_a7/l2d_cache_wb/ -e armv7_cortex_a7/ld_retired/ -e armv7_cortex_a7/st_retired/"
FILTER_RAW="-e r013 -e r015 -e r019 -e r01d -e r060 -e r061 -e r0c0 -e r0c1 -e r0c2 -e r0c3 -e r0c4 -e r0c5"

if [ "$4" = "IMI" ] || [ "$4" = "MIU" ]
then
  MEM_TYPE=$4
else
  MEM_TYPE="MIU"
fi

if [ "$5" = "0" ] || [ "$5" = "1" ]
then
  CACHABLE=$5
else
  CACHABLE="1"
fi

./perf stat $VERBOSE $FILTER_BASIC $FILTER_EX $FILTER_EX2 $FILTER_RAW echo $MEM_TYPE $CACHABLE $MEM_TYPE $CACHABLE $LOOP_COUNT $BUFFER_SIZE $3 > /sys/devices/virtual/sstar/msys/perf_test

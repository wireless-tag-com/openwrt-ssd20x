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
  echo '    L2_PMU_SELECT: valid option r|w|e|x'
  echo '                          r: drreq and drhit'
  echo '                          w: dwreq and dwhit'
  echo '                          e: cc and ipfalloc'
  echo '                          x: dwtreq and wa'
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
echo 'R/W:' $2 ', BUFFER_SIZE: ' $BUFFER_SIZE ', LOOP_COUNT: ' $LOOP_COUNT ', scheme: ' $3 ', mem type: ' $4 ', cachable: ' $5

if [ "$2" = "r" ]
then
  FILTER_EX="-e l2c_310/drreq/ -e l2c_310/drhit/"
elif [ "$2" = "w" ]
then
  FILTER_EX="-e l2c_310/dwreq/ -e l2c_310/dwhit/"
elif [ "$2" = "e" ]
then
  FILTER_EX="-e l2c_310/co/ -e l2c_310/ipfalloc/"
elif [ "$2" = "x" ]
then
  FILTER_EX="-e l2c_310/dwtreq/ -e l2c_310/wa/"
else
  echo 'ERROR: invalid parameter'
  echo ''
  usage
  exit 1
fi

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

./perf stat $VERBOSE $FILTER_BASIC $FILTER_EX echo $MEM_TYPE $CACHABLE $MEM_TYPE $CACHABLE $LOOP_COUNT $BUFFER_SIZE $3 > /sys/devices/virtual/sstar/msys/perf_test
#./perf stat $VERBOSE $FILTER_BASIC $FILTER_EX echo MIU 1 MIU 1 $LOOP_COUNT $BUFFER_SIZE $3 > /sys/devices/virtual/sstar/msys/perf_test
# -e l2c_310/co/ -e l2c_310/drhit/ -e l2c_310/drreq/ -e l2c_310/dwhit/ -e l2c_310/dwreq/ -e l2c_310/dwtreq/ -e l2c_310/epfalloc/ -e l2c_310/epfhit/ -e l2c_310/epfrcvd -e l2c_310/ipfalloc/ -e l2c_310/irhit/ -e l2c_310/irreq/ -e l2c_310/srconf/ -e l2c_310/srrcvd/ -e l2c_310/wa/

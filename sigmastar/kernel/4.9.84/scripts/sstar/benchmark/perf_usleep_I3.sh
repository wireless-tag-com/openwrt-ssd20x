FILTER_BASIC="-e cpu-cycles -e instructions -e cache-references -e cache-misses -e branch-instructions -e branch-misses -e stalled-cycles-frontend -e stalled-cycles-backend -e L1-dcache-loads -e L1-dcache-load-misses -e L1-dcache-stores -e L1-dcache-store-misses -e L1-icache-load-misses -e context-switches -e cpu-migrations -e page-faults -e dTLB-load-misses -e dTLB-store-misses -e iTLB-load-misses -e branch-loads -e branch-load-misses"
FILTER_EX="-e LLC-loads -e LLC-load-misses -e LLC-stores -e LLC-store-misses"
FILTER_EX2="-e armv7_cortex_a7/l1d_cache/ -e armv7_cortex_a7/l1d_cache_refill/ -e armv7_cortex_a7/l1d_cache_wb/ -e armv7_cortex_a7/l2d_cache/ -e armv7_cortex_a7/l2d_cache_refill/ -e armv7_cortex_a7/l2d_cache_wb/ -e armv7_cortex_a7/ld_retired/ -e armv7_cortex_a7/st_retired/"
FILTER_RAW="-e r013 -e r015 -e r019 -e r01d -e r060 -e r061 -e r0c0 -e r0c1 -e r0c2 -e r0c3 -e r0c4 -e r0c5"

CMDLINE="usleep"

./perf stat $FILTER_BASIC $FILTER_EX $FILTER_EX2 $FILTER_RAW $CMDLINE $1

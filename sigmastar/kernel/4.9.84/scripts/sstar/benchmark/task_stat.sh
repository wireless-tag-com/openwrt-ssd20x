printf "#\n# Auto detect find...@ "
if [ -e /usr/bin/find ]
then
    _FIND=/usr/bin/find
else
    _FIND=find
fi
printf "$_FIND\n"

printf "# Auto detect awk...@ "
if [ -e /usr/bin/awk ]
then
    _AWK=/usr/bin/awk
else
    _AWK=awk
fi
printf "$_AWK\n#\n"

printf "#%5s %6s %20s %6s %6s %6s %6s %6s\n" "PPID" "PID" "Name" "nThrd" "Prio" "Nice" "PrioRT" "policy"
#/usr/bin/find /proc -type f -mindepth 2 -maxdepth 2 -iname 'stat' -exec cat {} \; | /usr/bin/awk '{printf " PID %6d %20s priority %4d nice %3d\n",$1,$2,$18,$19}'
#/usr/bin/find /proc -type f -mindepth 2 -maxdepth 2 -iname 'stat' -exec cat {} \;| /usr/bin/awk '{printf "%6d %6d %30s %6d %6d %6d %6d %6d\n",$4,$1,$2,$20,$18,$19,$40,$41}'
$_FIND /proc -type f -mindepth 2 -maxdepth 2 -iname 'stat' -exec cat {} \; | $_AWK -F ' \(|\) ' '{
  for(i=1;i<=NF;i++) {
    if(i==2) {
      gsub(" ","/",$i)
    }
    printf "%s ",$i
  }
  printf "\n"
}' | $_AWK '{printf "%6d %6d %20s %6d %6d %6d %6d %6d\n",$4,$1,$2,$20,$18,$19,$40,$41}

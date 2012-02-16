rm x__*.out

for ((i=0;i<100;i=i+1)) do
echo -n "run $i: " >>x__$i.out
opp_run -n ../../../src:../../../examples:../../../tests/fingerprint -l ../../../src/inet -u Cmdenv -f omnetpp.ini -c General -r 0 --sim-time-limit=60s --fingerprint=35f1-bc7a --cpu-time-limit=40s --vector-recording=false --scalar-recording=false | grep Fingerprint >>x__$i.out
sleep 1
done
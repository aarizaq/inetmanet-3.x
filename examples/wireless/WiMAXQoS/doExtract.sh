#!/bin/bash
for ((i=1; i<=10; i++)) do
    echo "Extrahiere die Daten aus Durchlauf $i"
    grep ^203 omnetpp.$i.vec >wcli9.$i.dat
    grep ^158 omnetpp.$i.vec >wcli0.$i.dat
done
for ((i=1; i<=10; i++)) do
        echo "Extrahiere die Daten aus Durchlauf _du $i"
	    grep ^203 omnetpp_du.$i.vec >wcli9_du.$i.dat
	    grep ^158 omnetpp_du.$i.vec >wcli0_du.$i.dat
done

for ((i=1; i<=10; i++)) do
        echo "Extrahiere die Daten aus Durchlauf _du1 $i"
	    grep ^203 omnetpp_du1.$i.vec >wcli9_du1.$i.dat
	    grep ^158 omnetpp_du1.$i.vec >wcli0_du1.$i.dat
done

for ((i=1; i<=10; i++)) do
        echo "Extrahiere die Daten aus Durchlauf _du2 $i"
	    grep ^203 omnetpp_du2.$i.vec >wcli9_du2.$i.dat
	    grep ^158 omnetpp_du2.$i.vec >wcli0_du2.$i.dat
done

for ((i=1; i<=10; i++)) do
    echo "bilde gesamt compare Daten $i"
    rm compare_run.$i.dat
    grep ^203 omnetpp.$i.vec >>compare_run.$i.dat
    grep ^183 omnetpp.$i.vec >>compare_run.$i.dat
    grep ^193 omnetpp.$i.vec >>compare_run.$i.dat
    grep ^198 omnetpp.$i.vec >>compare_run.$i.dat
    grep ^188 omnetpp.$i.vec >>compare_run.$i.dat
    grep ^178 omnetpp.$i.vec >>compare_run.$i.dat
    grep ^173 omnetpp.$i.vec >>compare_run.$i.dat
    grep ^158 omnetpp.$i.vec >>compare_run.$i.dat
    grep ^163 omnetpp.$i.vec >>compare_run.$i.dat
done

cat wcli9_off_du3_7.dat wcli8_off_du3_7.dat wcli7_off_du3_7.dat
wcli6_off_du3_7.dat wcli5_off_du3_7.dat wcli4_off_du3_7.dat
wcli3_off_du3_7.dat wcli2_off_du3_7.dat wcli1_off_du3_7.dat
wcli0_off_du3_7.dat >>allOffRun7du3
### ...

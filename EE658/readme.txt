
compile:

g++ -o preprocess preprocess.cpp
g++ -o Dalg Dalg.cpp
g++ -o podem podem.cpp
g++ -o PFS PFS.cpp
g++ -o deductive deductive.cpp


// note here deductive can only support c17_ckt.txt

execute each program:
./preprocess <circuitname.ckt> <circuitname.faults>
./Dalg <circuitname.ckt> <circuitname.Dalg> <circuitname.faults>
./podem <circuitname.ckt> <circuitname.Dalg> <circuitname.faults>
./PFS <circuitname.ckt> <circuitname.Dalg> <circuitname.faults>



run overall:
**¡@need to change the circuit name¡@in overall.sh, then run
sh overall.sh 




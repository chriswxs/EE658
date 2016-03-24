g++ -o preprocess preprocess.cpp
g++ -o Dalg Dalg.cpp
g++ -o podem podem.cpp
g++ -o PFS PFS.cpp

./preprocess c17.ckt c17.faults
./Dalg c17.ckt c17.Dalg c17.faults
./PFS c17.ckt c17.Dalg c17.faults



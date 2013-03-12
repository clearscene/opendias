g++ -I. -I.. -Wall -g -O2 -c -o phash_tmp.o phash_tmp.cc
gcc -I. -I.. -Wall -g -O2 -c -o phash_test.o phash_test.c
echo compiled
g++  -g -O2 -o phash_test phash_tmp.o phash_test.o -llept -lpHash
echo OK
sudo ./phash_test

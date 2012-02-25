rm -f getScannersTest getScannersTest.o 
#gcc -I. -I../current_code/src/ -g -c -o getScannersTest.o getScannersTest.c
#g++ -g -o getScannersTest getScannersTest.o ../current_code/src/utils.o ../current_code/src/debug.o -lsane 
gcc -I. -g -c -o getScannersTest.o getScannersTest.c
g++ -g -o getScannersTest getScannersTest.o -lsane 
valgrind --leak-check=full --leak-resolution=high --error-limit=no --tool=memcheck --num-callers=50 --show-below-main=yes --track-origins=yes --track-fds=yes --show-reachable=yes ./getScannersTest

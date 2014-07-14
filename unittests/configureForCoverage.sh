
make clean
rm -rf test/results/coverage/*
rm -rf src/*.gcda src/*.gcno
./configure CFLAGS=' -g -O --coverage' LIBS='-lgcov'
make
echo Now run 'unittests/runForCoverage.sh'

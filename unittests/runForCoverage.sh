
rm -rf test/results/coverage/*
rm -rf src/*.gcda src/*.gcno
rm -rf unittests/*.gcda unittests/*.gcno
lcov -c -i -d src -o test/results/coverage/app_base.info
make check
lcov -c -d src -o test/results/coverage/app_test.info
lcov -a test/results/coverage/app_base.info -a test/results/coverage/app_test.info -o test/results/coverage/app_total.info 
genhtml -o test/results/coverage test/results/coverage/app_total.info 
echo Now check 'test/results/coverage/' for coverage results.

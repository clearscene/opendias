

#
# Cleanup
#
rm -rf src/*.gcda src/*.gcno test/valgrind.out test/lastCoverage test/lastRegression
mkdir test/lastCoverage/
mkdir test/lastRegression/


#
# Compile, so that 'code coverage' analysis reports can be generated.
#
echo Cleaning ...
make clean > test/lastRegression/buildLog.out
# if the file is here, then last time configure was run was in this script
# so no need to re-do it.
if [ ! -f dontBotherReCompilingMe-testScript ]; then
  echo Configuring ...
  ./configure CFLAGS='--coverage' LIBS='-lgcov' &> test/lastRegression/buildLog2.out
  # unfortunatly bash cannot support "&>>" - yet!
  cat test/lastRegression/buildLog2.out >> test/lastRegression/buildLog.out
  rm test/lastRegression/buildLog2.out
fi
# Will have to re-make, incase anything was changed in source.
echo Making ...
make &> test/lastRegression/buildLog2.out
# unfortunatly bash cannot support "&>>" - yet!
cat test/lastRegression/buildLog2.out >> test/lastRegression/buildLog.out
rm test/lastRegression/buildLog2.out
touch dontBotherReCompilingMe-testScript

#
# Generate baseline (coverage) information
#
echo Getting baseline coverage information ...
lcov -c -i -d src -o test/lastCoverage/app_base.info >> test/lastRegression/buildLog.out


#
# Start the app with memory management reporting enabled.
#
echo Creating startup scripts ...
CODENAME=`lsb_release -c | cut -f2`
SUPPRESS=""
for SUPP in `ls test/suppressions/$CODENAME/*`; do
  if [ -f $SUPP ]; then
    SUPPRESS="$SUPPRESS --suppressions=$SUPP"
  fi
done
VALGRINDOPTS="--leak-check=full --leak-resolution=high --error-limit=no --tool=memcheck --num-callers=50 --log-file=test/lastRegression/valgrind.out "
GENSUPP="--gen-suppressions=all "
echo touch runningTest > test/runMe
echo G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind $SUPPRESS $VALGRINDOPTS $GENSUPP src/opendias \&\> test/lastRegression/appLog.out >> test/runMe
echo rm runningTest >> test/runMe
chmod 755 test/runMe


#
# Backup the data area
#
echo Backing up personal data ...
DTE=`date +%Y%m%d%H%M%S`
tar -czf ~/openDias_bkp.$DTE.tar.gz ~/.openDIAS
rm -rf ~/.openDIAS



#######################################
# Run automated tests
echo Starting test harness ...
test/harness.sh $@
#######################################



#
# Recover data area
#
echo Restoring personal data ...
rm -rf ~/.openDIAS
OP=`pwd`
cd /
tar -xf ~/openDias_bkp.$DTE.tar.gz
cd $OP


#
# Collect process and build coverage report
#
echo Creating run coverage information ...
lcov -c -d src -o test/lastCoverage/app_test.info >> test/lastRegression/buildLog.out
lcov -a test/lastCoverage/app_base.info -a test/lastCoverage/app_test.info -o test/lastCoverage/app_total.info >> test/lastRegression/buildLog.out
genhtml -o test/lastCoverage test/lastCoverage/app_total.info >> test/lastRegression/buildLog.out



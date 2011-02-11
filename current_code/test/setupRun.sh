
usage() {
  echo "Usage: [-h] [-r] [-m] [-s] [-c] [<tests>]"
  echo "  -h       help (show this page)"
  echo "  -r       record results files (will distroy current results files of tests that are about to be run)"
  echo "  -m       don't do memory checking"
  echo "  -s       don't use any memory checking suppressions (exclusive to -m flag)"
  echo "  -c       don't do coverage checking"
  echo "  -b       don't rebuild app (may override -c)"
  echo "  <tests>  The tests you wish to run. Default to all tests."
  echo "             eg: 1*"
  echo "                 2_1 2_2 2_3"
  echo "                 2_1 3_4*"
}

#
# Parse off all the parameters
#

NOBUILD=""
RECORD=""
SKIPMEMORY=""
SKIPCOVERAGE=""
while getopts ":hrmcsb" flag 
do 
  case $flag in 
    h)
      usage;
      exit 1;
    ;;
    r)
      RECORD="-r";
      echo Will record results.
    ;;
    m)
      SKIPMEMORY="-m";
      echo Will skip analysing memory usage.
    ;;
    c)
      SKIPCOVERAGE="-c";
      echo Will skip analysing code coverage.
    ;;
    s)
      NOSUPPRESSIONS="-s";
    ;;
    b)
      NOBUILD="-b";
      echo Will not \(re\)build the app.
    ;;
  esac 
done 
if [ "$NOSUPPRESSIONS" != "" ]; then
  if [ "$SKIPMEMORY" == "" ]; then
    echo No memory suppression clauses will be used.
  else
    echo eek - you asked for no memory checking - and - use no memory supressions. This does not seam right.
    usage;
    exit 1;
  fi
fi
shift $((OPTIND-1))

if [ ! -f "/usr/bin/lcov" ]; then
  SKIPCOVERAGE="-c";
  echo "Will skip analysing code coverage (package not available)."
fi

# So that everything else does not have to run as root (for testing), reset back later
sudo chmod 757 /var/run
sudo rm -f /var/log/opendias/opendias.log

grep -q "^test" /etc/sane.d/dll.conf
if [ "$?" -ne "0" ]; then
  echo enable sane testing by commenting in "test" in "/etc/sane.d/dll.conf"
  exit
fi

#
# Cleanup
#
rm -rf ../src/*.gcda ../src/*.gcno results
mkdir -p results/coverage/
mkdir -p results/resultsFiles/


if [ "$NOBUILD" == "" ]; then
  #
  # Compile, so that 'code coverage' analysis reports can be generated.
  #
  echo Cleaning ...
  cd ../
  make clean > test/results/buildLog.out
  cd test

  # if the file is here, then last time configure was run was in this script
  # so no need to re-do it.
  echo -n Configuring 
  cd ../
  if [ "$SKIPCOVERAGE" == "-c" ]; then
    echo " (without coverage) ..."
    ./configure CFLAGS=' -g ' &> test/results/buildLog2.out
  else
    echo " (with coverage) ..."
    ./configure CFLAGS='--coverage' LIBS='-lgcov' &> test/results/buildLog2.out
  fi
  cd test
  # unfortunatly bash cannot support "&>>" - yet!
  cat results/buildLog2.out >> results/buildLog.out
  rm results/buildLog2.out

  # Will have to re-make, incase anything was changed in source.
  echo Making ...
  cd ../
  make &> test/results/buildLog2.out
  cd test
  # unfortunatly bash cannot support "&>>" - yet!
  cat results/buildLog2.out >> results/buildLog.out
  rm results/buildLog2.out

else
  echo Skipping the build process
fi


#
# Generate baseline (coverage) information
#
if [ "$SKIPCOVERAGE" == "" ]; then
  echo Getting baseline coverage information ...
  lcov -c -i -d ../src -o results/coverage/app_base.info >> results/buildLog.out
fi


#
# Start the app with memory management reporting enabled.
#
echo Creating startup scripts ...
if [ "$SKIPMEMORY" == "" ]; then
  #CODENAME=`lsb_release -c | cut -f2`
  SUPPRESS=""
  #if [ -d config/suppressions/$CODENAME ]; then
  #  for SUPP in `ls config/suppressions/$CODENAME/*`; do
  if [ -d suppressions ]; then
    for SUPP in `ls suppressions/*`; do
      if [ -f $SUPP ]; then
        SUPPRESS="$SUPPRESS --suppressions=$SUPP"
      fi
    done
  fi
  VALGRINDOPTS="--leak-check=full --leak-resolution=high --error-limit=no --tool=memcheck --num-callers=50 --log-file=results/resultsFiles/valgrind.out "
  GENSUPP="--gen-suppressions=all "
  VALGRIND="G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind "
#else
#  VALGRIND="strace "
fi

PWD=`pwd`
echo $VALGRIND $SUPPRESS $VALGRINDOPTS $GENSUPP ../src/opendias -c $PWD/config/testapp.conf \> results/resultsFiles/appLog.out > config/startAppCommands



#######################################
#######################################
# Run automated tests
echo Starting test harness ...
perl ./harness.pl $RECORD $SKIPMEMORY $@ 2> /dev/null
#######################################
#######################################


##
## Recover data area
##
#if [ -f conf/openDias_bkp.$DTE.tar.gz ]; then
#  echo Restoring personal data ...
#  rm -rf ~/.openDIAS
#  OP=`pwd`
#  cd /
#  tar -xf conf/openDias_bkp.$DTE.tar.gz
#  cd $OP
#fi


#
# Collect process and build coverage report
#
if [ "$SKIPCOVERAGE" == "" ]; then
  echo Creating run coverage information ...
  echo Creating run coverage information ... >> results/buildLog.out
  lcov -c -d ../src -o results/coverage/app_test.info >> results/buildLog.out
  lcov -a results/coverage/app_base.info -a results/coverage/app_test.info -o results/coverage/app_total.info >> results/buildLog.out
  genhtml -o results/coverage results/coverage/app_total.info >> results/buildLog.out
fi

#rm config/startAppCommands

sudo chmod 755 /var/run



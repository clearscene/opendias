#!/bin/bash

usage() {
  echo "Usage: [-h] [-r] [-m] [-s] [-c] [<tests>]"
  echo "  -h       help (show this page)"
  echo "  -r       record results files (will distroy current results files of tests that are about to be run)"
  echo "  -m       don't do memory checking"
  echo "  -s       don't use any memory checking suppressions (exclusive to -m flag)"
  echo "  -c       don't do coverage checking"
  echo "  -b       don't rebuild app (may override -c)"
  echo "  -g       show a visual debugger for the webclient"
  echo "  <tests>  The tests you wish to run. Default to all tests."
  echo "             eg: 5*"
  echo "                 01* 021* 022*"
  echo "                 006_scan"
}

#
# Parse off all the parameters
#

INSTALLLOCATION="/tmp/opendias_test"
NOBUILD=""
RECORD=""
SKIPMEMORY=""
SKIPCOVERAGE=""
GRAPHICALCLIENT=""
while getopts ":hrmcsbg" flag 
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
    g)
      GRAPHICALCLIENT="-g";
      echo The web client will have a graphical debugger frontend
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

# Check we don't have a running service atm.
ps -ef | grep -v "grep" | grep "bin/opendias"
if [ "$?" -eq "0" ]; then
  echo "It looks like the opendias service is already running. Please stop before running testing."
  exit
fi
rm -f /tmp/opendias_test/var/log/opendias/opendias.log

#
# Cleanup
#
if [ "$NOBUILD" == "" ]; then
  rm -rf results
  rm -rf ../src/*.gcda ../src/*.gcno
  mkdir -p results/coverage/
else
  # If we're not building the source, save the coverage information
  rm -f results/buildLog.out
  rm -rf results/resultsFiles
fi

mkdir -p results/resultsFiles/

if [ "$NOBUILD" == "" ]; then
  #
  # Compile, so that 'code coverage' analysis reports can be generated.
  #
  echo Cleaning ...
  cd ../
  make clean &> test/results/buildLog2.out
  cd test

  which apt-rdepends > /dev/null
  if [ "$?" -ne "0" ]; then
    echo Could not determine the installed packages. If you\'re on debian based system, install apt-rdepends
  else
    echo Recording current installed package versions off all dependencies
    dpkg -l `apt-rdepends build-essential libsqlite3-dev libsane-dev libmicrohttpd-dev uuid-dev libleptonica-dev libpoppler-cpp-dev libtesseract-dev libxml2-dev libzzip-dev libarchive-dev 2> /dev/null | grep -v "^ " | sort` &> results/buildLog2.out
    # unfortunatly bash cannot support "&>>" - yet!
    cat results/buildLog2.out >> results/buildLog.out
    rm results/buildLog2.out
  fi

  echo Performing code analysis ...
  cd ../
  mv config.h config.h.orig
  cp config.h.for_cppcheck config.h
  cppcheck --verbose --enable=all --max-configs=999 --platform=unix32 --platform=unix64 --suppress=missingIncludeSystem --error-exitcode=1 src/ &> test/results/buildLog2.out
  if [ "$?" -ne "0" ]; then
    echo "Code analysis found a problem. Check the buildLog.out for details."
    rm config.h 
    mv config.h.orig config.h
    cd test
    # unfortunatly bash cannot support "&>>" - yet!
    cat results/buildLog2.out >> results/buildLog.out
    rm results/buildLog2.out
    #exit
  fi
  mv config.h config.h.for_cppcheck
  mv config.h.orig config.h
  cd test
  # unfortunatly bash cannot support "&>>" - yet!
  cat results/buildLog2.out >> results/buildLog.out
  rm results/buildLog2.out

  # if the file is here, then last time configure was run was in this script
  # so no need to re-do it.
  echo -n Configuring 
  cd ../
  if [ "$SKIPCOVERAGE" == "-c" ]; then
    echo " (without coverage) ..."
    ./configure --prefix=$INSTALLLOCATION --enable-create_test_language --enable-werror -C CFLAGS=' -g -O ' &> test/results/buildLog2.out
  else
    echo " (with coverage) ..."
    ./configure --prefix=$INSTALLLOCATION --enable-create_test_language --enable-werror -C CFLAGS=' -g -O --coverage' LIBS='-lgcov' &> test/results/buildLog2.out
  fi
  cd test
  # unfortunatly bash cannot support "&>>" - yet!
  cat results/buildLog2.out >> results/buildLog.out
  rm results/buildLog2.out

  # Will have to re-make, incase anything was changed in source.
  echo Making ...
  cd ../
  make &> test/results/buildLog2.out
  if [ "$?" -ne "0" ]; then
    echo "Compile stage failed. Check the buildLog.out for details."
    cd test
    # unfortunatly bash cannot support "&>>" - yet!
    cat results/buildLog2.out >> results/buildLog.out
    rm results/buildLog2.out
    exit
  fi
  cd test
  # unfortunatly bash cannot support "&>>" - yet!
  cat results/buildLog2.out >> results/buildLog.out
  rm results/buildLog2.out

  # Install the app into the configured test location.
  echo Installing ...
  cd ../
  make install &> test/results/buildLog2.out
  if [ "$?" -ne "0" ]; then
    echo "Install stage failed. Check the buildLog.out for details."
    cd test
    # unfortunatly bash cannot support "&>>" - yet!
    cat results/buildLog2.out >> results/buildLog.out
    rm results/buildLog2.out
    exit
  fi
  cd test
  # unfortunatly bash cannot support "&>>" - yet!
  cat results/buildLog2.out >> results/buildLog.out
  rm results/buildLog2.out

  echo Creating testing \(override\) libs...
  cd override_libs
  ./build.sh
  cd ../

else
  echo Skipping the build process
fi


#
# Generate baseline (coverage) information
#
if [ "$SKIPCOVERAGE" == "" ]; then
  if [ "$NOBUILD" == "" ]; then
    echo Getting baseline coverage information ...
    lcov -c -i -d ../src -o results/coverage/app_base.info >> results/buildLog.out
  fi
fi


#
# Start the app with memory management reporting enabled.
#
echo Creating startup scripts ...
if [ "$SKIPMEMORY" == "" ]; then
  PWD=`pwd`;
  SUPPRESS=""
  if [ "$NOSUPPRESSIONS" == "" ]; then
    if [ -d suppressions ]; then
      for SUPP in `ls suppressions/*`; do
        if [ -f $SUPP ]; then
          SUPPRESS="$SUPPRESS --suppressions=${PWD}/$SUPP"
        fi
      done
    fi
  fi
  VALGRINDOPTS="--leak-check=full --leak-resolution=high --error-limit=no --tool=memcheck --num-callers=50 --log-file=${PWD}/results/resultsFiles/valgrind.out.%p --track-origins=yes --track-fds=yes --show-reachable=yes --trace-children=yes "
  # --trace-syscalls=yes 
  GENSUPP="--gen-suppressions=all "
  VALGRIND="valgrind "
#  VALGRIND="strace -f -o results/resultsFiles/valgrind.out "
fi


#
# Use testing sane config (so testers do not have to update their environment)
#
mkdir -p /tmp/opendiassaneconfig
cp config/sane/* /tmp/opendiassaneconfig/
export SANE_CONFIG_DIR=/tmp/opendiassaneconfig/

echo /tmp/opendiastest > $INSTALLLOCATION/etc/opendias/opendias.conf

echo $VALGRIND $SUPPRESS $VALGRINDOPTS $GENSUPP $INSTALLLOCATION/bin/opendias \> results/resultsFiles/appLog.out > config/startAppCommands



#######################################
#######################################
# Run automated tests
echo Starting test harness ...
perl ./harness.pl -z $GRAPHICALCLIENT $RECORD $SKIPMEMORY $@ 2> /dev/null
#######################################
#######################################


#
# Collect process and build coverage report
#
if [ "$SKIPCOVERAGE" == "" ]; then
  echo Creating total coverage report ...
  echo Creating total coverage report... >> results/buildLog.out
  lcov -c -d ../src -o results/coverage/app_test.info >> results/buildLog.out
  lcov -a results/coverage/app_base.info -a results/coverage/app_test.info -o results/coverage/app_total.info >> results/buildLog.out
  genhtml -o results/coverage results/coverage/app_total.info >> results/buildLog.out
fi

#rm config/startAppCommands


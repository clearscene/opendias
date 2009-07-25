#!/bin/bash

# Setup
DATE=`date +%Y-%b-%d_%H:%M`
echo -en "<html><style>.none {background-color: none}\n .ok {background-color: #ccFFcc;}\n .fail {background-color: #FFCCCC;}</style><body><h1>Regression Tests, last run: $DATE</h1><h2>Results</h2>\n" > test/lastRegression/index.html
echo -en "<table cellpadding=1 cellspacing=1 border=1><tr><th>Result</th><th>Test</th><th colspan=2>Memory</th><th colspan=2>Test Log</th><th colspan=2>App Log</th></tr>\n" >> test/lastRegression/index.html

runTests="";
testCount="";
passCount="";
failCount="";
outputDir="test/lastRegression";
PYTHONPATH="test/regressionTests/";

RECORD=""
SKIPMEMORY=""
while getopts ":rm" flag 
do
  case $flag in
#    h)
#      usage;
#      exit 1;
#    ;;
    r)
      RECORD="-r";
      GENERATE="Y"
    ;;
    m)
      SKIPMEMORY="-m";
    ;;
  esac
done

shift $((OPTIND-1))
GIVEN=$@


# Format the request, add a generic "all", if nothing was given.
for requested in $GIVEN; do
  runTests="$runTests '$requested' "
done
if [ "$runTests" == "" ]; then
  runTests="'*'"
fi

# Loop over each request.
for requested in $runTests; do
  # Find all tests that match this request (eg. "1*", given "1_1_General" and "1_2_GeneralFiles")
  for TEST in `eval find test/regressionTests/ -maxdepth 1 -type f -name $requested | grep -v utilsLib.py | grep -v "cvsignore" | sort`; do
    # Create results area and cleanup any old test results
    mkdir -p $outputDir/$TEST
    rm -rf $outputDir/$TEST/*
    rm -rf /tmp/ldtp-$USER/*

    # Create fixed startup environment
    # backup current environment
    if [ -d ~/.openDIAS/ ]; then
      echo Too hot for me! directory $HOME/.openDIAS exists. Cowidly resusing to destroy it.
      exit
    fi

    # Build new environment
    if [ -f $TEST.inputs/homeDir ]; then
      cp -r $TEST.inputs/homeDir/* ~/.openDIAS/
    fi

    # Reset test result vars
    RES=0;
    MEM_RES=""
    TEST_RES=""
    APP_RES=""

    # Run the test
    echo Running ... $TEST
    python $TEST

    # Check for test crash
    if [ "$?" == "0" ]; then

      # Wait until the child process has finished.
      while [ -f runningTest ]
      do
        sleep 1
      done

      # memory log
      if [ $SKIPMEMORY == "" ]; then
        mv $outputDir/valgrind.out $outputDir/$TEST/valgrind.out
        # parse out changeable content
        sed -f test/valgrindUnify.sed < $outputDir/$TEST/valgrind.out > $outputDir/$TEST/valgrind4Compare.out
        if [ "$GENERATE" == "Y" ]; then
          if [ ! -e ${TEST}.result ]; then
            mkdir -p ${TEST}.result
          fi
          cp $outputDir/$TEST/valgrind4Compare.out ${TEST}.result/valgrind.out
        fi
        MEM_RES="<td class='none'><a href='$TEST/valgrind.out'>actual</a></td>"
        diff -ydN ${TEST}.result/valgrind.out $outputDir/$TEST/valgrind4Compare.out > $outputDir/$TEST/valgrindDiff.out
        if [ "$?" == "0" ]; then
          rm $outputDir/$TEST/valgrindDiff.out
          MEM_RES="$MEM_RES<td class='ok'>OK</td>"
        else
          MEM_RES="$MEM_RES<td><a href='$TEST/valgrindDiff.out'>diff</a>&nbsp;|&nbsp;<a href='../../${TEST}.result/valgrind.out'>expected</a></td>"
          RES=1
        fi
      else
        MEM_RES="<td colspan=2 class='none'>-- SKIPPED --</td>"
      fi

      # test log
      mv /tmp/ldtp-$USER/* $outputDir/$TEST/
      cp $outputDir/$TEST/*.xml $outputDir/$TEST/index.out
      if [ "$GENERATE" == "Y" ]; then
        if [ ! -e ${TEST}.result ]; then
          mkdir -p ${TEST}.result
        fi
        cp $outputDir/$TEST/index.out ${TEST}.result/index.out
      fi
      TEST_RES="<td class='none'><a href='$TEST/index.out'>actual</a></td>"
      diff -ydN ${TEST}.result/index.out $outputDir/$TEST/index.out > $outputDir/$TEST/testLogDiff.out
      if [ "$?" == "0" ]; then
        rm $outputDir/$TEST/testLogDiff.out
        TEST_RES="$TEST_RES<td class='ok'>OK</td>"
      else
        TEST_RES="$TEST_RES<td><a href='$TEST/testLogDiff.out'>diff</a>&nbsp;|&nbsp;<a href='../../${TEST}.result/index.out'>expected</a></td>"
        RES=1
      fi

      # app log
      cp $outputDir/appLog.out $outputDir/$TEST/appLog.out
      if [ "$GENERATE" == "Y" ]; then
        if [ ! -e ${TEST}.result ]; then
          mkdir -p ${TEST}.result
        fi
        cp $outputDir/$TEST/appLog.out ${TEST}.result/appLog.out
      fi
      APP_RES="<td class='none'><a href='$TEST/appLog.out'>actual</a></td>"
      diff -ydN ${TEST}.result/appLog.out $outputDir/$TEST/appLog.out > $outputDir/$TEST/appLogDiff.out
      if [ "$?" == "0" ]; then
        rm $outputDir/$TEST/appLogDiff.out
        APP_RES="$APP_RES<td class='ok'>OK</td>"
      else
        APP_RES="$APP_RES<td><a href='$TEST/appLogDiff.out'>diff</a>&nbsp;|&nbsp;<a href='../../${TEST}.result/appLog.out'>expected</a></td>"
        RES=1
      fi

      # Collate results
      if [ "$RES" == "0" ]; then
        echo -en "<tr>" >> $outputDir/index.html
        echo -en "<td class='ok'>PASS</td>" >> $outputDir/index.html
        passCount="$passCount."
      else
        echo -en "<tr class='fail'>" >> $outputDir/index.html
        echo -en "<td>FAIL</td>" >> $outputDir/index.html
        failCount="$failCount."
      fi

    else
      echo -en "<tr class='fail'>" >> $outputDir/index.html
      echo -en "<td>CRASH</td>" >> $outputDir/index.html
      TEST_RES="<td colspan=6>&nbsp;</td>"
      failCount="$failCount."
    fi

    # Output result line
    echo "<td><a href='$TEST/'>$TEST</a></td>" >> $outputDir/index.html
    echo $MEM_RES >> $outputDir/index.html
    echo $TEST_RES >> $outputDir/index.html
    echo $APP_RES >> $outputDir/index.html
    testCount="$testCount."

    echo -en "</tr>" >> $outputDir/index.html

    # Restore users real environment
    rm -rf ~/.openDIAS

  done
done
echo -en "</table>" >> $outputDir/index.html

# Tot up the totals
testCount=`echo -n $testCount | wc -m`
passCount=`echo -n $passCount | wc -m`
failCount=`echo -n $failCount | wc -m`
echo -en "<hr /><p><strong>Tests: $testCount   Passed: $passCount     Failed: $failCount </strong></p>" >> $outputDir/index.html

# Finish off
echo -en "</body></html>\n" >> $outputDir/index.html



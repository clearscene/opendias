#!/bin/bash

# Setup
DATE=`date +%Y-%b-%d_%H:%M`
runTests="";
testCount="";
passCount="";
failCount="";
outputDir="results/resultsFiles";
PYTHONPATH="regressionTests\/";
echo -en "<html><style>.none {background-color: none}\n .ok {background-color: #ccFFcc;}\n .fail {background-color: #FFCCCC;}</style><body><h1>Regression Tests, last run: $DATE</h1><h2>Results</h2>\n" > $outputDir/index.html
echo -en "<table cellpadding=1 cellspacing=1 border=1><tr><th>Result</th><th>Test</th><th colspan=2>Memory</th><th colspan=2>Test Log</th><th colspan=2>App Log</th></tr>\n" >> $outputDir/index.html


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
  for TEST in `eval find $PYTHONPATH -maxdepth 1 -type f -name $requested | sort`; do
    # Create results area and cleanup any old test results
    TESTCASENAME=`echo "$TEST" | sed -e "s/$PYTHONPATH//" `
    mkdir -p $outputDir/$TESTCASENAME
    rm -rf $outputDir/$TESTCASENAME/*
    rm -rf /tmp/ldtp-$USER/*

    # Create fixed startup environment
    # backup current environment
    if [ -d ~/.openDIAS/ ]; then
      echo Too hot for me! directory $HOME/.openDIAS exists. Cowidly refusing to destroy it.
      exit
    fi

    # Build new environment
    if [ -f $PYTHONPATH/inputs/$TESTCASENAME/homeDir ]; then
      cp -r $PYTHONPATH/inputs/$TESTCASENAME/homeDir ~/.openDIAS
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
      if [ "$SKIPMEMORY" == "" ]; then
        mv $outputDir/valgrind.out $outputDir/$TESTCASENAME/valgrind.out
        # parse out changeable content
        sed -f config/valgrindUnify.sed < $outputDir/$TESTCASENAME/valgrind.out > $outputDir/$TESTCASENAME/valgrind4Compare.out
        if [ "$GENERATE" == "Y" ]; then
          if [ ! -e $PYTHONPATH/results/$TESTCASENAME ]; then
            mkdir -p $PYTHONPATH/results/$TESTCASENAME
          fi
          cp $outputDir/$TESTCASENAME/valgrind4Compare.out $PYTHONPATH/results/$TESTCASENAME/valgrind.out
        fi
        MEM_RES="<td class='none'><a href='$TESTCASENAME/valgrind.out'>actual</a></td>"
        diff -ydN $PYHTONPATH/results/$TESTCASENAME/valgrind.out $outputDir/$TESTCASENAME/valgrind4Compare.out > $outputDir/$TESTCASENAME/valgrindDiff.out
        if [ "$?" == "0" ]; then
          rm $outputDir/$TESTCASENAME/valgrindDiff.out
          MEM_RES="$MEM_RES<td class='ok'>OK</td>"
        else
          MEM_RES="$MEM_RES<td><a href='$TESTCASENAME/valgrindDiff.out'>diff</a>&nbsp;|&nbsp;<a href='../../$PYTHONPATH/results/$TESTCASENAME/valgrind.out'>expected</a></td>"
          RES=1
        fi
      else
        MEM_RES="<td colspan=2 class='none'>-- SKIPPED --</td>"
      fi

      # test log
      mv /tmp/ldtp-$USER/* $outputDir/$TESTCASENAME/
      cp $outputDir/$TESTCASENAME/*.xml $outputDir/$TESTCASENAME/index.out
      if [ "$GENERATE" == "Y" ]; then
        if [ ! -e $PYTHONPATH/results/$TESTCASENAME ]; then
          mkdir -p $PYTHONPATH/results/$TESTCAENAME
        fi
        cp $outputDir/$TESTCASENAME/index.out $PYTHONPATH/results/$TESTCASENAME/index.out
      fi
      TEST_RES="<td class='none'><a href='$TESTCASENAME/index.out'>actual</a></td>"
      diff -ydN $PYTHONPATH/results/$TESTCASENAME/index.out $outputDir/$TESTCASENAME/index.out > $outputDir/$TESTCASENAME/testLogDiff.out
      if [ "$?" == "0" ]; then
        rm $outputDir/$TESTCASENAME/testLogDiff.out
        TEST_RES="$TEST_RES<td class='ok'>OK</td>"
      else
        TEST_RES="$TEST_RES<td><a href='$TESTCASENAME/testLogDiff.out'>diff</a>&nbsp;|&nbsp;<a href='../../$PYHTONPATH/results/$TESTCASENAME/index.out'>expected</a></td>"
        RES=1
      fi

      # app log
      cp $outputDir/appLog.out $outputDir/$TESTCASENAME/appLog.out
      if [ "$GENERATE" == "Y" ]; then
        if [ ! -e $PYTHONPATH/results/$TESTCASENAME ]; then
          mkdir -p $PYTHONPATH/results/$TESTCAENAME
        fi
        cp $outputDir/$TESTCASENAME/appLog.out $PYTHONPATH/results/$TESTCASENAME/appLog.out
      fi
      APP_RES="<td class='none'><a href='$TESTCASENAME/appLog.out'>actual</a></td>"
      diff -ydN $PYTHONPATH/results/$TESTCASENAME/appLog.out $outputDir/$TESTCASENAME/appLog.out > $outputDir/$TESTCASENAME/appLogDiff.out
      if [ "$?" == "0" ]; then
        rm $outputDir/$TESTCASENAME/appLogDiff.out
        APP_RES="$APP_RES<td class='ok'>OK</td>"
      else
        APP_RES="$APP_RES<td><a href='$TESTCASENAME/appLogDiff.out'>diff</a>&nbsp;|&nbsp;<a href='../../$PYTHONPATH/results/$TESTCASENAME/appLog.out'>expected</a></td>"
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
    echo "<td><a href='$TESTCASENAME/'>$TESTCASENAME</a></td>" >> $outputDir/index.html
    echo $MEM_RES >> $outputDir/index.html
    echo $TEST_RES >> $outputDir/index.html
    echo $APP_RES >> $outputDir/index.html
    testCount="$testCount."

    echo -en "</tr>" >> $outputDir/index.html

    # Cleanup
    rm -rf ~/.openDIAS

  done
done
echo -en "</table>" >> $outputDir/index.html

# Tot up the totals
testCount=`echo -n $testCount | wc -m`
passCount=`echo -n $passCount | wc -m`
failCount=`echo -n $failCount | wc -m`
echo -en "<hr /><p><strong>Tests: $testCount<br />Passed: $passCount<br />Failed: $failCount </strong></p>" >> $outputDir/index.html

# Finish off
echo -en "</body></html>\n" >> $outputDir/index.html



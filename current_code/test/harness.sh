#!/bin/bash

# Setup
DATE=`date +%Y-%b-%d_%H:%M`
echo -en "<html><body><h1>Regression Tests, last run: $DATE</h1><h2>Results</h2>\n" > test/lastRegression/index.html
runTests="";
testCount="";
passCount="";
failCount="";
outputDir="test/lastRegression";

# Are we generating the results files?
FIRST=`echo $@ | cut -f1 -d' '`
if [ "$FIRST" == "-r" ]; then
  GENERATE="Y"
  echo Generating result files.
  GIVEN=`echo $@ | cut -b 3-`
else
  GIVEN=$@
fi

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
  for TEST in `eval find test/regressionTests/ -maxdepth 1 -type f -name $requested `; do
    # Create results area and cleanup any old test results
    mkdir -p $outputDir/$TEST
    rm -rf $outputDir/$TEST/*
    rm -rf /tmp/ldtp-$USER/*
    DIFF=""
    ALL=""

    # Run the test
    echo Running ... $TEST
    python $TEST

    # Count the result
    if [ "$?" == "0" ]; then
      mv /tmp/ldtp-$USER/* $outputDir/$TEST/
      cp $outputDir/$TEST/*.xml $outputDir/$TEST/index.out

      if [ "$GENERATE" == "Y" ]; then
        if [ ! -e ${TEST}.result ]; then
          mkdir -p ${TEST}.result
        fi
        cp $outputDir/$TEST/index.out ${TEST}.result/index.out
      fi

      diff -ydN ${TEST}.result/index.out $outputDir/$TEST/index.out > $outputDir/$TEST/diff.out

      if [ "$?" == "0" ]; then
        echo -en "<p>1 ......" >> $outputDir/index.html
        rm $outputDir/$TEST/diff.out
        passCount="$passCount."
      else
        echo -en "<p>0 ......" >> $outputDir/index.html
        DIFF="&nbsp;[<a href='$TEST/diff.out'>diff</a>]"
        failCount="$failCount."
      fi
      ALL="&nbsp;[<a href='$TEST/'>all</a>]"

    else
      echo -en "<p>Crash .." >> $outputDir/index.html
      failCount="$failCount."
    fi
    echo " <a href='$TEST/index.out'>$TEST</a>${DIFF}&nbsp;[<a href='../../${TEST}.result/index.out'>expected</a>]${ALL}</p>" >> $outputDir/index.html
    testCount="$testCount."

  done
done

# Tot up the totals
testCount=`echo -n $testCount | wc -m`
passCount=`echo -n $passCount | wc -m`
failCount=`echo -n $failCount | wc -m`
echo -en "<hr /><p><strong>Tests: $testCount   Passed: $passCount     Failed: $failCount </strong></p>" >> $outputDir/index.html

# Finish off
echo -en "</body></html>\n" >> $outputDir/index.html



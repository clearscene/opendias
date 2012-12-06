#!/usr/bin/perl -w

use lib qw( regressionTests regressionTests/lib );
use Getopt::Std;
use Time::HiRes qw( gettimeofday tv_interval );
use standardTests;

$| = 1;

# Setup
my $DATE=`date +%Y-%b-%d_%H:%M`;
my $pwd=`pwd`;
my $runTests="";
my $testCount=0;
my $passCount=0;
my $failCount=0;
my $outputDir="results/resultsFiles";
my $TESTPATH="regressionTests";
my $startCmd=`cat config/startAppCommands`;

##############################
# Read Ops
my $GENERATE="";
my $SKIPMEMORY="";
our ($opt_z, $opt_r, $opt_m, $opt_g, );
getopts('zrmg');

# An unpublished flag to ensure we're only called from setupRun.sh
die "You probably wanted to run ./setupRun.sh. Spotted" unless $opt_z;

if( defined $opt_r ) {
  $GENERATE="(GENERATED)"
}
if( defined $opt_m) {
  $SKIPMEMORY="-m";
}


##################################
# Generate the page header
#
open(OUTPUTINDEX, ">$outputDir/index.html") or die "Cannot open output file $outputDir/index.html. Because: $!\n";

print OUTPUTINDEX <<EOS;
<html>
  <style>
    .none {background-color: none}
    .ok {background-color: #ccFFcc;}
    .fail {background-color: #FFCCCC;}
  </style>
  <body>
    <h1>Regression Tests, last run: $DATE</h1>
    <h2>Results $GENERATE</h2>
    <table cellpadding=1 cellspacing=1 border=1>
      <tr>
        <th>Result</th>
        <th>Test</th>
        <th>Time</th>
        <th colspan=2>Memory</th>
        <th colspan=2>Test Log</th>
        <th colspan=2>App Log</th>
      </tr>
EOS




####################################
# Format the request, add a generic "all", if nothing was given.
@runTests = @ARGV;
push(@runTests, "''") unless @runTests;

# Loop over each request.
for my $requested (@runTests) {

  $requested .= '*'; # So we only have to specify the prefix

  # Find all tests that match this request (eg. "1*", given "1_1_General" and "1_2_GeneralFiles")
  my $found = "find $TESTPATH -maxdepth 1 -type f -name $requested.pm | sort";
  $found = `$found`;
  for my $TEST (split(" ", $found) ) {

    printProgressLine($TEST, "Setting up environment");

    # Create results area and cleanup any old test results
    my ($TESTCASENAME) = $TEST =~ /$TESTPATH\/(.*)\.pm/;

    $standardTests::testpath = $TESTPATH;
    $standardTests::testcasename = $TESTCASENAME;

    system("rm -rf $outputDir/$TESTCASENAME/*");
    system("rm -rf /tmp/opendiastest");
    system("mkdir -p $outputDir/$TESTCASENAME");
    system("mkdir -p /tmp/opendiastest");

    openlog( "$outputDir/$TESTCASENAME/testLog.out" );

    unless( -e "$TESTPATH/expected/$TESTCASENAME" ) {
      system("mkdir -p $TESTPATH/expected/$TESTCASENAME");
    }

    # Build new environment
    if( -d "$TESTPATH/inputs/$TESTCASENAME" ) {
      if( -d "$TESTPATH/inputs/$TESTCASENAME/homeDir" ) {
        o_log("Copying pre defined environment");
        system("cp -r $TESTPATH/inputs/$TESTCASENAME/homeDir/* /tmp/opendiastest/");
      }
      if( -e "/tmp/opendiastest/DEFAULTDB" ) {
        o_log("Copying default database");
        system("cp -r config/defaultdatabase.sqlite3 /tmp/opendiastest/openDIAS.sqlite3");
        system("rm -f /tmp/opendiastest/DEFAULTDB");
      }
      opendir(DIR, "$TESTPATH/inputs/$TESTCASENAME/" ) or die "Cannot read SQL directory $TESTPATH/inputs/$TESTCASENAME/, because: $!\n";
      while( ($filename = readdir(DIR))) {
        next if ( $filename eq "." || $filename eq ".." );
        my $fullPath = "$TESTPATH/inputs/$TESTCASENAME/".$filename;
        if ( -f $fullPath && $fullPath =~ /\.sql$/ ) {
          system("/usr/bin/sqlite3 /tmp/opendiastest/openDIAS.sqlite3 \".read $fullPath\""); 
        }
      }
      closedir(DIR);
    }
    ## We want to update the location of any outputs (from the app default)
    #system("/usr/bin/sqlite3 /tmp/opendiastest/openDIAS.sqlite3 \"UPDATE config SET config_option='/tmp/opendiastest' WHERE config_value='scan_directory'\""); 

    # Reset test result vars
    my $RES=0;
    my $MEM_RES="";
    my $TEST_RES="";
    my $APP_RES="";
    my $testTime = 0;

    eval ( "require '$TEST' " );

    if($@) {

      printProgressLine($TEST, "Crashed!\n");
      print OUTPUTINDEX "<tr class='fail'><td>CRASH</td>";
      $TEST_RES="<td colspan=6>$@</td>";
      $failCount++;

    }

    else {

      my $startCommand = $startCmd;

      my $testProfile = eval("regressionTests::".$TESTCASENAME."::testProfile();");

      if( $testProfile->{updateStartCommand} ) {
        eval "regressionTests::${TESTCASENAME}::".$testProfile->{updateStartCommand}."(\\\$startCommand)";
      }


      # Start opendias
      if( $testProfile->{valgrind} && $testProfile->{valgrind} == 1 ) {
        printProgressLine($TEST, "Starting service (valgrind)");
      }
      else {
        printProgressLine($TEST, "Starting service");
        $startCommand =~ s{valgrind.*\s([a-z_\/]*/bin/opendias)}{$1}xms;
        o_log("No need for valgrind on this test.");
      }
      $RES = 1 unless startService( $startCommand, $testProfile->{startTimeout} );


      # Start a web client
      unless( $RES ) {
        printProgressLine($TEST,  "Starting client");
        if ( $testProfile->{client} && $testProfile->{client} == 1 ) {
          o_log("Asked to start a web clinet.");
          $RES = 1 unless setupClient($opt_g);
        }
        else {
          o_log("No need for a web client for this test.");
        }
      }


      # Run the test
      unless( $RES ) {
        printProgressLine($TEST, "Running");
        my $t0 = [gettimeofday];
        eval "\$RES = regressionTests::".$TESTCASENAME."::test()";
        $testTime = sprintf("%.2f", tv_interval( $t0, [gettimeofday]) );
        o_log("Error while running test: $@") if ($@);
        printProgressLine($TEST, "Stopping");
        stopService();
      }


      ############
      # memory log - output from valgrind
      unless( length $SKIPMEMORY ) {
        if( -f "$outputDir/valgrind.out" ) {
          # copy and parse out changeable content
          system("mv $outputDir/valgrind.out $outputDir/$TESTCASENAME/valgrind.out");
          system("sed -f config/valgrindUnify.sed < $outputDir/$TESTCASENAME/valgrind.out > $outputDir/$TESTCASENAME/valgrind4Compare.out");

          # Make this the expected, if required
          if( length $GENERATE ) {
            system("cp $outputDir/$TESTCASENAME/valgrind4Compare.out $TESTPATH/expected/$TESTCASENAME/valgrind.out");
          }

          $MEM_RES="<td class='none'><a href='./$TESTCASENAME/valgrind.out'>actual</a></td>";
          system("diff -ydN $TESTPATH/expected/$TESTCASENAME/valgrind.out $outputDir/$TESTCASENAME/valgrind4Compare.out > $outputDir/$TESTCASENAME/valgrindDiff.out");
          if($? >> 8 == 0) {
            system("rm $outputDir/$TESTCASENAME/valgrindDiff.out");
            $MEM_RES .= "<td class='ok'>OK</td>";
          }
          else {
            $MEM_RES .= "<td><a href='./$TESTCASENAME/valgrindDiff.out'>diff</a>&nbsp;|&nbsp;<a href='../../$TESTPATH/expected/$TESTCASENAME/valgrind.out'>expected</a></td>";
            $RES=1
          }
        }
        else {
          $MEM_RES="<td colspan=2 class='none'>-- NA --</td>"
        }
      }
      else {
        $MEM_RES="<td colspan=2 class='none'>-- SKIPPED --</td>"
      }

      ##########
      # test log - output from the testing harness
      system("sed -f config/testLogUnify.sed < $outputDir/$TESTCASENAME/testLog.out > $outputDir/$TESTCASENAME/testLog4Compare.out");
      if( length $GENERATE ) {
        system("cp $outputDir/$TESTCASENAME/testLog4Compare.out $TESTPATH/expected/$TESTCASENAME/testLog.out");
      }
      $TEST_RES="<td class='none'><a href='./$TESTCASENAME/testLog.out'>actual</a></td>";
      system("diff -ydN $TESTPATH/expected/$TESTCASENAME/testLog.out $outputDir/$TESTCASENAME/testLog4Compare.out > $outputDir/$TESTCASENAME/testLogDiff.out");
      if( $? >> 8 == 0 ) {
        system("rm $outputDir/$TESTCASENAME/testLogDiff.out");
        $TEST_RES .= "<td class='ok'>OK</td>";
      }
      else {
        $TEST_RES .= "<td><a href='./$TESTCASENAME/testLogDiff.out'>diff</a>&nbsp;|&nbsp;<a href='../../$TESTPATH/expected/$TESTCASENAME/testLog.out'>expected</a></td>";
        $RES=1;
      }

      #########
      # app log - output from the app (stdout + log)
      system("echo STDOUT > $outputDir/$TESTCASENAME/appLog.out");
      system("cat $outputDir/appLog.out >> $outputDir/$TESTCASENAME/appLog.out");
      system("echo -------------------------------------------------- >> $outputDir/$TESTCASENAME/appLog.out");
      system("echo LOG-OUTPUT >> $outputDir/$TESTCASENAME/appLog.out");
      system("cat /tmp/opendias_test/var/log/opendias/opendias.log >> $outputDir/$TESTCASENAME/appLog.out");
      unlink("/tmp/opendias_test/var/log/opendias/opendias.log");
      system("sed -f config/appLogUnify.sed < $outputDir/$TESTCASENAME/appLog.out > $outputDir/$TESTCASENAME/appLog4Compare.out");
      removeDuplicateLines("$outputDir/$TESTCASENAME/appLog4Compare.out");
      if( length $GENERATE ) {
        system("cp $outputDir/$TESTCASENAME/appLog4Compare.out $TESTPATH/expected/$TESTCASENAME/appLog.out");
      }
      $APP_RES="<td class='none'><a href='./$TESTCASENAME/appLog.out'>actual</a></td>";
      system("diff -ydN $TESTPATH/expected/$TESTCASENAME/appLog.out $outputDir/$TESTCASENAME/appLog4Compare.out > $outputDir/$TESTCASENAME/appLogDiff.out");
      if( $? >> 8 == 0 ) {
        system("rm $outputDir/$TESTCASENAME/appLogDiff.out");
        $APP_RES .= "<td class='ok'>OK</td>";
      }
      else {
        $APP_RES .= "<td><a href='./$TESTCASENAME/appLogDiff.out'>diff</a>&nbsp;|&nbsp;<a href='../../$TESTPATH/expected/$TESTCASENAME/appLog.out'>expected</a></td>";
        $RES=1;
      }

      # Collate results
      if($RES == 0) {
        printProgressLine($TEST, "PASSED\n");
        print OUTPUTINDEX "<tr><td class='ok'>PASS</td>";
        $passCount++;
      }
      else {
        printProgressLine($TEST, "FAILED\n");
        print OUTPUTINDEX "<tr class='fail'><td>FAIL</td>";
        $failCount++;
      }

    }

    # Output result line
    print OUTPUTINDEX <<EOS;
      <td><a href='./$TESTCASENAME/'>$TESTCASENAME</a></td>
      <td style='text-align: right'>$testTime</td>
      $MEM_RES
      $TEST_RES
      $APP_RES 
    </tr>
EOS

    $testCount++;

  } # Tests implied by each specified case (ie "1*" is more than just one test)

} # Test cases on command line

print "\n";

print OUTPUTINDEX <<EOS;
    </table>
    <hr />
    <p>
      <strong>
        Tests: $testCount<br />
        Passed: $passCount<br />
        Failed: $failCount
      </strong>
    </p>
  </body>
</html>
EOS

close(OUTPUTINDEX);

sub printProgressLine {

  my ($item1, $item2, ) = @_;
  my $end = "\r";

  if( chomp( $item2 ) ) {
    $end = "\n";
  }
  print substr( sprintf ( '%-50s', $item1 ), 0, 50 ) . ' ' . substr( sprintf ( '%30s', $item2 ), 0, 30 );
  print $end;

}

__END__

package regressionTests::504_could_not_start_http_service;

use lib qw( regressionTests/lib );
use DBI;
use standardTests;
use strict;
use IO::Socket::INET;

our $sock;

sub updateStartCommand {
  # Start a port on the opendias port

  $sock = IO::Socket::INET->new( Listen => 1,
                                 LocalAddr => 'localhost',
                                 LocalPort => '8988',
                                 Proto => 'tcp');

  o_log("Reserved the opendiaqs port, to stop the service from starting correctly.");
}

sub test {

  system("while [ \"`pidof valgrind.bin`\" != \"\" ]; do sleep 1; done");

  # close the overridding http port
  $sock->close();

  return 0;
}

return 1;


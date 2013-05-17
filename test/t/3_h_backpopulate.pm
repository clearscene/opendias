package t::3_h_backpopulate;

use lib qw( lib );
use DBI;
use standardTests;
use Data::Dumper;
use strict;

sub testProfile {
  return {
    valgrind => 1,
    client => 0,
    updateStartCommand => 'updateStartCommand',
  };
}

sub updateStartCommand {
  my $startCommand = shift;
  chomp( my $pwd = `pwd` );
  my $prefix = "LD_LIBRARY_PATH=$pwd/override_libs/liblept:$pwd/override_libs/libpHash ";
  $$startCommand =~ s/^/$prefix/g;
  o_log("Updated start command to use overidden libs");
}

sub test {

  my $dbh = DBI->connect( "dbi:SQLite:dbname=/tmp/opendiastest/openDIAS.sqlite3",
                          "", "", { RaiseError => 1, AutoCommit => 1, sqlite_use_immediate_transaction => 1 } );

  my $sth = $dbh->prepare("SELECT config_value FROM config WHERE config_option = 'backpopulate_phash';");

  my $attempt = 0;
  while ( 1 ) {

    sleep ( 1 );
    $attempt++;

    $sth->execute();
    my $hashRef = $sth->fetchrow_hashref();
    my $ret = $hashRef->{config_value};
    $sth->finish;

    last if( $ret eq "complete" );

    if( $attempt > 120 ) {
      o_log( "Waiting for the back population to complete, but never happened! Forcing q shutdown.");
      last;
    }

  }
  $dbh->disconnect();
    
  return 0;
}

return 1;


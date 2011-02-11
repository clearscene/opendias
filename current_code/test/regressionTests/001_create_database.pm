package regressionTests::001_create_database;

use lib qw( regressionTests/lib );
use DBI;
use standardTests;
use strict;

sub test {

  my $dbh = DBI->connect( "dbi:SQLite:dbname=/tmp/opendiastest/openDIAS.sqlite3", 
                          "", "", { RaiseError => 1, AutoCommit => 0 } );

  my $sth = $dbh->prepare("SELECT version FROM version");
  $sth->execute();

  while( my $hashRef = $sth->fetchrow_hashref() ) {
    if ( $hashRef->{version} eq "4" ) {
      o_log("Correct DB version.");
    }
    else {
      o_log("Incorrect DB version.");
      $dbh->disconnect();
      return 1;
    }
  }

  $dbh->disconnect();
  return 0;
}

return 1;


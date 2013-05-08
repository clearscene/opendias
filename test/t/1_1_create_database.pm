package t::1_1_create_database;

use lib qw( lib );
use DBI;
use standardTests;
use strict;

sub testProfile {
  return {
    valgrind => 1,
    client => 0,
    startTimeout => 240,
  };
}

sub test {

  my $dbh = DBI->connect( "dbi:SQLite:dbname=/tmp/opendiastest/openDIAS.sqlite3", 
                          "", "", { RaiseError => 1, AutoCommit => 1, sqlite_use_immediate_transaction => 1 } );

  my $sth = $dbh->prepare("SELECT version FROM version");
  $sth->execute();

  while( my $hashRef = $sth->fetchrow_hashref() ) {
    if ( $hashRef->{version} eq "7" ) {
      o_log("Correct DB version.");
    }
    else {
      o_log("Incorrect DB version.");
      $dbh->disconnect();
      return 1;
    }
  }

  # Apply some diffs we know the reference DB will have from the default build
  # An additional test user, and
  # Wont try to back populate the pHASH for images.
  $dbh->do("INSERT INTO user_access VALUES('test-user','63294d4f9a52894e2ef9298d1b8ac455','Test - User','2012-11-22 17:26:00','automatically',2)");
  $dbh->do("UPDATE config SET config_value='no' WHERE config_option='backpopulate_phash'");

  $dbh->disconnect();

  # compare the generated database schema
  system("/usr/bin/sqlite3 /tmp/opendiastest/openDIAS.sqlite3 '.dump' > /tmp/generated.sql");
  system("/usr/bin/sqlite3 config/defaultdatabase.sqlite3 '.dump' > /tmp/reference.sql");
  o_log("Diff of generated-to-reference database: \n" . `diff /tmp/generated.sql /tmp/reference.sql` );

  return 0;
}

return 1;


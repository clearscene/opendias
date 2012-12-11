package r::3_7_deleteDoc;

use lib qw( r/lib );
use DBI;
use standardTests;
use Data::Dumper;
use strict;

sub testProfile {
  return {
    valgrind => 1,
    client => 0,
  }; 
} 

sub test {

  my $cookie_jar = HTTP::Cookies->new();
  my %updateData = (
    __cookiejar => $cookie_jar,
    action => 'deleteDoc',
    docid => 9999,
  );

  login( "test-user", "password", $cookie_jar );

  dumpState();

  # BAD doc details
  o_log( "Fail to Remove a doc linkage" );
  o_log( Dumper( directRequest( \%updateData ) ) );

  dumpState();

  # Update doc details
  o_log( "Remove a doc linkage" );
  $updateData{docid} = 2;
  o_log( Dumper( directRequest( \%updateData ) ) );

  dumpState();

  return 0;
}

sub dumpState {
  dumpQueryResult( "SELECT docid, title FROM docs ORDER BY docid" );
  dumpQueryResult( "SELECT doclinkid, docid, linkeddocid FROM doc_links ORDER BY doclinkid" );
  dumpQueryResult( "SELECT doctagid, docid, tagid FROM doc_tags ORDER BY doctagid" );
  dumpQueryResult( "SELECT tagid, tagname FROM tags ORDER BY tagid" );
}

return 1;


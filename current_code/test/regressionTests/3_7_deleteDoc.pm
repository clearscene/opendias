package regressionTests::3_7_deleteDoc;

use lib qw( regressionTests/lib );
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

  my %updateData = (
    action => 'deleteDoc',
    docid => 2,
  );

  dumpQueryResult( "SELECT docid, title FROM docs ORDER BY docid" );
  dumpQueryResult( "SELECT doclinkid, docid, linkeddocid FROM doc_links ORDER BY doclinkid" );
  dumpQueryResult( "SELECT doctagid, docid, tagid FROM doc_tags ORDER BY doctagid" );
  dumpQueryResult( "SELECT tagid, tagname FROM tags ORDER BY tagid" );

  # Update doc details
  o_log( "Remove a doc linkage" );
  o_log( Dumper( directRequest( \%updateData ) ) );

  dumpQueryResult( "SELECT docid, title FROM docs ORDER BY docid" );
  dumpQueryResult( "SELECT doclinkid, docid, linkeddocid FROM doc_links ORDER BY doclinkid" );
  dumpQueryResult( "SELECT doctagid, docid, tagid FROM doc_tags ORDER BY doctagid" );
  dumpQueryResult( "SELECT tagid, tagname FROM tags ORDER BY tagid" );

  return 0;
}

return 1;


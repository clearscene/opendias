package regressionTests::3_6_moveTag;

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

  my %getData = (
    action => 'getDocDetail',
    docid => 2,
  );

  my %updateData = (
    action => 'moveTag',
    docid => 2,
    tag => undef,
    subaction => undef, # 'addTag','removeTag','addDoc','removeDoc'
  );

  # Call getDocDetails
  o_log( "Doc Details" );
  o_log( Dumper( directRequest( \%getData ) ) );


  # Update doc details
  o_log( "Add a tag linkage" );
  $updateData{tag} = 'new Ϋ tag';
  $updateData{subaction} = 'addTag';
  o_log( Dumper( directRequest( \%updateData ) ) );

  # Call getDocDetails
  o_log( "Doc Details - after update" );
  o_log( Dumper( directRequest( \%getData ) ) );


  # Update doc details
  o_log( "Add a doc linkage" );
  $updateData{tag} = 3;
  $updateData{subaction} = 'addDoc';
  o_log( Dumper( directRequest( \%updateData ) ) );

  # Call getDocDetails
  o_log( "Doc Details - after update" );
  o_log( Dumper( directRequest( \%getData ) ) );



  dumpQueryResult( "SELECT doctagid, docid, tagid FROM doc_tags ORDER BY doctagid" );
  dumpQueryResult( "SELECT tagid, tagname FROM tags ORDER BY tagid" );

  # Update doc details
  o_log( "Remove a tag linkage" );
  $updateData{tag} = 'tag two';
  $updateData{subaction} = 'removeTag';
  o_log( Dumper( directRequest( \%updateData ) ) );

  # Call getDocDetails
  o_log( "Doc Details - after update" );
  o_log( Dumper( directRequest( \%getData ) ) );

  dumpQueryResult( "SELECT doctagid, docid, tagid FROM doc_tags ORDER BY doctagid" );
  dumpQueryResult( "SELECT tagid, tagname FROM tags ORDER BY tagid" );

  # Update doc details
  o_log( "Remove a tag linkage" );
  $updateData{tag} = 'tag one';
  $updateData{subaction} = 'removeTag';
  o_log( Dumper( directRequest( \%updateData ) ) );

  # Call getDocDetails
  o_log( "Doc Details - after update" );
  o_log( Dumper( directRequest( \%getData ) ) );

  # Were expecting not only the doc_tag linkage (to 'tag one') to be deleted, but also the tag itself, since it was linked to only one doc
  dumpQueryResult( "SELECT doctagid, docid, tagid FROM doc_tags ORDER BY doctagid" );
  dumpQueryResult( "SELECT tagid, tagname FROM tags ORDER BY tagid" );


  # Update doc details
  o_log( "Remove a doc linkage" );
  $updateData{tag} = 4;
  $updateData{subaction} = 'removeDoc';
  o_log( Dumper( directRequest( \%updateData ) ) );

  # Call getDocDetails
  o_log( "Doc Details - after update" );
  o_log( Dumper( directRequest( \%getData ) ) );


  return 0;
}

return 1;


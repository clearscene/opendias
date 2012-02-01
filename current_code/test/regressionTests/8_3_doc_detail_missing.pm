package regressionTests::8_3_doc_detail_missing;

use lib qw( regressionTests/lib );

use WWW::HtmlUnit;
use standardTests;

use strict;

sub testProfile {
  return {
    valgrind => 0,
    client => 1,
  }; 
} 

sub test {

  my ($page, $elements, ) = @_;

  # Check the document page loads - without data
  
  $page = getPage("/docDetail.html?docid=999");
  waitForPageToFinish($page);
  my $alert = getNextJSAlert();
  unless ( defined $alert && $alert eq "Unable to get document details: Your request could not be processed" ) {
    o_log("FAIL: 'no such doc' alert is missing.");
    return 1;
  }
  o_log("Correctly informed - no such doc");
  return 0;
}

return 1;

__END__

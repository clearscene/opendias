package regressionTests::004_doc_detail_missing;

use lib qw( regressionTests/lib );

use WWW::HtmlUnit;
use standardTests;

use strict;

sub test {

  my ($page, $elements, ) = @_;

  # Check the document page loads - without data
  
  $page = getPage("/docDetail.html?docid=999");
  waitForPageToFinish($page);
  my $alert_arrayref = $alert_handler->getCollectedAlerts->toArray();
  if ( $alert_arrayref->[0] ne "Unable to get document details: Your request could not be processed" ) {
    o_log("FAIL: No alert that we could not load the document");
    return 1;
  }
  o_log("Correctly informed - no such doc");
  return 0;
}

return 1;

__END__

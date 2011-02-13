package regressionTests::003_doc_detail;

use lib qw( regressionTests/lib );

use WWW::HtmlUnit;
use standardTests;

use strict;

sub test {

  my ($page, $elements, ) = @_;

  # Check the document page loads - with data
  $page = getPage("/docDetail.html?docid=1");
  waitForPageToFinish($page);
  $elements = $page->getElementsByTagName("h2")->toArray();
  if( @{$elements}[0]->getTextContent() ne "Document Detail" ) {
    o_log("FAIL: Doc detail title was incorrect");
    return 1;
  }

  return 0;
}

return 1;

__END__

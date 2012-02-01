package regressionTests::003_doc_detail;

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

  # Check the document page loads - with data
  $page = getPage('/docDetail.html?docid=1');
  waitForPageToFinish($page);

  $elements = $page->getElementsByTagName('h2')->toArray();
  if( @{$elements}[0]->getTextContent() ne 'Document Detail' ) {
    o_log('FAIL: Doc detail title was incorrect');
    return 1;
  }

  # Ensure the data loaded OK
  $elements = $page->getElementById('title');
  if( $elements->getText() ne 'Test Title' ) {
    o_log('FAIL: Incorrect title');
    return 1;
  }

  $elements = $page->getElementById('scanDate');
  if( $elements->getTextContent() ne '2011-01-02T21:13:04.393946Z' ) {
    o_log('FAIL: Incorrect scan date');
    return 1;
  }

  $elements = $page->getElementById('docDate');
  if( $elements->getText() ne '2010/12/31' ) {
    o_log('FAIL: Incorrect document date');
    return 1;
  }
  
  $elements = $page->getElementById('type');
  if( $elements->getTextContent() ne 'Scanned Document' ) {
    o_log('FAIL: Incorrect document type');
    return 1;
  }
  
  $elements = $page->getElementById('ocrtext');
  if( $elements->getText() ne 'This is the OCR text.' ) {
    o_log('FAIL: Incorrect ocrtext');
    return 1;
  }

  o_log('All loaded OK');  
  return 0;
}

return 1;

__END__

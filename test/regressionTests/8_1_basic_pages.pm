package regressionTests::8_1_basic_pages;

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

  # Check the homepage loads
  $page = getPage("/");
  waitForPageToFinish($page);
  $elements = $page->getElementsByTagName("h2")->toArray();
  if( @{$elements}[0]->getTextContent() ne "Application Home" ) {
    o_log("FAIL: Home page title was incorrect");
    return 1;
  }

  $elements = $page->getElementById('username');
  $elements->setText("test-user");
  $elements = $page->getElementById('password');
  $elements->setText("password");
  $elements = $page->getElementById('loginbutton');
  $elements->click();
  sleep(1);

  # Check document page loads
  $page = $page->getElementById("doclistLink")->click();
  waitForPageToFinish($page);
  $elements = $page->getElementsByTagName("h2")->toArray();
  if( @{$elements}[0]->getTextContent() ne "Document List" ) {
    o_log("FAIL: Document page title was incorrect");
    return 1;
  }
  
  # Check aquire loads
  $page = $page->getElementById("acquireLink")->click();
  waitForPageToFinish($page);
  $elements = $page->getElementsByTagName("h2")->toArray();
  if( @{$elements}[0]->getTextContent() ne "Acquire new Document" ) {
    o_log("FAIL: Acquire page title was incorrect");
    return 1;
  }
  
  # Check returning to the homepage loads
  $page = $page->getElementById("homeLink")->click();
  waitForPageToFinish($page);
  $elements = $page->getElementsByTagName("h2")->toArray();
  if( @{$elements}[0]->getTextContent() ne "Application Home" ) {
    o_log("FAIL: Home page title was incorrect");
    return 1;
  }

  return 0;
}

return 1;

__END__

#  my $f = $page->getFormByName('f');
#  my $submit = $f->getInputByName("btnG");
#  my $query  = $f->getInputByName("q");
#  $page = $query->type("HtmlUnit");
#  $page = $query->type("\n");
#  my $content = $page2->asXml;
#  print "Result:\n$content\n\n";

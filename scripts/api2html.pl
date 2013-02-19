#!/usr/bin/perl -w

use strict;
use warnings;

use XML::Simple;
use Data::Dumper;

my $xml = XMLin('API', ForceArray => 1 );

#
# Standard Header
#
print <<EOS;
<!--#include virtual="/include/head.html" -->

<style>
.code {
  font-family: courier;
  background-color: #eeeeff;
  border: 1px solid #444;
  padding: 4px;
  font-size: 0.8em;
  overflow-x: auto;
}
li .code {
  margin-left: -40px;
}
.spec {
  margin-left: 25px;
  margin-bottom: 45px;
}
</style>
EOS

print "<h2>openDIAS API Spec</h2>\n";
print "<p>Version: " . $xml->{version}[0] . "</p>\n";

#
# Contents
#
print "<ul>\n";
foreach my $spec ( @{$xml->{ApiSpec}[0]->{ApiCall}} ) {
  print "<li>" . $spec->{call_name}[0] . "</li>\n";
}
print "</ul>\n";

#
# General Help sections
#
foreach my $spec ( @{$xml->{GeneralHelp}[0]->{Section}} ) {
  print "<h3>" . $spec->{title}[0] . "</h3>\n";
  print "<p>" . $spec->{body}[0] . "</p>\n";
  if( exists $spec->{example} ) {
    $spec->{example}[0] =~ s/</\&lt;/g;
    print "<pre class='code'>" . $spec->{example}[0] . "</pre>\n";
  }
}


#
# API sections
#
foreach my $spec ( @{$xml->{ApiSpec}[0]->{ApiCall}} ) {
  print "<h3>" . $spec->{call_name}[0] . "</h3>\n";
  print "<div class='spec'>\n";
  print "<p><strong>" . $spec->{purpose}[0] . "</strong></p>\n";

  print "<h4>Calls method</h4>\n";
  print "<p>" . $spec->{calls_method}[0] . "</p>\n";

  print "<h4>Known Callers</h4>\n";
  print "<ul>\n";
  foreach my $caller ( @{$spec->{KnownCallers}[0]->{caller}} ) {
    print "<li>" . $caller . "</li>\n";
  }
  print "</ul>";

  print "<h4>Takes inputs of</h4>\n";
  print "<dl>\n";
  foreach my $input ( @{$spec->{Inputs}[0]->{Input}} ) {
    print "<dt>" . $input->{name}[0] . "</dt>\n";
    print "<dd>" . $input->{value}[0] . "</dd>\n";
  }
  print "</dl>";

  if( exists $spec->{encoding} ) {
    print "<p>" . $spec->{encoding}[0] . "</p>\n";
  }

  if( exists $spec->{example} ) {
    $spec->{example}[0]->{content} =~ s/</\&lt;/g;
    print "<pre class='code'>" . $spec->{example}[0]->{content} . "</pre>\n";
  }
  print "</div>\n\n";
}


#
# Standard footer
#
print <<EOS;
<!--#include virtual="/include/footer.html" -->
EOS

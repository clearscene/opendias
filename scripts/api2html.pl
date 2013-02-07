#!/usr/bin/perl -w

use strict;
use warnings;

use XML::Simple;
use Data::Dumper;

my $xml = XMLin('API', ForceArray => 1 );
delete $xml->{ApiSpec}[0]->{naaa};
#print Dumper( $xml );

print join(', ', (keys $xml)) . "\n";
print "Version: " . $xml->{version}[0] . "\n";

#print Dumper( $xml->{ApiSpec} ) . "\n";
#print join(', ', (keys %{$xml->{ApiSpec}[0]} ) ) . "\n";
foreach my $spec ( @{$xml->{ApiSpec}[0]->{ApiCall}} ) {
  print $spec->{call_name}[0]."\n";
}

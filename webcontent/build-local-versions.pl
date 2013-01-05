#!/usr/bin/perl -w

use strict;
use warnings;

my @templates = qw( includes/header.txt includes/footer.txt );
my %langs = ( 'en' => 1 );

opendir( DIR, "./" );
while( my $FILE = readdir( DIR ) ) {

  if ( my ($lang) = ( $FILE =~ /language\.resource\.(..)/ ) ) {
    $langs{$lang} = 1;    
  }

  elsif ( my ($template) = ( $FILE =~ /^(.*\.tmpl)$/ ) ) {
    push @templates, $template;
  }

}
closedir( DIR );

foreach my $lang ( keys %langs ) {
  my %lang_pack = ();
  open( LANGPACK, "language.resource.$lang" );
  while ( my $line = <LANGPACK> ) {
    chomp( $line );
    my ( $key, $data ) = split( /\|/, $line );
    $lang_pack{ $key } = $data;
  }
  close( LANGPACK );

  foreach my $file ( @templates ) {
    print "Generating an '$lang' version of '$file'\n";
    open( SOURCE, $file );
    my $targ = $file;
    $targ =~ s/tmpl/html/g;
    open( TARGET, ">$targ.$lang" );
    while( my $line = <SOURCE> ) {
      print TARGET translate( $line, %lang_pack );
    }
    close( TARGET );
    close( SOURCE );
  }
}

sub translate {
  my ( $line, %lang_pack ) = @_;

  foreach my $key ( keys %lang_pack ) {
    $line =~ s/---$key---/$lang_pack{$key}/g;
  }
  return $line;
}

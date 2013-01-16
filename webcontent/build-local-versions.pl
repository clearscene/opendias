#!/usr/bin/perl -w

use strict;
use warnings;

my @templates = qw( includes/header.txt includes/footer.txt );
my %langs = ( 'en' => 1 );
my $hide = 0;
my %option;

#########################################
# Collection options from the command line (ie what ./configure sends us)
foreach my $opt ( @ARGV ) {
  $option{$opt} = 1;
}


#########################################
# Find all the files that need localising 
# and the languages that can do it
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


#####################################
# Localise each file
foreach my $lang ( keys %langs ) {
  my %lang_pack = ();

  # Load the language pack
  open( LANGPACK, "language.resource.$lang" );
  while ( my $line = <LANGPACK> ) {
    chomp( $line );
    my ( $key, $data ) = split( /\|/, $line );
    $lang_pack{ $key } = $data;
  }
  close( LANGPACK );

  # Localise each file
  foreach my $file ( @templates ) {
    print "Generating an '$lang' version of '$file'\n";
    $hide = 0;
    open( SOURCE, $file );
    my $targ = $file;
    $targ =~ s/tmpl/html/g;
    open( TARGET, ">$targ.$lang" );
    while( my $line = <SOURCE> ) {
      if ( include_line( $line ) ) {
        print TARGET translate( $line, %lang_pack );
      }
    }
    close( TARGET );
    close( SOURCE );
  }
}


###############################################
# Does a config option say not to include something
sub include_line {
  my ( $line, ) = @_;
  chomp( $line );
  if ( $line =~ s/^#ifdef // ) {
    if ( ! $option{$line} ) {
      $hide++;
    }
    return 0; # never include a control line
  }
  elsif ( $line =~ s/^#endifdef // ) {
    if ( ! $option{$line} ) {
      $hide--;
    }
    return 0; # never include a control line
  }
  elsif ( $line =~ s/^#ifndef // ) {
    if ( $option{$line} ) {
      $hide++;
    }
    return 0; # never include a control line
  }
  elsif ( $line =~ s/^#endifndef // ) {
    if ( $option{$line} ) {
      $hide--;
    }
    return 0; # never include a control line
  }
  return ! $hide;
}


####################################
# Actually replace text
sub translate {
  my ( $line, %lang_pack ) = @_;

  foreach my $key ( keys %lang_pack ) {
    $line =~ s/---$key---/$lang_pack{$key}/g;
  }
  return $line;
}

__END__

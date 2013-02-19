#!/usr/bin/perl -w

use strict;
use warnings;

my @templates = qw( webcontent/includes/header.txt webcontent/includes/footer.txt );
my %langs = ( 'en' => 1 );
my $hide = 0;
my %option;
my $year = 1900+((gmtime())[5]);
my $package_string = 'opendias';

#########################################
# Collection options from the command line (ie what ./configure sends us)
$package_string = shift @ARGV;
foreach my $opt ( @ARGV ) {
  $option{$opt} = 1;
}


#########################################
# Find all the files that need localising 
# and the languages that can do it
opendir( DIR, "webcontent/" );
while( my $FILE = readdir( DIR ) ) {

  if ( my ($lang) = ( $FILE =~ /language\.resource\.(..)/ ) ) {
    $langs{$lang} = 1;    
  }

  elsif ( my ($template) = ( $FILE =~ /^(.*\.tmpl)$/ ) ) {
    push @templates, 'webcontent/'.$template;
  }

}
closedir( DIR );


#####################################
# Generate a testing language pack.
generate_test_language();


#####################################
# Localise each file
foreach my $lang ( keys %langs ) {
  my %lang_pack = ();

  my $resp = validate_language_pack( $lang );
  die $resp if $resp ne '';

  # Load the language pack
  open( LANGPACK, "webcontent/language.resource.$lang" );
  while ( my $line = <LANGPACK> ) {
    chomp( $line );
    my ( $key, $data ) = split( /\|/, $line );
    $lang_pack{ $key } = $data;
  }
  close( LANGPACK );

  # Add 'special cases' to the language pack
  $lang_pack{'CURRENT_YEAR'} = $year;
  $lang_pack{'PACKAGE_STRING'} = $package_string;

  # Localise each file
  foreach my $file ( @templates ) {
    print "Generating an '$lang' version of '$file'\n";
    $hide = 0;
    $lang_pack{'UPDATE_WARNING'} =<<EOS;
<!-- ##############################################
      Do not update this file directly, it has 
      been auto generated, from:
      $file - for the structure, and
      language.resource.$lang - for the readable text
     ############################################## -->
EOS
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


####################################
# Generate test localisation packs
sub generate_test_language {

  if ( exists $option{'CREATE_TEST_LANGUAGE'} ) {

    my @resource_files = qw( webcontent/language.resource.en i18n/language.resource.en );
    opendir( DIR, 'webcontent/includes/local/' );
    while( my $FILE = readdir( DIR ) ) {
      if ( $FILE =~ /\.resource\.en/ ) {
        push @resource_files, 'webcontent/includes/local/'.$FILE;
      }
    }
    closedir( DIR );

    # Load the language pack
    foreach my $file (@resource_files) {

      print "Creating a test language pack for $file\n";

      my $file_out = $file;
      $file_out =~ s/en$/hh/;

      open( SOURCE, $file );
      open( TARGET, ">$file_out" );
      while ( my $line = <SOURCE> ) {
        chomp( $line );
        next if $line =~ /^$/ || $line =~ /^#/ || $line =~ /^\/\//;
        if ( $file =~ /includes\/local/ ) {
          # A javascript localise resource file
          my ( $key, $data ) = $line =~ /^(.*) = '(.*)';/;
          $data =~ s/[^\s]/#/g;
          print TARGET "$key = '$data';\n";
        }
        else {
          # An application resource file
          my ( $key, $data ) = split( /\|/, $line );
          $data =~ s/[^\s]/#/g;
          print TARGET "$key|$data\n";
        }
      }
      close( TARGET );
      close( SOURCE );
      $langs{'hh'} = 1;
    }

  }
  else {
    unlink('webcontent/language.resource.hh');
    delete $langs{'hh'};
  }

}

sub validate_language_pack {
  my ( $lang, ) = @_;
  my $ret = '';

  # English files are the baseline to which everything else is compared
  return $ret if $lang eq 'en';

  my @resource_files = qw( webcontent/language.resource.en i18n/language.resource.en );
  opendir( DIR, 'webcontent/includes/local/' );
  while( my $FILE = readdir( DIR ) ) {
    if ( $FILE =~ /\.resource\.en/ ) {
      push @resource_files, 'webcontent/includes/local/'.$FILE;
    }
  }
  closedir( DIR );

  # Load the language pack
  foreach my $file (@resource_files) {

    my $working_file = $file;
    my %compare;

    foreach my $compare_langs ( 'en', $lang ) {
    
      $working_file =~ s/en$/$compare_langs/;

      open( SOURCE, $working_file );
      while ( my $line = <SOURCE> ) {
        chomp( $line );
        next if $line =~ /^$/ || $line =~ /^#/ || $line =~ /^\/\//;
        my ( $key, $data );
        if ( $file =~ /includes\/local/ ) {
          # A javascript localise resource file
          ( $key, $data ) = $line =~ /^(.*) = '(.*)';/;
          $compare{$compare_langs}{$key} = 1;
        }
        else {
          # An application resource file
          ( $key, $data ) = split( /\|/, $line );
          $compare{$compare_langs}{$key} = 1;
        }
        if ( $data =~ /LOCALISE_ME/ ) {
          print "Please localise the phrase '$key' into '$lang'\n";
        }
      }
      close( SOURCE );
    }

    foreach my $key ( keys %{$compare{'en'}} ) {
      if ( ! exists $compare{$lang}{$key} ) {
        $ret .= "The resouce file $file is missing the phrase '$key'.\n";
      }
    }

  }

  return $ret;
}
__END__

#!/usr/bin/env perl

use strict;
use warnings;

use lib ('.');

use File::Basename;

my $vhome = '.';

use BRLCAD_DOC (':all');

# shadow colors from http://brlcad.org/~sean/images/color_palette.png
# Sean suggest using one of them (as yet unnamed)

# cc6666
# cc9966
# 669966
# 6699cc

my %color
  = (
     'green' => 'green',
     'limegreen' => 'limegreen',
     'red' => 'red',
     'blue' => 'blue',

     # in hex
     'cc6666' => '#cc6666',
     'cc9966' => '#cc9966',
     '669966' => '#669966',
     '6699cc' => '#6699cc',
    );

#my $color = 'limegreen';
#my $color = 'red';
#my $color = 'green';
#my $color = 'blue';

# shadow colors
#my $color = 'cc9966'; # brown
#my $color = 'cc6666'; # red
#my $color = '669966'; # green
#my $color = '6699cc'; # blue

die "Enter book DB xml source file name.\n" if !@ARGV;

my $ifil      = shift @ARGV;
my $lang_code = 0;
my $debug     = 0;
my $draft     = 0;

foreach my $arg (@ARGV) {
  if ($arg =~ m{\A [-]{1,2}de }xmsi) {
    $debug = 1;
  }
  elsif ($arg =~ m{\A [-]{1,2}dr }xmsi) {
    $draft = 1;
  }
  elsif (!$lang_code) {
    $lang_code = lc $arg;
  }
}
$lang_code = 'en' if !$lang_code;

# known names

# the autogenerated file:
my $nam = $ifil;
$nam = 'books/en/BRL-CAD_Tutorial_Series-VolumeI.xml'
  if $nam eq 'dummy.xml';

# trim extra stuff to leave a unique name
$nam =~ s{\.xml \z}{}xmsi;
$nam =~ s{\.fo \z}{}xmsi;

$nam =~ s{\A books/en/}{}xmsi;

my $brldir = './resources/brlcad';
my $ofil  = "${brldir}/book-covers-fo-autogen.xsl";
my $ofil2 = "${brldir}/brlcad-colors-autogen.xsl";

# colors are hard-wired by title
my %name
  = (
     'BRL-CAD_Tutorial_Series-VolumeI'
     => {
	 titles => [ 'Tutorial Series',
		     'Volume I',
		     'An Overview of BRL-CAD',
		   ],
	 color => 'cc9966',
	},

     'BRL-CAD_Tutorial_Series-VolumeII'
     => {
	 titles => [
		    'Tutorial Series',
		    'Volume II',
		    'Introduction to MGED',
		   ],
	 color => 'cc6666',
	},

     'BRL-CAD_Tutorial_Series-VolumeIII'
     => {
	 titles => [
		    'Tutorial Series',
		    'Volume III',
		    'Principles of Effective Modeling',
		   ],
	 color => '669966',
	},

     'BRL-CAD_Tutorial_Series-VolumeIV'
     => {
	 titles => [
		    'Tutorial Series',
		    'Volume IV',
		    'Converting Geometry Between BRL-CAD and Other Formats',
		   ],
	 color => '6699cc',
	},
    );

if (!exists $name{$nam}) {
  die "Unknown file for cover '$ifil'.";
}

my $color = $name{$nam}{color};
die "ERROR:  Color selected ($color) not known in color has \%color.\n"
  if !exists $color{$color};
my $color_code = $color{$color};

my @titles = @{$name{$nam}{titles}};

# the covers template source:
my $cvr = "${brldir}/book-covers-fo-template.xsl";

open my $fpi, '<', $cvr
  or die "$cvr: $!";

# get the subversion data for the cover
my ($last_rev, $has_codes) = get_svn_status($ifil);

if (0) {
  print "file '$ifil'\n";
  print "svn revision '$last_rev'\n";
  print "has code: '$has_codes'\n";
  if ($has_codes) {
    print "file is NOT fully commited\n";
  }
  else {
    print "file IS fully commited\n";
  }
}

if ($has_codes) {
  print "ERROR:  file is NOT fully commited: '$ifil'\n";
}

open my $fp, '>', $ofil
  or die "$ofil: $!";
open my $fp2, '>', $ofil2
  or die "$ofil2: $!";

print "DEBUG: output file  = '$ofil'\n"
  if $debug;
print "DEBUG: output file2 = '$ofil2'\n"
  if $debug;


# special first line handling
my $line = <$fpi>;
if ($line =~ m{\A \s* \<\?xml}xms) {
  print $fp $line;
  BRLCAD_DOC::print_autogen_header($fp, $0);
  BRLCAD_DOC::print_autogen_header($fp2, $0);
}
else {
  print "Unexpected first line:\n";
  print $line;
  die "Unable to continue";
}

print_color_file($fp2, $color_code);
close $fp2;

my $need_brlcad_logo_group = 1;
my $need_draft             = $draft;
my $need_title             = 1;

while (defined(my $line = <$fpi>)) {

  if ($need_draft
      && $line =~ m{\A \s* \<\?brlcad \s+ insert\-draft\-overlay \s* \?\>}xms) {
    BRLCAD_DOC::print_draft_overlay($fp);
    $need_draft = 0;
    next;
  }

  if ($need_brlcad_logo_group
      && $line =~ m{\A \s* \<\?brlcad \s+ insert\-brlcad\-logo\-group \s* \?\>}xms) {
    BRLCAD_DOC::print_brlcad_logo_group($fp, $color);
    $need_brlcad_logo_group = 0;
    next;
  }

  # at the moment titles are in black
  if ($need_title
      && $line =~ m{\A \s* \<\?brlcad \s+ insert\-title \s* \?\>}xms) {
    BRLCAD_DOC::print_book_title($fp, $last_rev, @titles);
    $need_title = 0;
    next;
  }

  print $fp $line;
}

package ParsePPCHeader;

# parse a pre-processed C header (with gcc -E)

use strict;
use warnings;

use CGrammar; # <== an auto-generated file

sub parse_cfile {
  my $ifil = shift @_;
  my $oref = shift @_;

  $oref = 0 if !defined $oref;

  open my $fp, '<', $ifil
    or die "$ifil: $!";

  # The diagnostics provided by the tracing mechanism always go to
  # STDERR. If you need them to go elsewhere, localize and reopen
  # STDERR prior to the parse.
  #
  # For example:
  #
  #  {
  #      local *STDERR = IO::File->new(">$filename") or die $!;
  #
  #      my $result = $parser->startrule($text);
  #  }


  # IMPORTANT: namespaces as defined below are critical for use in the
  # CGrammar module!!
  package Parse::Recdescent::CGrammar;

  my $errfil = 'PRD-errfile.txt';
  local *STDERR = IO::File->new(">$errfil")
    or die $!;
  push @{$oref}, $errfil
    if $oref;

  local $/;
  my @ilines = <$fp>;
  my $text = join(' ', @ilines);

  $::RD_HINT = 1;
  $::RD_AUTOACTION = q {%item};
  $::opt_FUNCTIONS    = '1';
  $::opt_DECLARATIONS = '1';
  $::opt_STRUCTS      = '1';

  $::functions_output = '';
  $::declarations_output = '';
  $::structs_output = '';

  my $parser = CGrammar->new();

  my $ptree = $parser->translation_unit($text);
  if (!defined $ptree) {
    warn "undef \$ptree";
    return;
  }

  print "\nDefined Functions:\n\n$::functions_output\n\n"
    if defined $::functions_output
      and $::opt_FUNCTIONS;
  print "\nDeclarations:\n\n$::declarations_output\n\n"
    if defined $::declarations_output
      and $::opt_DECLARATIONS;
  print "\nStructures:\n\n$::structs_output\n\n"
    if defined $::structs_output
      and $::opt_STRUCTS;

  use Data::Dumper;
  print Dumper($ptree);

  die "debug exit";

} # parse_cfile


# IMPORTANT: namespaces as defined below are critical for use in the
# CGrammar module!!

package Parse::Recdescent::CGrammar;

#===============================================================================
# two functions (modified) from P::RD's 'csourceparser.pl'
#===  FUNCTION  ================================================================
#         NAME:  flatten_list
#  DESCRIPTION:  Extracts values from a recursive list. Double whitespaces will
#  				 be reduced
# PARAMETER  1:  Array Reference
#===============================================================================

sub ::flatten_list {
    ( my $tokens = join ' ', map { ref($_) ? ::flatten_list(@$_) : ($_) } @_ ) =~ s/\s+/ /g;
    $tokens;
}

#===  FUNCTION  ================================================================
#         NAME:  flatten_list_beautified
#  DESCRIPTION:  Like flatten_list but inserts a newline after each semicolon
# PARAMETER  1:  Array Reference
#===============================================================================
sub ::flatten_list_beautified {
    ( my $tokens = join ' ', map { ref($_) ? ::flatten_list(@$_) : ($_) } @_ ) =~ s/\s+/ /g;
    $tokens =~ s/;/;\n/g;
	$tokens =~ s/^\s*/\t/mg;
    $tokens;
}

# mandatory true return for a Perl module
1;

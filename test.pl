#!/usr/bin/env perl

use 5.014;
use warnings;
use Getopt::Std;

my %opts;
getopts('rcop', \%opts);

if ($opts{c}) {
  system('g++ -c main.cpp -pthread -std=c++11');
  system('g++ main.o -o main -pthread -std=c++11');
}

my ($n_vars, $n_funcs, $n_files) = @ARGV;
unless ($opts{p}) {
  system("./makePoly.pl @ARGV");
  say "makePoly.pl done.";
}

if ($opts{o}) {
  exec("./main $n_vars $n_files") if $opts{r};
  system("./main $n_vars $n_files");
} else {
  exec("./main $n_vars $n_files >/dev/null") if $opts{r};
  system("./main $n_vars $n_files >/dev/null");
}

say "main done.";
my $all = 2**$n_vars;

sub make_num {
    my @a = split /x/, shift;
    shift @a;
    my @v = map {0} 1..$n_vars;
    for (@a) {
      $v[$_-1] = 1;
    }
    my $r = oct("0b".join '', @v);

    return $r // 0;
}

sub log2 {
  my $x = shift;
  my ($ans, $y) = (0, $x);
  while( $x>>=1 ) {$ans++};
  return $ans;
}

sub polynomial {
  my @v = map {0} 1..$all;
  $v[$_] = 1 for (map {make_num $_} split(/\+/, shift));
  my $j;
  for (my $i = 0; $i < log2($all); $i++) {
    $j = $all - 1;
    while ($j > 0) {
          for (my $k = 0; $k < 2**$i; $k++) {
              $v[$j - $k] = ($v[$j - $k] ^ $v[$j - $k - 2**$i]);
          }
          $j = $j - 2**($i + 1);
      }
  }
  return @v;
}

for my $n (1..$n_files) {
  my $pfname = "poly_${n}.txt";
  open (my $pfh, "<", "poly_${n}.txt");
  my $ofname = "out_${n}.txt";
  open (my $ofh, "<", "out_${n}.txt");

  while (<$pfh>) {
    chomp;
    chomp(my $o = <$ofh>);
    my @v = polynomial $_;
    local $" = '';
    die "Wrong for $_ $o @v" unless ($o == join '', @v);
  }
}

say "All right!";

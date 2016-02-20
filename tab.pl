#!/usr/bin/env perl
my $old = ' ' x 4;
my $new = ' ' x 2;

while (<>) {
  s/    /  /g;
  print;
}

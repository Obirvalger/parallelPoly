#! /usr/bin/env perl

# Запуск программы: ./ИМЯ_ФАЙЛА ЧИСЛО_ПЕРЕМЕННЫХ ЧИСЛО_ФУНКЦИЙ ЧИСЛО_ФАЙЛОВ
# Можно указывать не все параметры, тогда будут использованы значения по умолчанию
# Если программа не запускается попробуйте добавить права выполнения: chmod +x ИМЯ_ФАЙЛА

use 5.014;
use warnings;

my ($n_vars, $n_funcs, $n_files)  = @ARGV;
$n_vars  //= 3; # число переменных по умолчанию
$n_funcs //= $n_vars; # число функций по умолчанию
$n_files //= 3; #число файлов по умолчанию
my $all_funcs = 2**$n_vars;
my @vars = map {'x' . $_} 1..$n_vars;
my ($min_len, $max_len) = (0.25, 0.65); # вариация длины полинома

sub make_monom {
    my $r = '';
    my @s = split //, sprintf("%0${n_vars}b",shift);
    while ((my $i , $_) = each @s) {
        $r .= $vars[$i] if $_;
    }
    die "All bad s = @s" unless $r;
    return $r;
}

for my $n (1 .. $n_files) {
    #my $fname = "poly_${n_vars}_${n_funcs}_${n}.txt"; # шаблон имени файла
    my $fname = "poly_${n}.txt"; # шаблон имени файла
    open (my $fh, ">", $fname);
    for my $i (1..$n_funcs) {
        my $len = $min_len * $all_funcs + rand(($max_len - $min_len) * $all_funcs);
        my %f;
        for (1..$len) {
            $f{make_monom (1 + rand $all_funcs)} = 1;
        }
        say $fh join('+', keys %f);
    }
}

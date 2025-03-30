#!env gnuplot
set terminal pngcairo enhanced font "Arial,12" size 800,600
set output 'rtn_benchmark_results.png'

set title "Append chunks up to 1MB file size (throughput in MB/s)"
set xlabel "Chunk size (bytes)"
set ylabel "Throughput (MB/s)"
set logscale x 2
set grid
set key left top
set xtics nomirror
set ytics nomirror
set format x "2^{%L}"
set datafile separator ","
set key autotitle columnheader

plot "rtn_benchmark_results.csv" using 1:2 with linespoints lw 2 title "MMKV*", \
     "rtn_benchmark_results.csv" using 1:3 with linespoints lw 2 title "EncryptedMMKV*", \
     "rtn_benchmark_results.csv" using 1:4 with linespoints lw 2 title "Mmfile", \
     "rtn_benchmark_results.csv" using 1:5 with linespoints lw 2 title "EncryptedMmfile", \
     "rtn_benchmark_results.csv" using 1:6 with linespoints lw 2 title "ReactNativeFS"

set terminal pngcairo enhanced font "Arial,12" size 800,600
set output 'benchmark_results.png'

set title "File Append Benchmark"
set xlabel "Chunk Size (bytes)"
set ylabel "Throughput (MB/s)"
set logscale x 2
set grid
set key left top
set xtics nomirror
set ytics nomirror
set format x "2^{%L}"
set datafile separator ","
set key autotitle columnheader

plot "benchmark_results.csv" using 1:2 with linespoints lw 2 title "File Append", \
     "benchmark_results.csv" using 1:3 with linespoints lw 2 title "MMap Append", \
     "benchmark_results.csv" using 1:4 with linespoints lw 2 title "MMap Encrypted Append"

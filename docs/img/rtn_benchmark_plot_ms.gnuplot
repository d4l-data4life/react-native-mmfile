#!env gnuplot
set terminal pngcairo enhanced font "Arial,12" size 800,600
set output 'rtn_benchmark_results_ms.png'

set title "Append chunks up to 1MB file size (total time in ms)"
set xlabel "Chunk size"
set ylabel "Total time (ms)"
set logscale x 2
set logscale y 2
set grid
set key top right
set xtics ( "16B" 16, "64B" 64, "256B" 256, "1kB" 1024, "4kB" 4096, "16kB" 16384, "64kB" 65536,"256kB" 262144, "1MB" 1048576 )

set ytics nomirror

set datafile separator ","
set key autotitle columnheader

plot "rtn_benchmark_results_ms.csv" using 1:2 with linespoints lw 2 title "MMKV*", \
     "rtn_benchmark_results_ms.csv" using 1:3 with linespoints lw 2 title "EncryptedMMKV*", \
     "rtn_benchmark_results_ms.csv" using 1:4 with linespoints lw 2 title "Mmfile", \
     "rtn_benchmark_results_ms.csv" using 1:5 with linespoints lw 2 title "EncryptedMmfile", \
     "rtn_benchmark_results_ms.csv" using 1:6 with linespoints lw 2 title "ReactNativeFS"

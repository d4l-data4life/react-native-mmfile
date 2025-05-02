#!env gnuplot
set terminal svg enhanced font "Arial,14" size 800,600 dynamic
set output 'rtn_benchmark_results.svg'

# Use neutral mid-gray for all text with moderate sizes
set title "Append chunks up to 1MB file size (throughput in MB/s)" textcolor rgb "#7f7f7f" font ",16"
set xlabel "Chunk size (bytes)" textcolor rgb "#7f7f7f" font ",14"
set ylabel "Throughput (MB/s)" textcolor rgb "#7f7f7f" font ",14"
set tics textcolor rgb "#7f7f7f"
set key textcolor rgb "#7f7f7f"

# Grid and axis configuration
set logscale x 2
set grid front lc rgb "#888888" lw 1
set key left top
set xtics nomirror
set ytics nomirror
set format x "2^{%L}"
set datafile separator ","
set key autotitle columnheader

# Define line styles with contrasting colors
set style line 1 lt 1 lw 2 lc rgb "#1f77b4" pt 7
set style line 2 lt 1 lw 2 lc rgb "#ff7f0e" pt 7
set style line 3 lt 1 lw 2 lc rgb "#2ca02c" pt 7
set style line 4 lt 1 lw 2 lc rgb "#d62728" pt 7
set style line 5 lt 1 lw 2 lc rgb "#9467bd" pt 7

plot "rtn_benchmark_results.csv" using 1:2 with linespoints ls 1 title "MMKV*", \
     "rtn_benchmark_results.csv" using 1:3 with linespoints ls 2 title "EncryptedMMKV*", \
     "rtn_benchmark_results.csv" using 1:4 with linespoints ls 3 title "Mmfile", \
     "rtn_benchmark_results.csv" using 1:5 with linespoints ls 4 title "EncryptedMmfile", \
     "rtn_benchmark_results.csv" using 1:6 with linespoints ls 5 title "ReactNativeFS"
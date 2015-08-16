set datafile separator ","
set timefmt "%d:%H:%M:%S"
set xdata time
set xtics format "%H:%M"
set   autoscale                        # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set title "Test Plot of data from arduino"
set xlabel "Time(h:m)"
set ylabel "Light sensor(0-255)"

plot 	"data.txt" using 1:2 t 'T', \
	"data.txt" using 1:3 t 'RH', \
	"data.txt" using 1:7 t 'R', \
	"data.txt" using 1:8 t 'G', \
	"data.txt" using 1:9 t 'B', \
	"data.txt" using 1:($5+$6/100) t 'pH'#, \
	#"data.txt" using 1:10 t 'interval', \
	#"data.txt" using 1:11 t 'duration'
gnuplot lidar_plot.plt --persist 
while true; do
./a.out --channel --serial /dev/ttyUSB0 115200
done

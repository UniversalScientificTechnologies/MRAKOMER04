file1 = "M41.log"
file2 = "M40.log"
set xdata time
set timefmt "%s"
set format x "%H:%M"
#set terminal x11 persist
set autoscale xy
plot file1 using ($1+60*60*2):($6/100) with lines title 'Ta 41' 1, \
  file1 using ($1+60*60*2):($7/100) with lines title 'To1' 2, \
  file1 using ($1+60*60*2):($8/100) with lines title 'To2' 3, \
  file2 using ($1+60*60*2):($6/100) with lines title 'Ta 40' 4, \
  file2 using ($1+60*60*2):($7/100) with lines title 'To1' 5
#  file using 0:2 '$M4.1 %lf %lf %lf %lf' with lines title 'To1' 2, \
#  file using 0:3 '$M4.1 %lf %lf %lf %lf' with lines title 'To2' smooth bezier 3
  
set terminal png small color
set size 1,0.75
set output "OUTPUTFILE"
set timestamp
set xrange [0:24]
set xtics 0,2,24
set key top left Left reverse
set grid
set lmargin 8
plot "/home/bbs/dayonline" u 1:2 title "total" with l 1,\
     "/home/bbs/dayonline" u 1:3 title "www guest" with l 2,\
     "/home/bbs/dayonline" u 1:4 title "www not guest" with l 3,\
     "/home/bbs/dayonline" u 1:5 title "telnet" with l 4,\
     "/home/bbs/dayonline" u 1:6 title "www guest in tsinghua" with l 8,\
     "/home/bbs/dayonline" u 1:7 title "www not guest in tsinghua" with l 5,\
     "/home/bbs/dayonline" u 1:8 title "telnet in tsinghua" with l 0

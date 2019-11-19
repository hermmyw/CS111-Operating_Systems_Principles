#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2b_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2b_list-1.png ... cost per operation vs threads and iterations
#	lab2b_list-2.png ... threads and iterations that run (un-protected) w/o failure
#	lab2b_list-3.png ... threads and iterations that run (protected) w/o failure
#	lab2b_list-4.png ... cost per operation vs number of threads
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# List-1: throughput vs. number of threads for mutex and spin-lock synchronized list operations
set title "List-1: Throughput vs. Number of threads"
set xlabel "Number of threads"
set logscale x 2
unset xrange
set xrange [0.75:32]
set ylabel "Throughput"
set logscale y
set output 'lab2b_1.png'

# 
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'spin-lock' with linespoints lc rgb 'green'


# List-2: mean time per mutex wait and mean time per operation for mutex-synchronized list operations.
set title "List-2: Wait time vs. cost per operation"
set xlabel "Number of threads"
set logscale x 2
unset xrange
set xrange [0.75:32]
set ylabel "time"
set logscale y
set output 'lab2b_2.png'

# 
plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'wait time/operation' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'cost/operation' with linespoints lc rgb 'green'


#successful iterations vs. threads for each synchronization method.
set title "List-3: Successful iterations for synchronized method"
set xlabel "Number of threads"
set logscale x 2
unset xrange
set xrange [0.75:16]
set ylabel "Number of iterations"
set logscale y
set output 'lab2b_3.png'

# 
plot \
     "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'mutex' with points lc rgb 'green', \
     "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'spin-lock' with points lc rgb 'blue'



#throughput vs. number of threads for mutex synchronized partitioned lists.
set title "List-4: Throughput vs. number of threads(mutex)"
set xlabel "Number of threads"
set logscale x 2
unset xrange
set xrange [0.75:32]
set ylabel "Throughput"
set logscale y
set output 'lab2b_4.png'

# 
plot \
     "< grep 'list-none-m,[0-9]*,[0-9]*,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 list' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 lists' with linespoints lc rgb 'green', \
	 "< grep 'list-none-m,[0-9]*,[0-9]*,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 lists' with linespoints lc rgb 'orange', \
     "< grep 'list-none-m,[0-9]*,[0-9]*,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 lists' with linespoints lc rgb 'blue'

#throughput vs. number of threads for spin-lock synchronized partitioned lists.
set title "List-4: Throughput vs. number of threads(spin-lock)"
set xlabel "Number of threads"
set logscale x 2
unset xrange
set xrange [0.75:32]
set ylabel "Throughput"
set logscale y
set output 'lab2b_5.png'

# 
plot \
     "< grep 'list-none-s,[0-9]*,[0-9]*,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 list' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 lists' with linespoints lc rgb 'green', \
	 "< grep 'list-none-s,[0-9]*,[0-9]*,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 lists' with linespoints lc rgb 'orange', \
     "< grep 'list-none-s,[0-9]*,[0-9]*,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 lists' with linespoints lc rgb 'blue'

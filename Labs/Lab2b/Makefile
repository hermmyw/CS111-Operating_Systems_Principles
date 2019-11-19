all:
	gcc -lpthread -Wall -Wextra -g lab2_list.c SortedList.c -o lab2_list

profile:
	rm -f ./raw.gperf
	LD_PRELOAD=/u/cs/ugrad/huimin/lib/libprofiler.so.0 CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=thread_list ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf	

tests:
	@chmod +x test-list.sh
	@chmod +x lab2_list.gp 
	@-./test-list.sh > lab2b_list.csv 2> err_list.txt
	@rm -f *txt

graphs:
	@-./lab2_list.gp 

clean:
	rm -f lab2_list *.tar.gz


dist:
	make
	make tests
	make graphs
	make profile
	tar -czf lab2b-704978214.tar.gz README Makefile profile.out \
	lab2_list.c \
	SortedList.h SortedList.c \
	test-list.sh \
	lab2_list.gp \
	*.png *.csv

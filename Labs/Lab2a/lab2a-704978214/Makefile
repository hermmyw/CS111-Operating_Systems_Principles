all:
	gcc -pthread -Wall -Wextra -g lab2_add.c -o lab2_add
	gcc -pthread -Wall -Wextra -g lab2_list.c SortedList.c -o lab2_list

tests:
	@chmod +x test-add.sh
	@chmod +x test-list.sh
	@chmod +x lab2_add.gp
	@chmod +x lab2_list.gp 
	@-./test-add.sh > lab2_add.csv 2> err_add.txt
	@-./test-list.sh > lab2_list.csv 2> err_list.txt
	@rm -f *txt

graphs:
	@-./lab2_add.gp 
	@-./lab2_list.gp 

clean:
	rm -f lab2_add lab2_list *.tar.gz


dist:
	make
	make tests
	make graphs
	tar -czf lab2a-704978214.tar.gz README Makefile \
	lab2_add.c lab2_list.c \
	SortedList.h SortedList.c \
	test-list.sh test-add.sh \
	lab2_add.gp lab2_list.gp \
	*.png *.csv

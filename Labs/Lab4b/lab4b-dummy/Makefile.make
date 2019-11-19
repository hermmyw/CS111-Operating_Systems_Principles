all:
	gcc -lm -Wall -Wextra -g lab4b.c -o lab4b

check:
	@./lab4b --period=-1 --scale=C --log="LOGFILE" ; \
	if [ $$? -eq 1 ] ;\
	then echo "Success" ;\
	fi;
	@./lab4b --period=2 --scale=C --log= ; \
	if [ $$? -eq 1 ] ;\
	then echo "Success" ;\
	fi;
	@./lab4b --wrong ; \
	if [ $$? -eq 1 ] ;\
	then echo "Success" ;\
	fi;


clean:
	rm -f lab4b *.tar.gz


dist:
	tar -czf lab4b-704978214.tar.gz README Makefile lab4b.c
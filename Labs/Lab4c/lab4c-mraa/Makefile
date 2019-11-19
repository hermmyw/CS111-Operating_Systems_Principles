all:
	gcc -lmraa -lm -Wall -Wextra -g lab4c_tcp.c -o lab4c_tcp
	gcc -lmraa -lm -Wall -Wextra -g lab4c_tls.c -o lab4c_tls -lssl -lcrypto

check:
	@./lab4c_tcp --period=-1 --scale=C --log="LOGFILE" ; \
	if [ $$? -eq 1 ] ;\
	then echo "Success" ;\
	fi;
	@./lab4c_tcp --period=2 --scale=C --log= ; \
	if [ $$? -eq 1 ] ;\
	then echo "Success" ;\
	fi;
	@./lab4c_tcp --wrong ; \
	if [ $$? -eq 1 ] ;\
	then echo "Success" ;\
	fi;

	@./lab4c_tls --period=-1 --scale=C --log="LOGFILE" ; \
	if [ $$? -eq 1 ] ;\
	then echo "Success" ;\
	fi;
	@./lab4c_tls --period=2 --scale=C --log= ; \
	if [ $$? -eq 1 ] ;\
	then echo "Success" ;\
	fi;
	@./lab4c_tls --wrong ; \
	if [ $$? -eq 1 ] ;\
	then echo "Success" ;\
	fi;

	@rm LOGFILE


clean:
	rm -f lab4c_tcp lab4c_tls *.tar.gz


dist:
	tar -czf lab4c-704978214.tar.gz README Makefile lab4c_tcp.c lab4c_tls.c
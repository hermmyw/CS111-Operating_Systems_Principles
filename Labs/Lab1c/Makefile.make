all:
	gcc -g -Wall -Wextra simpsh.c -o simpsh

check:
	@echo "Testing..."
	@touch 1.txt 2.txt 3.txt 4.txt 5.txt
	@echo "abc"  > 1.txt


	@./simpsh \
	  --profile \
	  --rdonly 1.txt \
	  --pipe \
	  --pipe \
	  --creat --trunc --wronly c.txt \
	  --creat --append --wronly d.txt \
	  --command 3 5 6 tr A-Z a-z \
	  --command 0 2 6 sort \
	  --command 1 4 6 cat b - \
	  --close 2 \
	  --close 4 \
	  --wait >o.txt 2>e.txt; \
	if [ $$? -eq 1 ] \
	 && grep -q "exit 0 sort" o.txt \
	 && grep -q "exit 0 tr A-Z a-z" o.txt \
	 && grep -q "exit 1 cat b -" o.txt ;\
	then \
		echo "test 1 passed" ;\
	else \
		echo "test 1 failed" ;\
	fi
	@cat o.txt


	@./simpsh \
	  --profile \
	  --rdonly a \
	  --pipe \
	  --pipe \
	  --creat --trunc --wronly e.txt \
	  --creat --append --wronly f.txt \
	  --command 3 5 6 bash -c "exit 17" \
	  --command 0 2 6 bash -c "exit 30" \
	  --command 1 4 6 bash -c "exit 45" \
	  --close 2 \
	  --close 4 \
	  --wait >o.txt 2>e.txt; \
	if [ $$? -eq 45 ] ;\
	then \
		echo "test 2 passed" ;\
	else \
		echo "test 2 failed" ;\
	fi
	@cat o.txt


	@./simpsh \
	  --profile \
	  --default 11 \
	  --catch 11 \
	  --abort >o.txt 2>e.txt;\
	if [ $$? -eq 11 ] ;\
	then \
		echo "test 3 passed" ;\
	else \
		echo "test 3 failed" ;\
	fi
	@cat o.txt

	@./simpsh \
	  --profile \
	  --catch 11 \
	  --default 11 \
	  --abort >o.txt 2>e.txt ;\
	if [ $$? -eq 139 ] ;\
	then \
		echo "test 4 passed" ;\
	else \
		echo "test 4 failed" ;\
	fi
	@cat o.txt


	@./simpsh \
	--profile \
	--rdonly 1.txt \
	--wronly 2.txt \
	--wronly 3.txt \
	--close 0 \
	--command 0 1 2 cat >o.txt 2>e.txt;\
	if [ $$? -eq 1 ] ;\
	then \
		echo "test 5 passed" ;\
	else \
		echo "test 5 failed" ;\
	fi
	@cat o.txt



clean:
	rm -f simpsh *.txt *.tar.gz

dist:
	tar -czf lab1-704978214.tar.gz README Makefile simpsh.c report.pdf
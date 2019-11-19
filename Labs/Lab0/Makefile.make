all:
	gcc -g -Wall -Wextra lab0.c -o lab0

#check
check:
	@echo "abcdefg" > in1.txt
	@echo "" > in2.txt
	@echo -e "\n" > in3.txt
	@echo "xyz" > out.txt

	#correctly copied...
	@./lab0 --input=in1.txt --output=out.txt
	@diff -q in1.txt out.txt
	@if [ $$? -eq 0 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	@./lab0 --input=in2.txt --output=out.txt
	@diff -q in2.txt out.txt
	@if [ $$? -eq 0 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	@./lab0 --input=in3.txt --output=out.txt
	@diff -q in3.txt out.txt
	@if [ $$? -eq 0 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#output to stdout...
	@./lab0 --input=in3.txt > out.txt
	@diff -q in3.txt out.txt
	@if [ $$? -eq 0 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#input from piplining...
	@echo "abc" | ./lab0 > out.txt
	@echo "abc" > in4.txt
	@diff -q in4.txt out.txt
	@if [ $$? -eq 0 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#--input overwrites stdin...
	@cat in4.txt | ./lab0 --input=in1.txt --output=out.txt
	@diff -q in1.txt out.txt
	@if [ $$? -eq 0 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#--output overwrites stdout...
	@./lab0 --input=in1.txt --output=out.txt > o.txt
	@diff -q in1.txt out.txt
	@if [ $$? -eq 0 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	@./lab0 --input=in1.txt > o.txt --output=out.txt
	@diff -q in1.txt out.txt 
	@if [ $$? -eq 0 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#multiple inputs and outputs
	@./lab0 --input=in1.txt --output=out1.txt --input=in4.txt --output=out4.txt
	@diff -q in4.txt out4.txt
	@if [ $$? -eq 0 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	@touch empty.txt
	@diff -q empty.txt out1.txt
	@if [ $$? -eq 0 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#unspecified input file...
	@./lab0 --input ; \
	if [ $$? -eq 1 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#unspecified input file, treat the next arg as the input file...
	@./lab0 --input --output=out.txt ; \
	if [ $$? -eq 2 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#nonexistent input file...
	@./lab0 --input=nonexistent.txt ; \
	if [ $$? -eq 2 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#unreadable/unopenable input file...
	@echo "m" > unreadable.txt
	@chmod -r unreadable.txt
	@./lab0 --input=unreadable.txt ; \
	if [ $$? -eq 2 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#unwrittable output file...
	@touch unwrittable.txt
	@chmod -w unwrittable.txt
	@./lab0 --input=in1.txt --output=unwrittable.txt ; \
	if [ $$? -eq 3 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#unspecified input and output, --input treats the next arg as the input file...
	@./lab0 --input --output ; \
	if [ $$? -eq 2 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#segmentation fault...
	@echo "123" > in.txt
	@./lab0 --segfault ; \
	if [ $$? -eq 139 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	@./lab0 --segfault --input=in.txt --output=out.txt ; \
	if [ $$? -eq 139 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	@./lab0 --input=in.txt --output=out.txt --segfault ; \
	if [ $$? -eq 139 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	@./lab0 --segfault --input=nonexistent.txt --output=out.txt ; \
	if [ $$? -eq 139 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	@./lab0 --segfault --catch --input=nonexistent.txt ; \
	if [ $$? -eq 139 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#input error comes before segfault...
	@./lab0 --input=nonexistent.txt --segfault ; \
	if [ $$? -eq 2 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#catch segfault...
	@./lab0 --catch --segfault ; \
	if [ $$? -eq 4 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	@./lab0 --catch --segfault --dump-core; \
	if [ $$? -eq 4 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	#dump-core negates catch...
	@./lab0 --catch --dump-core --segfault ; \
	if [ $$? -eq 139 ]; \
	then \
		echo "..."; \
	else \
		echo "FAILED!!!"; \
	fi

	@echo "All test cases passed."


#clean
clean:
	rm -f lab0 *.txt *.tar.gz

#build the distribution tarball
dist:
	tar -czf lab0-704978214.tar.gz README Makefile lab0.c backtrace.png breakpoint.png

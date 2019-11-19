all:
		gcc -g -Wall -Wextra lab1a.c -o simpsh

check:
		@echo "Testing..."
		@touch a.txt b.txt c.txt d.txt e.txt f.txt
		@echo abc > a.txt

		@echo "Test 1: invalid file number"
		@./simpsh --rdonly a.txt --wronly b.txt --wronly c.txt --wronly d.txt --command q -1 ecd cat 2> err; \
		if [ $$? -eq 1 ] && [ -s err ]; \
		then \
				echo "" ;\
		else \
				echo "FAILED!!!"; \
				echo ""; \
		fi

		@echo "Test 2: less than 3 file number"
		@./simpsh --rdonly a.txt --wronly b.txt --wronly c.txt --wronly d.txt --command 0 2 cat 2> err; \
		if [ $$? -eq 1 ] && [ -s err ]; \
		then \
				echo "" ;\
		else \
				echo "FAILED!!!"; \
				echo "" ; \
		fi

		@echo "Test 3: wrong subcommand option"
		@./simpsh --rdonly a.txt --wronly b.txt --wronly c.txt --command 0 2 1 sleep a 2> err
		@echo "message:"
		@cat b.txt

		@echo "Test 4: unreadable file"
		@chmod -r a.txt
		@./simpsh --rdonly a.txt --wronly b.txt --wronly c.txt --wronly d.txt --command 0 2 1 sleep ab 2> err; \
		if [ $$? -eq 0 ]; \
		then \
			echo "FAILED!!!" ;\
		fi
		@if [ -s err ]; \
		then \
			cat err ; \
		else \
			echo "FAILED!!!" ;\
		fi

		@echo "Test 5: insufficient arguments after command"
		@./simpsh --rdonly a.txt --wronly b.txt --wronly c.txt --command 0 2 1 --wronly d.txt 2> err; \
		if [ $$? -eq 1 ] && [ -s err ]; \
		then \
			echo "" ;\
		else \
			echo "FAILED!!!" ;\
		fi

		@echo ""
		@echo "Finished"

		@rm *.txt
clean:
		rm -f lab1a *.txt *.tar.gz

dist:
		tar -czf lab1a-704978214.tar.gz README Makefile lab1a.c


all:
	cp lab3b.py lab3b
	chmod +x lab3b

tests:
	make
	@./lab3b mycsv.csv
	@rm -f *.csv

clean:
	rm -f *.tar.gz lab3b

dist:
	make
	make tests
	tar -czf lab3b-704978214.tar.gz README Makefile lab3b.py mycsv.csv
#lab2_add-1.png
./lab2_add --iteration=10 --threads=2
./lab2_add --iteration=10 --threads=4
./lab2_add --iteration=10 --threads=8
./lab2_add --iteration=10 --threads=12
./lab2_add --iteration=20 --threads=2
./lab2_add --iteration=20 --threads=4
./lab2_add --iteration=20 --threads=8
./lab2_add --iteration=20 --threads=12
./lab2_add --iteration=40 --threads=2
./lab2_add --iteration=40 --threads=4
./lab2_add --iteration=40 --threads=8
./lab2_add --iteration=40 --threads=12
./lab2_add --iteration=80 --threads=2
./lab2_add --iteration=80 --threads=4
./lab2_add --iteration=80 --threads=8
./lab2_add --iteration=80 --threads=12
./lab2_add --iteration=100 --threads=2
./lab2_add --iteration=100 --threads=4
./lab2_add --iteration=100 --threads=8
./lab2_add --iteration=100 --threads=12
./lab2_add --iteration=1000 --threads=2
./lab2_add --iteration=1000 --threads=4
./lab2_add --iteration=1000 --threads=8
./lab2_add --iteration=1000 --threads=12
./lab2_add --iteration=10000 --threads=2
./lab2_add --iteration=10000 --threads=4
./lab2_add --iteration=10000 --threads=8
./lab2_add --iteration=10000 --threads=12
./lab2_add --iteration=100000 --threads=2
./lab2_add --iteration=100000 --threads=4
./lab2_add --iteration=100000 --threads=8
./lab2_add --iteration=100000 --threads=12


#yield-none
#lab2_add-1.png
./lab2_add --iteration=10 --threads=1 --yield
./lab2_add --iteration=20 --threads=1 --yield
./lab2_add --iteration=80 --threads=1 --yield
./lab2_add --iteration=100 --threads=1 --yield
./lab2_add --iteration=1000 --threads=1 --yield
./lab2_add --iteration=10000 --threads=1 --yield
./lab2_add --iteration=100000 --threads=1 --yield

./lab2_add --iteration=10 --threads=2 --yield
./lab2_add --iteration=10 --threads=4 --yield
./lab2_add --iteration=10 --threads=8 --yield
./lab2_add --iteration=10 --threads=12 --yield
./lab2_add --iteration=20 --threads=2 --yield
./lab2_add --iteration=20 --threads=4 --yield
./lab2_add --iteration=20 --threads=8 --yield
./lab2_add --iteration=20 --threads=12 --yield
./lab2_add --iteration=40 --threads=2 --yield
./lab2_add --iteration=40 --threads=4 --yield
./lab2_add --iteration=40 --threads=8 --yield
./lab2_add --iteration=40 --threads=12 --yield
./lab2_add --iteration=80 --threads=2 --yield
./lab2_add --iteration=80 --threads=4 --yield
./lab2_add --iteration=80 --threads=8 --yield
./lab2_add --iteration=80 --threads=12 --yield
./lab2_add --iteration=100 --threads=2 --yield
./lab2_add --iteration=100 --threads=4 --yield
./lab2_add --iteration=100 --threads=8 --yield
./lab2_add --iteration=100 --threads=12 --yield
./lab2_add --iteration=1000 --threads=2 --yield
./lab2_add --iteration=1000 --threads=4 --yield
./lab2_add --iteration=1000 --threads=8 --yield
./lab2_add --iteration=1000 --threads=12 --yield
./lab2_add --iteration=10000 --threads=2 --yield
./lab2_add --iteration=10000 --threads=4 --yield
./lab2_add --iteration=10000 --threads=8 --yield
./lab2_add --iteration=10000 --threads=12 --yield
./lab2_add --iteration=100000 --threads=2 --yield
./lab2_add --iteration=100000 --threads=4 --yield
./lab2_add --iteration=100000 --threads=8 --yield
./lab2_add --iteration=100000 --threads=12 --yield



#lab2_add-3.png
./lab2_add --iteration=1 --threads=1
./lab2_add --iteration=5 --threads=1
./lab2_add --iteration=10 --threads=1
./lab2_add --iteration=50 --threads=1
./lab2_add --iteration=100 --threads=1
./lab2_add --iteration=500 --threads=1
./lab2_add --iteration=1000 --threads=1
./lab2_add --iteration=5000 --threads=1
./lab2_add --iteration=10000 --threads=1



#lab2_add-4.png
#yield-m
./lab2_add --iteration=10000 --threads=1 --yield --sync=m
./lab2_add --iteration=10000 --threads=2 --yield --sync=m
./lab2_add --iteration=10000 --threads=4 --yield --sync=m
./lab2_add --iteration=10000 --threads=8 --yield --sync=m
./lab2_add --iteration=10000 --threads=12 --yield --sync=m

#yield-s
./lab2_add --iteration=1000 --threads=1 --yield --sync=s
./lab2_add --iteration=1000 --threads=2 --yield --sync=s
./lab2_add --iteration=1000 --threads=4 --yield --sync=s
./lab2_add --iteration=1000 --threads=8 --yield --sync=s
./lab2_add --iteration=1000 --threads=12 --yield --sync=s

#yield-c
./lab2_add --iteration=10000 --threads=1 --yield --sync=c
./lab2_add --iteration=10000 --threads=2 --yield --sync=c
./lab2_add --iteration=10000 --threads=4 --yield --sync=c
./lab2_add --iteration=10000 --threads=8 --yield --sync=c
./lab2_add --iteration=10000 --threads=12 --yield --sync=c





#lab2_add-5.png
#m
./lab2_add --iteration=10000 --threads=1 --sync=m
./lab2_add --iteration=10000 --threads=2 --sync=m
./lab2_add --iteration=10000 --threads=4 --sync=m
./lab2_add --iteration=10000 --threads=8 --sync=m
./lab2_add --iteration=10000 --threads=12 --sync=m

#s
./lab2_add --iteration=10000 --threads=1 --sync=s
./lab2_add --iteration=10000 --threads=2 --sync=s
./lab2_add --iteration=10000 --threads=4 --sync=s
./lab2_add --iteration=10000 --threads=8 --sync=s
./lab2_add --iteration=10000 --threads=12 --sync=s

#c
./lab2_add --iteration=10000 --threads=1 --sync=c
./lab2_add --iteration=10000 --threads=2 --sync=c
./lab2_add --iteration=10000 --threads=4 --sync=c
./lab2_add --iteration=10000 --threads=8 --sync=c
./lab2_add --iteration=10000 --threads=12 --sync=c







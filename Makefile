all : ruzzle
	
ruzzle : main.o
	g++ main.cpp -lrt -o ruzzle
	
clean:
	rm -f *.o


#makefile
# makefile

all: shell

shell.o: shell.cpp
	g++ -g -w -std=c++11 -c shell.cpp

shell: shell.o
	g++ -g -w -o shell -std=c++11 shell.o

clean:
	rm *.o shell
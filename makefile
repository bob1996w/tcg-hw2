CPLUSPLUS = g++ -std=c++11 -O2 -Wall
DEL = rm -f

all: src/agent.cpp
	mkdir -p target
	$(CPLUSPLUS) src/*.cpp -c
	$(CPLUSPLUS) agent.o -o target/agent
	$(DEL) *.o

clean:
	$(DEL) *.o
	$(DEL) -r target

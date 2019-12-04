CPLUSPLUS = g++ -std=c++11 -O2 -Wall
DEL = rm -f

all: src/agent.cpp board.cpp
	mkdir -p target
	$(CPLUSPLUS) src/agent.cpp -o target/agent
	$(CPLUSPLUS) -D LOG src/agent.cpp -o target/agent-log
	$(DEL) *.o

testRandom: src/testRandom.cpp
	mkdir -p target
	$(CPLUSPLUS) src/testRandom.cpp -o target/testRandom

clean:
	$(DEL) *.o
	$(DEL) -r target

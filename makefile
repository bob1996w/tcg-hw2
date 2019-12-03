CPLUSPLUS = g++ -std=c++11 -O2 -Wall
DEL = rm -f

all: src/agent.cpp
	mkdir -p target
	$(CPLUSPLUS) src/agent.cpp -o target/agent
	$(CPLUSPLUS) -D LOG src/agent.cpp -o target/agent-log
	$(DEL) *.o

clean:
	$(DEL) *.o
	$(DEL) -r target

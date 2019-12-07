CPLUSPLUS = g++ -std=c++11 -O2 -Wall
DEL = rm -f

all: src/agent.cpp src/board.hpp src/agentRT.cpp
	mkdir -p target
	$(CPLUSPLUS) src/agent.cpp -o target/agent
	$(CPLUSPLUS) -D LOG src/agent.cpp -o target/agent-log -g
	$(CPLUSPLUS) src/agentRT.cpp -o target/agentRT
	$(CPLUSPLUS) -D LOG src/agentRT.cpp -o target/agentRT-log -g
	$(CPLUSPLUS) src/agentMCTBasic.cpp -o target/agentMCTBasic
	$(CPLUSPLUS) -D LOG src/agentMCTBasic.cpp -o target/agentMCTBasic-log -g
	$(DEL) *.o

testRandom: src/testRandom.cpp
	mkdir -p target
	$(CPLUSPLUS) src/testRandom.cpp -o target/testRandom

clean:
	$(DEL) *.o
	$(DEL) -r target

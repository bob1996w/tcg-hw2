CPLUSPLUS = g++ -std=c++11 -O2 -Wall
DEL = rm -f

all: src/agent.cpp
	$(CPLUSPLUS) src/*.cpp -c


clean:

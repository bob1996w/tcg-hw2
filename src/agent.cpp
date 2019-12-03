#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>


#include <iostream>
#include <fstream>
#include <chrono>
#include <functional>
#include <vector>

#include "board.hpp"

using namespace std;

fstream flog;
bool myTurn = false;
char ourPlayer;
inline void flipBit(bool &v) { v = !v; }
GameBoard* b;
int num, mov;

void logger(string fileName) {
    flog.open(fileName, fstream::out);
    if (!flog.is_open()) {
        throw runtime_error("error opening log file\n");
    }
}

int main (int argv, char* argc[]) {

    // timer returns 0 (end program) when reset = true is received.
    function <double (bool)> timer = [] (bool reset = false) -> double {
        static decltype(chrono::steady_clock::now()) timerStart, timerEnd;
        if (reset) {
            timerStart = chrono::steady_clock::now();
            return 0;
        }
        timerEnd = chrono::steady_clock::now();
        return (chrono::duration_cast<chrono::duration<double>>(timerEnd - timerStart).count());
    };

    srand(time(NULL));

#ifdef LOG
    logger("myAgent.log.txt");
#endif

    do {
        b = new GameBoard;
        // read initial board from pipe
        b->readBoard();
        // am I first player ('f') or second player ('s') ?
        ourPlayer = getchar();
#ifdef LOG
        flog << *b << endl;
        flog << "ourPlayer: " << ourPlayer << endl << flush;
#endif
        for (myTurn = (ourPlayer == 'f'); ; flipBit(myTurn)) {
            if (myTurn) {

            }

            // not myTurn
            else {
                num = getchar();
                mov = getchar();
                flog << "enemy: " << num << mov << endl;
                b->applyMove(num, mov);
            }
        }

        delete b;
    } while (getchar() == 'y');
    

    return 0;
}
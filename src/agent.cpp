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
int game = 0;
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
        flog << "===== GAME #" << game << " =====" << endl; 
        flog << *b;
        flog << "ourPlayer: " << ourPlayer << endl << endl;
        flog << "=====START=====" << endl;
#endif
        for (myTurn = (ourPlayer == 'f'); b->winner == 2; flipBit(myTurn)) {
            if (myTurn) {
                // auto moves = b->getAllMoves();
                // auto move = moves.at(getUniformIntRand(moves.size()));
                auto move = b->getRandomMove();
#ifdef LOG
                flog << "me: " << b->sendMove(move) << endl << flush;
#endif
                b->applyMove(move);
                cout << b->sendMove(move) << flush;
            }   

            // not myTurn
            else {
                num = getchar() - '0';
                mov = getchar() - '0';
#ifdef LOG
                flog << "enemy: " << (int)num << (int)mov << endl;
#endif
                b->applyMove(num, mov);
            }
#ifdef LOG
            flog << *b << endl << flush;
#endif
        }
#ifdef LOG
        flog << "winner: " << "rb_x"[b->winner] << endl;
        flog << "=====END=====" << endl << flush;
#endif
        delete b;
        game++;
    } while (getchar() == 'y');
    

    return 0;
}
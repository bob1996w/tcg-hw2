#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <fstream>
#include <chrono>
#include <functional>
using namespace std;

fstream flog;
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

    while (true) {
        
    };

    return 0;
}

#include <iostream>
using namespace std;
#include "board.hpp"

int main() {
    for (int i = 0; i < 100; ++i) {
        int n[6] = {0, 1, 2, 3, 4, 5};
        // Fisher–Yates shuffle
        for (int i = 0; i < 5; ++i) {
            swap (n[i], n[i + getUniformIntRandFixedSize(6 - i)]);
        }
        cout << n[0] << " " << n[1] << " " << n[2] << " " << n[3] << " " << n[4] << " " << n[5] << endl;
    }
    return 0;
}
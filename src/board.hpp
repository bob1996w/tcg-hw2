/*
    board: 

 */
#ifndef _BOARD_HPP_
#define _BOARD_HPP_

#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <chrono>
#include <algorithm>

using namespace std;

#define BOARD_WIDTH 6
#define BOARD_HEIGHT 6
#define BOARD_AREA 36
#define PLAYER_NUM 2
#define CUBE_NUM 6

#define ABS(x) ((x > 0)? x : -x)

extern fstream flog; // agent.cpp

struct _space {
    /*
        Cube number uses -6 ~ -1 to place mention enemy's number
        and 1 ~ 6 to denote this player's number
        if this cube is 0 then no one is occupying this block.

        WARNING: the original input is 0 ~ 5 so we need to add 1 before storing.
     */
    int cubeNumber = 0;
    int pos = BOARD_AREA;

    _space () = default;
    // color = 1 or -1 (player), num = 1 ~ 6
    _space (int position, int color = 0, int num = 0) {
        cubeNumber = (color == 0) ? 0: (color == 1) ? num : -num;
        pos = position;
    }
    bool hasCube () { return cubeNumber == 0; }
    int x () { return pos % BOARD_WIDTH; }
    int y () { return pos / BOARD_WIDTH; }

    // TODO: what should be added to space?
};
using Space = _space;

ostream& operator<< (ostream &os, Space &sp) {
    if (sp.cubeNumber == 0) { os << "__"; }
    else if (sp.cubeNumber > 0) { os << "+" << sp.cubeNumber; }
    else {os << sp.cubeNumber; }
    return os;
}

const array<int, CUBE_NUM> init_cube_pos[2] = {
	{ 0,  1,  2,  6,  7, 12},
	{23, 28, 29, 33, 34, 35}};

// TODO: is move needed?
struct _game_board {
    using PII = pair<int, int>;
    using VII = vector<PII>;

    Space board[BOARD_AREA];
    
    void readBoard () {
        string num[2];
        for (int i = 0; i < PLAYER_NUM; ++i) {
            for (int j = 0; j < CUBE_NUM; ++j) {
                num[i] += getchar();
            }
        }
        readBoard(num[0], num[1]);
    }

    void readBoard (string topLeft, string bottomRight) {
        for (int j = 0; j < CUBE_NUM; ++j) {
            board[init_cube_pos[0][j]] = Space(init_cube_pos[0][j], 1, topLeft[j] - '0' + 1);
            board[init_cube_pos[1][j]] = Space(init_cube_pos[1][j], -1, bottomRight[j] - '0' + 1);
        }

    }

    bool isOut (int x, int y) {
        return (x < 0 || y < 0 || x >= BOARD_WIDTH || y >= BOARD_HEIGHT);
    }

    bool isOccupied (int x, int y) {
        return board[y * BOARD_WIDTH + x].hasCube();
    }

    // return (-100, -100) if not found
    PII findCube (int color, int num) {
        int cubeNumber = (color > 0) ? num : -num;
        for (int i = 0; i < BOARD_AREA; ++i) {
            if (board[i].cubeNumber == cubeNumber) {
                return make_pair(i / BOARD_WIDTH, i % BOARD_WIDTH);
            }
        }
        return make_pair(-100, -100);
    }

    // TODO: what should be added to board?
};
using GameBoard = _game_board;

ostream& operator<< (ostream &os, GameBoard &b) {
    for (int i = 0; i < BOARD_AREA; ++i) {
        os << b.board[i] << " \n"[i % BOARD_WIDTH == (BOARD_WIDTH - 1)];
    }
    return os;
}



#endif
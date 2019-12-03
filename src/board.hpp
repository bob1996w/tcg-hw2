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
#define RED 0
#define BLUE 1

#define ABS(x) ((x > 0)? x : -x)

extern fstream flog; // agent.cpp

struct _space {
    /*
        inner: 
        Cube number uses -6 ~ -1 to place mention enemy's number
        and 1 ~ 6 to denote this player's number
        if this cube is 0 then no one is occupying this block.

        from outside: 
        the number is always 0~5 and the color is 0 for red and 1 for blue.
        all the conversion are done inside.
     */
    int cubeNumber = 0;
    int pos = BOARD_AREA;

    _space () {
        cubeNumber = 0;
    };
    // color = 0 (RED) or 1(BLUE) or -1 (empty), num = 0 ~ 5
    _space (int position, int color = 0, int num = 0) {
        setSpace(color, num);
        pos = position;
    }
    void setSpaceEmpty () {
        cubeNumber = 0;
    }
    void setSpace (int color, int num) {
        cubeNumber = (color == -1) ? 0 : (color == 0) ? num + 1 : -num - 1;
    }
    void setSpace (_space &sp) {
        cubeNumber = sp.cubeNumber;
    }
    bool hasCube () { return cubeNumber != 0; }
    bool color () { return cubeNumber < 0; } // no cube returns RED!
    int num () { return ABS(cubeNumber) - 1; }
    int x () { return pos % BOARD_WIDTH; }
    int y () { return pos / BOARD_WIDTH; }

    // TODO: what should be added to space?
};
using Space = _space;

ostream& operator<< (ostream &os, Space &sp) {
    if (!sp.hasCube()) { os << "__"; }
    else if (sp.color() == RED) { os << "+" << sp.num(); }
    else {os << "-" << sp.num(); }
    return os;
}

const array<int, CUBE_NUM> init_cube_pos[2] = {
	{ 0,  1,  2,  6,  7, 12},
	{23, 28, 29, 33, 34, 35}};
const int dx[2][3] = {{0, 1, 1}, {0, -1, -1}};
const int dy[2][3] = {{1, 0, 1}, {-1, 0, -1}};

// TODO: is move needed?
struct _game_board {
    using PII = pair<int, int>;
    using VII = vector<PII>;

    Space board[BOARD_AREA];
    bool turn = 0; // 0 = red (upper left), 1 = blue (lower right)
    int cubesLeft[2] = {6, 6};
    
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
            board[init_cube_pos[0][j]] = Space(init_cube_pos[0][j], RED, topLeft[j] - '0');
            board[init_cube_pos[1][j]] = Space(init_cube_pos[1][j], BLUE, bottomRight[j] - '0');
        }

    }

    bool isOut (int y, int x) {
        return (x < 0 || y < 0 || x >= BOARD_WIDTH || y >= BOARD_HEIGHT);
    }

    bool isOccupied (int y, int x) {
        return board[y * BOARD_WIDTH + x].hasCube();
    }

    void nextTurn () { turn = !turn; }

    // return (-100, -100) if not found
    PII findCube (int color, int num) {
        int cubeNumber = (color == RED) ? num + 1: -num - 1;
        for (int i = 0; i < BOARD_AREA; ++i) {
            if (board[i].cubeNumber == cubeNumber) {
                cout << i << endl;
                return make_pair(i / BOARD_WIDTH, i % BOARD_WIDTH);
            }
        }
        return make_pair(-100, -100);
    }


    void applyMove (int num, int dir) {
        if (num == 15 && dir == 15) { // ??: pass
            nextTurn();
            return;
        }
        PII pos = findCube(turn, num);
        int yy = pos.first + dy[turn][dir];
        int xx = pos.second + dx[turn][dir];
        int currentPos = pos.first * BOARD_WIDTH + pos.second;
        int nextPos = yy * BOARD_WIDTH + xx;
        if (isOccupied(yy, xx)) {
            // eat
            (board[nextPos].color() == RED)? cubesLeft[RED]-- : cubesLeft[BLUE]--;
        }
        // swap the cubes
        board[nextPos].setSpace(board[currentPos]);
        board[currentPos].setSpaceEmpty();
        nextTurn();
    }

    // TODO: what should be added to board?
};
using GameBoard = _game_board;

ostream& operator<< (ostream &os, GameBoard &b) {
    os << "CubeLeft: +RED " << b.cubesLeft[RED] << ", -BLUE " << b.cubesLeft[BLUE] << endl;
    for (int i = 0; i < BOARD_AREA; ++i) {
        os << b.board[i] << " \n"[i % BOARD_WIDTH == (BOARD_WIDTH - 1)];
    }
    return os;
}



#endif
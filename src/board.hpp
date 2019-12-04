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

struct _cube {
    /*
        inner: 
        Cube number uses -6 ~ -1 to place mention enemy's number
        and 1 ~ 6 to denote this player's number

        from outside: 
        the number is always 0~5 and the color is 0 for red and 1 for blue.
        all the conversion are done inside.

        position:
        pos = -1: removed from board
     */
    int cubeNumber = 0;
    int pos = -1;
    
    _cube () = default;
    _cube (int position, int cn) {
        cubeNumber = cn;
        pos = position;
    }
    // color = 0 (RED) or 1 (BLUE), num = 0 ~ 5
    _cube (int position, int color, int num) {
        setCube(color, num);
        pos = position;
    }
    void setCube (int color, int num) {
        cubeNumber = (color == 0)? num + 1 : -num - 1;
    }
    void setPos (int position) {
        pos = position;
    }
    void setCube (_cube *c) {
        cubeNumber = c->cubeNumber;
        pos = c->pos;
    }
    void remove () {
        pos = -1;
    }
    bool color () { return cubeNumber < 0; }
    int num () { return ABS(cubeNumber) - 1; }
    int isOnBoard() { return pos >= 0; }
    int x () { return pos % BOARD_WIDTH; }
    int y () { return pos / BOARD_WIDTH; }

    string printDetail () {
        return ((color() == RED) ? " RED +" : "BLUE -") + to_string(num()) + (isOnBoard()? " y=" + to_string(y()) + ", x=" + to_string(x()) : " Removed");
    }
};
using Cube = _cube;

ostream& operator<< (ostream &os, Cube *c) {
    if (c->color() == RED) { os << "+"; }
    else { os << "-"; }
    os << c->num();
    return os;
}

struct _space {
    Cube *c = nullptr;
    int pos = BOARD_AREA;

    _space () = default;
    // color = 0 (RED) or 1(BLUE) or -1 (empty), num = 0 ~ 5
    _space (int position, int color, int num) {
        c = new Cube(color, num);
        pos = position;
    }
    _space (int position, Cube* cube = nullptr) {
        c = cube;
        pos = position;
    }
    void setSpaceEmpty () {
        c = nullptr;
    }
    void removeCube () {
        if (hasCube()) {
            c->remove();
            c = nullptr;
        }
    }
    void moveCubeFrom (_space &sp) {
        removeCube();
        c = sp.c;
        c->setPos(pos);
    }
    bool hasCube () { return c != nullptr; }
    int x () { return pos % BOARD_WIDTH; }
    int y () { return pos / BOARD_WIDTH; }
    // TODO: what should be added to space?
};
using Space = _space;

ostream& operator<< (ostream &os, Space &sp) {
    if (!sp.hasCube()) { os << "__"; }
    else { os << sp.c; }
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
    int cubesLeft[2] = {CUBE_NUM, CUBE_NUM};
    Cube initialCubes[PLAYER_NUM][CUBE_NUM];
    Cube cubes[PLAYER_NUM][CUBE_NUM];
    
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
        for (int i = 0; i < CUBE_NUM; ++i) {
            int cubeNumTop = topLeft[i] - '0';
            int cubeNumBtm = bottomRight[i] - '0';
            initialCubes[RED][cubeNumTop] = Cube(init_cube_pos[RED][i], RED, cubeNumTop);
            initialCubes[BLUE][cubeNumBtm] = Cube(init_cube_pos[BLUE][i], BLUE, cubeNumBtm);
        }
        resetBoard();
    }

    void resetBoard () {
        for (int i = 0; i < BOARD_AREA; ++i) {
            board[i] = Space(i);
        }
        for (int i = 0; i < CUBE_NUM; ++i) {
            cubes[RED][i] = initialCubes[RED][i];
            cubes[BLUE][i] = initialCubes[BLUE][i];
            board[cubes[RED][i].pos] = Space(cubes[RED][i].pos, &cubes[RED][i]);
            board[cubes[BLUE][i].pos] = Space(cubes[BLUE][i].pos, &cubes[BLUE][i]);
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
        if (cubes[color][num].pos != -1) {
            int pos = cubes[color][num].pos;
            return make_pair(pos / BOARD_WIDTH, pos % BOARD_WIDTH);
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
            (board[nextPos].c->color() == RED)? cubesLeft[RED]-- : cubesLeft[BLUE]--;
        }
        // swap the cubes
        board[nextPos].moveCubeFrom(board[currentPos]);
        board[currentPos].setSpaceEmpty();
        nextTurn();
    }

    // TODO: what should be added to board?
};
using GameBoard = _game_board;

ostream& operator<< (ostream &os, GameBoard &b) {
    os << "Turn: " << ((b.turn == RED) ? "RED" : "BLUE") << endl;
    for (int p = 0; p < PLAYER_NUM; ++p) {
        for (int j = 0; j < CUBE_NUM; ++j) {
            os << b.cubes[p][j].printDetail() << endl;
        }
    }
    os << "CubeLeft: +RED " << b.cubesLeft[RED] << ", -BLUE " << b.cubesLeft[BLUE] << endl;
    for (int i = 0; i < BOARD_AREA; ++i) {
        os << b.board[i] << " \n"[i % BOARD_WIDTH == (BOARD_WIDTH - 1)];
    }
    return os;
}



#endif
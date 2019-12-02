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

extern fstream flog; // agent.cpp

struct _space {
    /*
        Cube number uses -6 ~ -1 to place mention enemy's number
        and 1 ~ 6 to denote this player's number
        if this cube is 0 then no one is occupying this block.
     */
    int cubeNumber;

    _space () = default;
    // TODO: what should be added to space?
};
typedef struct _space Space;


// TODO: is move needed?
struct _game_board {
    Space board[BOARD_AREA];
    // TODO: what should be added to board?
};
typedef struct _game_board GameBoard;



#endif
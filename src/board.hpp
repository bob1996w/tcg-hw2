/*
    board: 

 */
#ifndef _BOARD_HPP_
#define _BOARD_HPP_

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>

#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <chrono>
#include <algorithm>
#include <random>

using namespace std;

#define BOARD_WIDTH 6
#define BOARD_HEIGHT 6
#define BOARD_AREA 36
#define PLAYER_NUM 2
#define CUBE_NUM 6
#define RED 0
#define BLUE 1
#define R_CORNER 0
#define B_CORNER 35
#define NEXT_MOVE_NUM 18
#define MAX_NODE 0
#define MIN_NODE 1

//const double UCB_C = 1.18;      // exploration coefficient of UCB
const double UCB_C = 1.18;
const double timerMaxAllow = 9; // max allow time in seconds
#define MAX_PV_DEPTH 100

const double PP_RD = 1;
const double PP_SIGMA = 2;
const int MIN_PRUNE_NUM_TRIAL = 300;

#define ABS(x) (((x) > 0)? (x) : -(x))

extern fstream flog; // agent.cpp
static mt19937_64 randomEngine(random_device{}());
int getUniformIntRandFixedSize(int);

int maxPVDepth = 0;
int minSteps = 10000;
int totalTrialInOneDetermination = 0;

bool currentOurPlayer = RED;

const int RANDOM_DIR[6][3] = {
    {0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}
};

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

    string printOstream () {
        string ret;
        if (color() == RED) {ret += "+";}
        else {ret += "-";}
        ret += to_string(num());
        return ret;
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

    string printOstream () {
        string ret;
        if (!hasCube()) { ret += "__"; }
        else {ret += c->printOstream(); }
        return ret;
    }
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
const int statusToWinner[4] = {2, 0, 1, 3};

// TODO: is move needed?
struct _game_board {
    using PII = pair<int, int>;
    using VII = vector<PII>;

    Space board[BOARD_AREA];
    bool turn = 0; // 0 = red (upper left), 1 = blue (lower right)
    bool initialTurn = 0;
    int initialCubesLeft[2] = {CUBE_NUM, CUBE_NUM};
    int cubesLeft[2] = {CUBE_NUM, CUBE_NUM};
    Cube initialCubes[PLAYER_NUM][CUBE_NUM];
    Cube cubes[PLAYER_NUM][CUBE_NUM];
    int initialWinner = 2;
    int winner = 2; // 0 = RED, 1 = BLUE, 2 = not decided, 3 = draw
    int steps = 0; // how many steps since game start
    int initialSteps = 0;

    void setBoardAs (_game_board* b) {
        for (int p = 0; p < 2; ++p) {
            for (int i = 0; i < CUBE_NUM; ++i) {
                initialCubes[p][i] = b->initialCubes[p][i];
                cubes[p][i] = b->cubes[p][i];
            }
            initialCubesLeft[p] = b->initialCubesLeft[p];
            cubesLeft[p] = b->cubesLeft[p];
        }
        turn = b->turn;
        initialWinner = b->initialWinner;
        winner = b->winner;
        steps = b->steps;
        initialSteps = b->initialSteps;
        // reverseCopyCube();
        setCurrentAsInitial();
        resetBoard();
    }

    // set the initial board/cube as "after" applying the initial move
    void setBoardAs (_game_board* b, PII initialMove) {
        for (int p = 0; p < 2; ++p) {
            for (int i = 0; i < CUBE_NUM; ++i) {
                initialCubes[p][i] = b->cubes[p][i];
                cubes[p][i] = b->cubes[p][i];
            }
            initialCubesLeft[p] = b->cubesLeft[p];
            cubesLeft[p] = b->cubesLeft[p];
        }
        turn = b->turn;
        initialTurn = b->turn;
        initialWinner = b->initialWinner;
        winner = b->winner;
        steps = b->steps;
        initialSteps = b->initialSteps;
        resetBoard();
        applyMove(initialMove);
        setCurrentAsInitial();
    }

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

    void reverseCopyCube () {
        for (int i = 0; i < CUBE_NUM; ++i) {
            initialCubes[RED][i] = cubes[RED][i];
            initialCubes[BLUE][i] = cubes[BLUE][i];
        }
    }

    void setCurrentAsInitial () {
        initialTurn = turn;
        initialWinner = winner;
        initialCubesLeft[0] = cubesLeft[0];
        initialCubesLeft[1] = cubesLeft[1];
        initialSteps = steps;
        reverseCopyCube();
    }

    void resetBoard () {
        for (int i = 0; i < BOARD_AREA; ++i) {
            board[i] = Space(i);
        }
        for (int i = 0; i < CUBE_NUM; ++i) {
            cubes[RED][i] = initialCubes[RED][i];
            cubes[BLUE][i] = initialCubes[BLUE][i];
            if (cubes[RED][i].isOnBoard()) {
                board[cubes[RED][i].pos] = Space(cubes[RED][i].pos, &cubes[RED][i]);
            }
            if (cubes[BLUE][i].isOnBoard()) {
                board[cubes[BLUE][i].pos] = Space(cubes[BLUE][i].pos, &cubes[BLUE][i]);
            }
        }
        turn = initialTurn;
        winner = initialWinner;
        cubesLeft[0] = initialCubesLeft[0];
        cubesLeft[1] = initialCubesLeft[1];
        steps = initialSteps;
    }

    bool isOut (int y, int x) {
        return (x < 0 || y < 0 || x >= BOARD_WIDTH || y >= BOARD_HEIGHT);
    }

    bool isOccupied (int y, int x) {
        return board[y * BOARD_WIDTH + x].hasCube();
    }

    void nextTurn () { turn = !turn; steps += 1; }

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

        updateWinner(checkWin());
        nextTurn();
    }
    void applyMove (PII move) {
        applyMove(move.first, move.second);
    }

    // vector<num, dir>
    VII getAllMoves () {
        VII res;
        for (int num = 0; num < CUBE_NUM; ++num) {
            PII pos = findCube(turn, num);
            if (pos.first == -100) {
                continue;
            }
            for (int dir = 0; dir < 3; ++dir) {
                int yy = pos.first + dy[turn][dir];
                int xx = pos.second + dx[turn][dir];
                if (isOut(yy, xx)) {
                    continue;
                }
#ifdef LOG
                // flog << "turn " << turn << endl;
                // flog << "num dir = " << num << " " << dir << endl << flush;
#endif
                res.emplace_back(num, dir);
            }
        }
        if (res.empty()) {
            res.emplace_back(15, 15); // ?? for skip
        }
        return res;
    }

    PII getRandomMove () {
        int nodeList[6] = {0, 1, 2, 3, 4, 5};
        int num, dirSeq, dir, yy, xx;
        PII pos;
        // Fisher–Yates shuffle
        for (int i = 0; i < 5; ++i) {
            swap (nodeList[i], nodeList[i + getUniformIntRandFixedSize(6 - i)]);
        }
        for (int i = 0; i < CUBE_NUM; ++i) {
            num = nodeList[i];
            if (cubes[turn][num].isOnBoard()) {
                pos = findCube(turn, num);
                dirSeq = getUniformIntRandFixedSize(3);
                for (int j = 0; j < 3; ++j) {
                    dir = RANDOM_DIR[dirSeq][j];
                    yy = pos.first + dy[turn][dir];
                    xx = pos.second + dx[turn][dir];
                    if (!isOut(yy, xx)) {
                        return make_pair(num, dir);
                    }
                }
            }
        }
        return make_pair(15, 15);
    }

    // Only used for simulation.
    // If we found a better move, return the move immediately.
    // Otherwise, return (-1, -1).
    PII getBetterMove() {
        if (cubesLeft[turn] < 2) { return make_pair(-1, -1); }  
        int yy, xx, poss, yyy, xxx, posss;
        PII pos;
        for (int num = CUBE_NUM - 1; num >= 0; --num) {
            if (cubes[turn][num].isOnBoard()) {
                pos = findCube(turn, num);
                Cube* currentCube = board[pos.first * BOARD_WIDTH + pos.second].c;
                for (int dir = 0; dir < 3; ++dir) {
                    yy = pos.first + dy[turn][dir];
                    xx = pos.second + dx[turn][dir];
                    poss = yy * BOARD_WIDTH + xx;
                    if (isOut(yy, xx)) { continue; }
                    /*
                        O is the player (assume currently red), current turn
                        X is the enemy (assume current blue)
                        
                        rule 1: (dir = 1, ->)
                        
                        .OXX   abc
                        . X     ed
                        If we (a) go right, we can eat (b), 
                        and then probably eaten by (c, d, e).
                        O should be large, and X's should be small.
                    */
                    // Does b exist?
                    Cube* target = board[poss].c;
                    // compare color and size of cube's num
                    if (target != nullptr && target->color() != currentCube->color()) {
                        bool better = true;
                        // Does c,d,e exist?
                        for (int k = 0; k < 3; ++k) {
                            yyy = yy + dy[turn][k];
                            xxx = xx + dx[turn][k];
                            posss = yyy * BOARD_WIDTH + xxx;
                            if (isOut(yyy, xxx)) { continue; }
                            Cube* trap = board[posss].c;
                            if (trap != nullptr && target->num() < currentCube->num() && 
                                    trap->color() != currentCube->color() && trap->num() > currentCube->num()) {
                                better = false;
                                break;
                            }
                        }
                        if (better) {
                            return make_pair(num, dir);
                        }
                    }
                }
            }
        }
        return make_pair(-1, -1);
    }

    // return all of the better moves
    // if none, return empty vector
    VII getBetterMoves () {
        VII returnValues = vector<PII>();
        for (int num = 0; num < CUBE_NUM ; ++num) {
            PII pos = findCube(turn, num);
            if (pos.first == -100) {
                continue;
            }
            for (int dir = 0; dir < 3; ++dir) {
                int yy = pos.first + dy[turn][dir];
                int xx = pos.second + dx[turn][dir];
                if (isOut(yy, xx)) {
                    continue;
                }
                if (isBetterMove(make_pair(num, dir))) {
                    returnValues.emplace_back(num, dir);
                }
            }
        }
        return returnValues;
    }

    bool isBetterMove (PII move) {
        int num = move.first, dir = move.second;
        PII pos = findCube(turn, num);
        int yy = pos.first + dy[turn][dir];
        int xx = pos.second + dx[turn][dir];
        int place = yy * BOARD_WIDTH + xx;

        // 0: enemy reach and larger than us
        if (turn == RED && place == 35) {
            if (board[0].c != nullptr && board[0].c->color() == BLUE && num < board[0].c->num()) { return true; }
        }
        if (turn == BLUE && place == 0) {
            if (board[35].c != nullptr && board[35].c->color() == RED && num < board[35].c->num()) { return true; }
        }

        // 1: be eaten
        int yyy, xxx, posss;
        Cube* currentCube = board[pos.first * BOARD_WIDTH + pos.second].c;
        // if (isOut(yy, xx)) { continue; }
        Cube* target = board[place].c;
        if (target != nullptr && target->color() != currentCube->color()) {
            bool better = true;
            for (int k = 0; k < 3; ++k) {
                yyy = yy + dy[turn][k];
                xxx = xx + dx[turn][k];
                posss = yyy * BOARD_WIDTH + xxx;
                if (isOut(yyy, xxx)) { continue; }
                Cube* trap = board[posss].c;
                if (trap != nullptr && target->num() < currentCube->num() && 
                        trap->color() != currentCube->color() && trap->num() > currentCube->num()) {
                    better = false;
                    break;
                }
            }
            if (better) {
                return true;
            }
        }
        return false;
    }

    VII getFilteredMoves () {
        int nodeList[6] = {0, 1, 2, 3, 4, 5};
        int num, dir, yy, xx;
        // Fisher–Yates shuffle
        for (int i = 0; i < 5; ++i) {
            swap (nodeList[i], nodeList[i + getUniformIntRandFixedSize(6 - i)]);
        }
        int dirList[3] = {0, 1, 2};
        for (int i = 0; i < 2; ++i) {
            swap (dirList[i], dirList[i + getUniformIntRandFixedSize(3 - i)]);
        }
        VII betterMoves;
        VII res;
        VII worseMoves;
        for (int index = 0; index < CUBE_NUM; ++index) {
            num = nodeList[index];
            PII pos = findCube(turn, num);
            if (pos.first == -100) {
                continue;
            }
            for (int dirIndex = 0; dirIndex < 3; ++dirIndex) {
                dir = dirList[dirIndex];
                yy = pos.first + dy[turn][dir];
                xx = pos.second + dx[turn][dir];
                if (isOut(yy, xx)) {
                    continue;
                }
#ifdef LOG
                // flog << "turn " << turn << endl;
                // flog << "num dir = " << num << " " << dir << endl << flush;
#endif
                if (isBetterMove(make_pair(num, dir))) {
                    betterMoves.emplace_back(num, dir);
                }
                else if (isWorseMove(make_pair(num, dir))) {
                    // we still need to save the worse moves because we might
                    // end up no moves other than worseMoves.
                    worseMoves.emplace_back(num, dir);
                }
                else {
                    res.emplace_back(num, dir);
                }
            }
        }
        if (!betterMoves.empty()) {
            return betterMoves;
        }
        else if (!res.empty()) {
            return res;
        }
        else if (!worseMoves.empty()) {
            return worseMoves;
        }
        else {
            res.emplace_back(15, 15);
            return res;
        }
    }

    /*
    VII getWorseMoves () {
        VII returnValues = vector<PII>();
        int yy, xx;
        PII pos;
        if (turn == RED) {
            if (!board[0].hasCube() || board[0].c->color() == turn) {
                for (int num = CUBE_NUM - 1; num >= 0; --num) {
                    pos = findCube(turn, num);
                    Cube* currentCube = board[pos.first * BOARD_WIDTH + pos.second].c;
                    for (int dir = 0; dir < 3; ++dir) {
                        yy = pos.first + dy[turn][dir];
                        xx = pos.second + dx[turn][dir];
                        if (yy == 5 && xx == 5) {
                            returnValues.emplace_back(num, dir);
                        }
                    }
                }
            }
        }
        else {
            if (!board[35].hasCube() || board[35].c->color() == turn) {
                for (int num = CUBE_NUM - 1; num >= 0; --num) {
                    pos = findCube(turn, num);
                    Cube* currentCube = board[pos.first * BOARD_WIDTH + pos.second].c;
                    for (int dir = 0; dir < 3; ++dir) {
                        yy = pos.first + dy[turn][dir];
                        xx = pos.second + dx[turn][dir];
                        if (yy == 0 && xx == 0) {
                            returnValues.emplace_back(num, dir);
                        }
                    }
                }
            }
        }
        
        return returnValues;
    }
    */

    bool isWorseMove (PII move) {
        PII pos = findCube(turn, move.first);
        int yy = pos.first + dy[turn][move.second];
        int xx = pos.second + dx[turn][move.second];
        int place = yy * BOARD_WIDTH + xx;
        return (turn == RED && (board[0].c == nullptr || board[0].c->color() == turn) && place == 35) || 
                (turn == BLUE && (board[35].c == nullptr || board[35].c->color() == turn) && place == 0);
    }

    string sendMove(PII move) {
        string send;
        send += (char)(move.first + '0');
        send += (char)(move.second + '0');
        return send;
    }

    // 0: not over, 1: RED win, 2: BLUE win, 3: draw
    int checkWin () {
        if (cubesLeft[BLUE] == 0) { return 1; }
        if (cubesLeft[RED] == 0) { return 2; }
        if (board[R_CORNER].hasCube() && board[R_CORNER].c->color() == BLUE && 
                board[B_CORNER].hasCube() && board[B_CORNER].c->color() == RED) {
            if (board[R_CORNER].c->num() < board[B_CORNER].c->num()) {
                return 2;
            }
            else if (board[R_CORNER].c->num() > board[B_CORNER].c->num()) {
                return 1;
            }
            else {
                return 3;
            }
        }
        return 0;
    }

    // status from checkWin
    void updateWinner (int status) {
        winner = statusToWinner[status];
    }

    string printOstream() {
        string ret;
        for (int p = 0; p < PLAYER_NUM; ++p) {
            for (int j = 0; j < CUBE_NUM; ++j) {
                ret += string(cubes[p][j].printDetail()) + "\n";
            }
        }
        ret += ("CubeLeft: +RED " + to_string(cubesLeft[RED]) + ", -BLUE " + to_string(cubesLeft[BLUE]) + "\n");
        for (int i = 0; i < BOARD_AREA; ++i) {
            ret += (board[i].printOstream() + " \n"[i % BOARD_WIDTH == (BOARD_WIDTH - 1)]);
        }
        ret += ("NextTurn: " + string((turn == RED) ? "RED" : "BLUE") + "\n");
        return ret;
    }

    // TODO: what should be added to board?
};
using GameBoard = _game_board;

ostream& operator<< (ostream &os, GameBoard& b) {
    for (int p = 0; p < PLAYER_NUM; ++p) {
        for (int j = 0; j < CUBE_NUM; ++j) {
            os << b.cubes[p][j].printDetail() << endl << flush;
        }
    }
    os << "CubeLeft: +RED " << b.cubesLeft[RED] << ", -BLUE " << b.cubesLeft[BLUE] << endl << flush;
    for (int i = 0; i < BOARD_AREA; ++i) {
        os << b.board[i] << " \n"[i % BOARD_WIDTH == (BOARD_WIDTH - 1)];
    }
    os << "NextTurn: " << ((b.turn == RED) ? "RED" : "BLUE") << endl << flush;
    return os;
}

// generate number: integer in [min, max)
int getUniformIntRand(int min, int max) {
    uniform_int_distribution<int> d(min, max - 1);
    return d(randomEngine);
}
// generate number: integer in [0, max)
int getUniformIntRand(int max) {
    return getUniformIntRand(0, max);
}

// generate number: integer in [0, max), max = 1 ~ 6
int getUniformIntRandFixedSize (int max) {
    static uniform_int_distribution<int> dist[6] = {
        uniform_int_distribution<int>(0, 0),
        uniform_int_distribution<int>(0, 1),
        uniform_int_distribution<int>(0, 2),
        uniform_int_distribution<int>(0, 3),
        uniform_int_distribution<int>(0, 4),
        uniform_int_distribution<int>(0, 5)
    };
    return dist[max - 1](randomEngine);
}

void swap (int& a, int& b) {
    int t = a;
    a = b;
    b = t;
}

// Monte-Carlo tree node
struct TreeNode {
    using PII = pair<int, int>;
    using VII = vector<PII>;

    TreeNode* child[NEXT_MOVE_NUM];
    TreeNode* parent = nullptr;
    int childCount = 0;
    int scoreWin = 0;           // in random trials, how many leads to win
    int scoreDraw = 0;          // in random trials, how many leads to draw
    int scoreLose = 0;          // in random trials, how many leads to lose
    int trial = 0;              // total trials tried for this node
    double winRate = 0.0;       // = scoreWin / trial (parent = MAX_NODE) or scoreLose / trial (parent = MIN_NODE)
    double sqrtLogN = 0.0;      // sqrt(log(trial))
    bool nodeType = MAX_NODE;   // is this a MAX_NODE (0, player) or MIN_NODE (1, enemy)?
    int currentBestChild = -1;  // -1: not decided
    PII move;                   // the move from its parent.

    // statistics: for each simulation,
    // old: 1 = win, 0 = draw, -1 = lose
    // new: 1/sqrt(board.steps) * ((winner == parent.turn)? 1: -1);
    double sum1 = 0;               // = \Sigma x_i
    double sum2 = 0;               // = \Sigma X_i^2
    int pruned = false;         // is this node pruned in progressive pruning?
    double mean = 0;
    double stdev = 0;

    GameBoard board;

    TreeNode () = delete;

    TreeNode (GameBoard* currentBoard, TreeNode* parentPtr = nullptr, bool type = MAX_NODE) {
        board = GameBoard();
        board.setBoardAs(currentBoard);
        parent = parentPtr;
        nodeType = type;
    }

    TreeNode (GameBoard* currentBoard, PII initialMove, TreeNode* parentPtr, bool type = MAX_NODE) {
        board = GameBoard();
        board.setBoardAs(currentBoard, initialMove);
        // flog << board << endl << flush;
        parent = parentPtr;
        nodeType = type;
        move = initialMove;
    }

    // slow code
    double slowUCBScore (int totalTrial) {
        return winRate + sqrt(log(totalTrial) / trial);
    }

    double UCBScore (double sqrtLogNTotal) {
        return winRate + UCB_C * sqrtLogNTotal / sqrtLogN;
    }

    double UCBExploreTerm (double sqrtLogNTotal) {
        return UCB_C * sqrtLogNTotal / sqrtLogN;
    }

    // isStatisticallyInferiorTo = this + node1.stdev < PP_SIGMA && node2.stdev < PP_SIGMA
    bool isStatisticallyInferiorTo (TreeNode *that) {
#ifdef LOG
        // flog << "inferior " << nodeType << " " << rightExpectedOutcome() << " " << that->nodeType << " " << that->leftExpectedOutcome() << endl;
#endif
        return rightExpectedOutcome() < that->leftExpectedOutcome();
    }

    double rightExpectedOutcome () {
        if (nodeType == MAX_NODE) {
            return mean + PP_RD * stdev;
        }
        else {
            return -mean + PP_RD * stdev;
        }
    }

    double leftExpectedOutcome () {
        if (nodeType == MAX_NODE) {
            return mean - PP_RD * stdev;
        }
        else {
            return -mean - PP_RD * stdev;
        }
    }

    // winRate now contains same winRate as parents
    int findBestWinRate () {
        if (childCount == 1) { return 0; }
        int winnerStep = -1;
        double winnerRate = -10000 ;
        //double leastWinnerRate = 10000;
        for (int c = 0; c < childCount; ++c) {
            // calculate by win rate
            if (!(child[c]->pruned) && (double)(child[c]->scoreWin) / child[c]->trial > winnerRate) {
                winnerStep = c;
                winnerRate = (double)(child[c]->scoreWin) / child[c]->trial;
            }

            // calculate by enemy's least winRate
            /*
            if ((!child[c]->pruned) && child[c]->winRate < leastWinnerRate) {
                winnerStep = c;
                leastWinnerRate = child[c]->winRate;
            }
            */
        }
        return winnerStep;
    }

    void updateBestChild () {
        currentBestChild = findBestChildByUCB();
    }

    // called by child: parent->updateScoreFromChild
    void updateScoreFromChild (int winAdded, int drawAdded, int loseAdded, int trialAdded, 
            bool skipPruning = false, int pruneScoreUpdate = 0, double sum1Added = 0, double sum2Added = 0) {
        scoreWin += winAdded;
        scoreDraw += drawAdded;
        scoreLose += loseAdded;
        trial += trialAdded;
        if (nodeType != MAX_NODE) { // parent node has different node type than this
            winRate = (double)scoreWin / trial;
        }
        else {
            winRate = (double)scoreLose / trial;
        }
        if (pruneScoreUpdate == 0) {
            sum1 = scoreWin - scoreLose;
            sum2 = scoreWin + scoreLose;
        }
        else if (pruneScoreUpdate == 1) {
            sum1 += sum1Added;
            sum2 += sum2Added;
            // cerr << "updateScoreFromChild" << sum1Added << " " << sum2Added << endl << flush;
        }
        mean = (double) sum1 / trial;
        stdev = sqrt((double) (sum2 - 2 * mean * sum1) / trial + mean * mean);
        sqrtLogN = sqrt(log(trial));
        // updateBestChild();
        if (!skipPruning) {
            progressivePruningParentUpdate();
        }
        if (parent != nullptr) {
            //parent->updateScoreFromChild(scoreAdded, trialAdded);
            parent->updateScoreFromChild(winAdded, drawAdded, loseAdded, trialAdded, true,
                pruneScoreUpdate, sum1Added, sum2Added);
        }
    }

    // find winRate to us in MAX_NODE, find winRate to enemy in MIN_NODE
    // if no child, return -1
    int findBestChildByUCB () {
        return findMaxChildByUCB();
    }

    int findMaxChildByUCB () {
        // reverse winRate in child
        // because the winRate for child is the opposite for current parent.
        if (childCount == 0) { return -1; }
        else if (childCount == 1) { return 0; }
        else {
            int bestChild = -1;
            double bestScore = -10000, s;
            for (int i = 0; i < childCount; ++i) {
                if (!(child[i]->pruned) && (s = child[i]->winRate + child[i]->UCBExploreTerm(sqrtLogN)) > bestScore) {
                    bestChild = i;
                    bestScore = s;
                }
            }
            // cerr << "bestChild " << nodeType << " " << bestChild << " " << bestScore << " : " << child[bestChild]->winRate << " " << child[bestChild]->UCBExploreTerm(sqrtLogN) << endl;
            return bestChild;
        }
    }

    // update the progressive pruning as parent
    void progressivePruningParentUpdate () {
        if (childCount == 1) { return; }
        // TODO: how to update non-leaf pruning?
        if (nodeType == currentOurPlayer) {
            double largestLeftExpectedOutcome = -10000, temp;
            for (int c = 0; c < childCount; ++c) {
                if ((child[c]->trial >= MIN_PRUNE_NUM_TRIAL) && !(child[c]->pruned) &&
                        child[c]->stdev < PP_SIGMA &&
                        (temp = (child[c]->leftExpectedOutcome())) > largestLeftExpectedOutcome) {
                    largestLeftExpectedOutcome = temp;
                }
            }
            // prune child that is isStatisticallyInferiorTo largestLeftExpectedOutcome
            for (int c = 0; c < childCount; ++c) {
                if ((child[c]->trial >= MIN_PRUNE_NUM_TRIAL) && !(child[c]->pruned) &&
                        child[c]->stdev < PP_SIGMA &&
                        (temp = (child[c]->rightExpectedOutcome())) < largestLeftExpectedOutcome) {
                    child[c]->pruned = true;
                }
            }
        }
        else {
            // if MIN_NODE, we want to prune the child with the highest score.
            double smallestRightExpectedOutcome = 10000, temp;
            for (int c = 0; c < childCount; ++c) {
                if ((child[c]->trial >= MIN_PRUNE_NUM_TRIAL) && !(child[c]->pruned) && 
                        child[c]->stdev < PP_SIGMA &&
                        (temp = (child[c]->rightExpectedOutcome())) < smallestRightExpectedOutcome) {
                    smallestRightExpectedOutcome = temp;
                }
            }
            for (int c = 0; c < childCount; ++c) {
                if ((child[c]->trial >= MIN_PRUNE_NUM_TRIAL) && !(child[c]->pruned) &&
                        child[c]->stdev < PP_SIGMA &&
                        (temp = (child[c]->leftExpectedOutcome())) > smallestRightExpectedOutcome ) {
                    child[c]->pruned = true;
                }
            }

        }
    }

    // pruneScoreUpdate: the method of updating the prune score
    // 0: normal (+1 win, 0 draw, -1 lose)
    // 1: 1/sqrt(board.steps) * (+1 win, 0 draw, -1 lose) when winner is decided
    void runRandomTrial (int numTrial, int pruneScoreUpdate = 0) {
        pair<int, int> move;
        VII moves;
        double tempSum1 = 0, tempSum2 = 0, tempX;
        for (int t = 0; t < numTrial; ++t) {
            int turns = 0;
            while (board.winner == 2) {
#ifdef RULE
    #ifdef LOG
                //cerr << "running betterMove" << endl << flush;
    #endif
                // move = board.getBetterMove();
    #ifdef LOG
                //cerr << "end betterMove" << endl << flush;
                //cerr << move.first << " " << move.second << endl << flush;
    #endif
                //if (move.first == -1) {
                    //moves = board.getFilteredWorseMoves();
                    //move = moves.at(0);
                    //move = board.getRandomMove();
                //}
                move = board.getFilteredMoves()[0];
#else
                move = board.getRandomMove();
#endif
                board.applyMove(move);
#ifdef LOG
                // flog << t << " " << board.winner << endl;
#endif
                turns += 1;
                if (turns >= 200) {
#ifdef LOG
                    flog << "turn > 200 error, dump board\n" << board << endl << flush;
#endif
                    break;
                }
            }
            if (board.winner == currentOurPlayer) {
                scoreWin += 1;
                if (pruneScoreUpdate == 1) {
                    tempX = 1.0 / sqrt(board.steps);
                    tempSum1 += tempX;
                    tempSum2 += tempX * tempX;
                }
            }
            else if (board.winner == 3) {
                scoreDraw += 1;
            }
            else {
                scoreLose += 1;
                if (pruneScoreUpdate == 1) {
                    tempX = -1.0 / sqrt(board.steps);
                    tempSum1 += tempX;
                    tempSum2 += tempX * tempX;
                }
            }
#ifdef LOG
            if (board.steps < minSteps) {
                minSteps = board.steps;
                //cerr << "minSteps " << minSteps << endl << flush;
            }
#endif
            board.resetBoard();
            totalTrialInOneDetermination += 1;
        }
        trial += numTrial;
        if (pruneScoreUpdate == 0) {
            sum1 = scoreWin - scoreLose;
            sum2 = scoreWin + scoreLose;
        }
        else if (pruneScoreUpdate == 1) {
            sum1 = tempSum1;
            sum2 = tempSum2;
        }
        mean = (double) sum1 / trial;
        stdev = sqrt((double) (sum2 - 2 * mean * sum1) / trial + mean * mean);
        if (parent->nodeType == MAX_NODE) {
            winRate = (double)scoreWin / trial;
        }
        else {
            winRate = (double)scoreLose / trial;
        }
        sqrtLogN = sqrt(log(trial));
    }

    // return the best children
    void runRandomTrialForAllChildren (int numTrial) {
        int winAdded = 0, drawAdded = 0, loseAdded = 0, trialAdded = 0;
        int batchNumTrial = numTrial / 10, currentChildTrial = 0;
        for (int c = 0; c < childCount; ++c) {
#ifdef LOG
            //flog << "running child " << c << endl << flush;
#endif
            currentChildTrial = 0;
            while (currentChildTrial < numTrial) {
                child[c]->runRandomTrial(batchNumTrial);
                currentChildTrial += batchNumTrial;
                if (currentChildTrial >= MIN_PRUNE_NUM_TRIAL) {
                    // try pruning child
                    for (int c2 = 0; c2 < c; ++c2) {
                        if (child[c]->stdev < PP_SIGMA && child[c2]->stdev < PP_SIGMA &&
                                child[c]->isStatisticallyInferiorTo(child[c2])) {
                            child[c]->pruned = true;
                            break;
                        }
                    }
                }
                if (child[c]->pruned) { break; }
            }
            trialAdded += currentChildTrial;
            winAdded += child[c]->scoreWin;
            drawAdded += child[c]->scoreDraw;
            loseAdded += child[c]->scoreLose;
        }
        updateScoreFromChild(winAdded, drawAdded, loseAdded, trialAdded, true);
    }

    // run uneven trials for each children
    void runUnevenTrialsForAllChildren(int leastTrial, int pruneScoreUpdate) {
        VII betterMoves = board.getBetterMoves();
        int winAdded = 0, drawAdded = 0, loseAdded = 0, trialAdded = 0;
        double sum1Added = 0, sum2Added = 0;
        for (int c = 0; c < childCount; ++c) {
            int currentChildTrial = leastTrial;
            // is this move a betterMove?
            VII::iterator findResult = find(betterMoves.begin(), betterMoves.end(), child[c]->move);
            if (findResult != betterMoves.end()) {
#ifdef LOG
                //cerr << "isBetterMove" << endl;
#endif
                currentChildTrial *= 2;
            }
            child[c]->runRandomTrial(currentChildTrial, 1);
            trialAdded += currentChildTrial;
            winAdded += child[c]->scoreWin;
            drawAdded += child[c]->scoreDraw;
            loseAdded += child[c]->scoreLose;
            if (pruneScoreUpdate == 1) {
                // cerr << "runUnEven" << child[c]->sum1 << " " << child[c]->sum2 << endl;
                sum1Added += child[c] -> sum1;
                sum2Added += child[c] -> sum2;
            }
        }
        // cerr << "runUneven " << sum1Added << " " << sum2Added << endl;
#ifdef PRUNE
        updateScoreFromChild(winAdded, drawAdded, loseAdded, trialAdded, false, pruneScoreUpdate, sum1Added, sum2Added);
#else
        updateScoreFromChild(winAdded, drawAdded, loseAdded, trialAdded, true, pruneScoreUpdate, sum1Added, sum2Added);
#endif
    }
    
    // run uneven trials for all children, specifiying a total trials
    // where sum(all children trials) <= totalTrial
    // each of the betterMoves move gets twice of trial.
    void runUnevenTrialsForAllChildrenTotal(int totalTrial, int pruneScoreUpdate) {
        VII betterMoves = board.getBetterMoves();
        int winAdded = 0, drawAdded = 0, loseAdded = 0, trialAdded = 0;
        double sum1Added = 0, sum2Added = 0;
        int leastTrial = totalTrial / (childCount + betterMoves.size());
        for (int c = 0; c < childCount; ++c) {
            int currentChildTrial = leastTrial;
            // is this move a betterMove?
            VII::iterator findResult = find(betterMoves.begin(), betterMoves.end(), child[c]->move);
            if (findResult != betterMoves.end()) {
#ifdef LOG
                //cerr << "isBetterMove" << endl;
#endif
                currentChildTrial = (int)((double) currentChildTrial * 2);
            }
            child[c]->runRandomTrial(currentChildTrial, 1);
            trialAdded += currentChildTrial;
            winAdded += child[c]->scoreWin;
            drawAdded += child[c]->scoreDraw;
            loseAdded += child[c]->scoreLose;
            if (pruneScoreUpdate == 1) {
                // cerr << "runUnEven" << child[c]->sum1 << " " << child[c]->sum2 << endl;
                sum1Added += child[c] -> sum1;
                sum2Added += child[c] -> sum2;
            }
        }
        // cerr << "runUneven " << sum1Added << " " << sum2Added << endl;
#ifdef PRUNE
        updateScoreFromChild(winAdded, drawAdded, loseAdded, trialAdded, false, pruneScoreUpdate, sum1Added, sum2Added);
#else
        updateScoreFromChild(winAdded, drawAdded, loseAdded, trialAdded, true, pruneScoreUpdate, sum1Added, sum2Added);
#endif
    }

    PII getRandomTrialScoreMove () {
        // bool ourPlayer = board.turn;
        currentOurPlayer = board.turn;
        VII possibleMoves = board.getAllMoves();
        childCount = possibleMoves.size();
#ifdef LOG
        flog << "getAllMovesFromThisBoard\n" << board << endl << flush;
        flog << "childCount = " << childCount << endl << flush;
#endif
        if (childCount == 1) {
            return possibleMoves[0];
        }
        for (int c = 0; c < childCount; ++c) {
            child[c] = new TreeNode (&board, possibleMoves[c] ,this, MIN_NODE);
        }
        runRandomTrialForAllChildren (3000);
        int bestMove = findBestChildByUCB();
#ifdef LOG
        flog << "Best move: Child " << bestMove << ", best score: " << child[bestMove]->UCBScore(sqrtLogN) << endl << flush;
#endif
        return possibleMoves[bestMove];
    }

    // return false if should stop searching, true otherwise
    bool pvSearchWithUCB (int depth, bool isEvenlyRandom = true, int pruneScoreUpdate = 0, int trialCountType = 0) {
        if (depth > maxPVDepth) {
            maxPVDepth = depth;

            TreeNode* n = parent;
            while (n->parent != nullptr) { n = n->parent; }
            int nodeCount = n->numNode();
            // cerr << "Max PV Depth " << maxPVDepth << ", node " << nodeCount << endl;
            //flog << "Max PV Depth " << maxPVDepth << ", node " << nodeCount << endl;

        }


        if (depth >= MAX_PV_DEPTH) { return false; }
        /*
        if (currentBestChild != -1) {
            return child[currentBestChild]->pvSearchWithUCB(ourPlayer, maxDepth - 1);
        }
        */
        if (childCount >= 1) {
            currentBestChild = findBestChildByUCB();
            return child[currentBestChild]->pvSearchWithUCB(depth + 1, isEvenlyRandom, pruneScoreUpdate, trialCountType);
        }
        if (board.winner != 2) {
            return false;
        } // winner decided / draw
        
        VII possibleMoves = board.getAllMoves();
        childCount = possibleMoves.size();
        if (childCount == 1) {
            child[0] = new TreeNode(&board, possibleMoves[0], this, !nodeType);
            currentBestChild = 0;
            return true;
        }
        for (int c = 0; c < childCount; ++c) {
            child[c] = new TreeNode(&board, possibleMoves[c], this, !nodeType);
        }
        if (trialCountType == 0) {
            if (isEvenlyRandom) {
                runRandomTrialForAllChildren (500);
            }
            else {
                runUnevenTrialsForAllChildren(300, pruneScoreUpdate);
            }
        }
        else if (trialCountType == 1) {
            runUnevenTrialsForAllChildrenTotal(4000, pruneScoreUpdate);
        }
        
        // updateBestChild();
        return true;
    }

    PII getMonteCarloBasicMove (bool isEvenlyRandom = true, int pruneScoreUpdate = 0, int trialCountType = 0) {
        double timerElapsed;
        decltype(chrono::steady_clock::now()) timerStart = chrono::steady_clock::now();
        maxPVDepth = 0;
        minSteps = 10000;
        totalTrialInOneDetermination = 0;
        currentOurPlayer = board.turn;
#ifdef LOG 
            flog << "isEvenlyRandom " << isEvenlyRandom << endl;
            flog << "pruneScoreUpdate " << pruneScoreUpdate << endl;
            flog << "trialCountType " << trialCountType << endl;
#endif
        while ((timerElapsed = chrono::duration_cast<chrono::duration<double>>(chrono::steady_clock::now() - timerStart).count()) 
                < timerMaxAllow) {
            if (!pvSearchWithUCB (0, isEvenlyRandom, pruneScoreUpdate, trialCountType)) { break; }
            if (childCount == 1) { break; }
            // updateBestChild();
        }
        int bestMove = findBestWinRate();
#ifdef LOG
        flog << "board.turn = " << string((board.turn == RED)? "RED" : "BLUE") << endl << flush; 
        // flog << printNode(0) << endl << flush;
        flog << "totalTrialInOneDetermination = " << totalTrialInOneDetermination << endl << flush;
#endif
        return child[bestMove]->move;
    }
    
    string printNode (int depth) {
        string ret;
        for (int i = 0; i < depth; ++i) {
            ret += "  ";
        }
        ret += "<" + to_string(move.first) + "," + to_string(move.second) + ">";
        ret += string((nodeType == MAX_NODE)? "+":"-") + string(pruned? "p":" ") + " ";
        ret += "w" + to_string(scoreWin) + " d" + to_string(scoreDraw) + " l" + to_string(scoreLose) + "/" + to_string(trial);
        ret += " " + to_string(winRate);
        ret += " best=" + to_string(currentBestChild);
        if (depth != 0) {
            ret += " ue" + to_string(UCBExploreTerm(parent->sqrtLogN));
        }
        ret += " mean" + to_string(mean) + " stdev" + to_string(stdev);
        ret += "\n";
        for (int c = 0; c < childCount; ++c) {
            ret += child[c]->printNode(depth + 1);
        }
        return ret;
    }

    int numNode () {
        int currentNodeCount = 0;
        for (int c = 0; c < childCount; ++c) {
            currentNodeCount += child[c] -> numNode();
        }
        return currentNodeCount + 1;
    }
};

#endif
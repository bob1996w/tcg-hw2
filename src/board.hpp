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

const double UCB_C = 1.18;      // exploration coefficient of UCB
const double timerMaxAllow = 2; // max allow time in seconds

#define ABS(x) ((x > 0)? x : -x)

extern fstream flog; // agent.cpp
static mt19937_64 randomEngine(random_device{}());
int getUniformIntRandFixedSize(int); 

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
    int score = 0;              // in random trials, how many leads to win
    int trial = 0;              // total trials tried for this node
    double winRate = 0.0;       // = score / trial
    double sqrtLogN = 0.0;      // sqrt(log(trial))
    bool nodeType = MAX_NODE;   // is this a MAX_NODE (0, player) or MIN_NODE (1, enemy)?
    int currentBestChild = -1;  // -1: not decided
    PII move;                   // the move from its parent.

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

    int findBestWinRate () {
        if (childCount == 1) { return 0; }
        int winnerStep = 0;
        double winnerRate = child[0]->winRate;
        for (int c = 1; c < childCount; ++c) {
            if (child[c]->winRate > winnerRate) {
                winnerStep = c;
                winnerRate = child[c]->winRate;
            }
        }
        return winnerStep;
    }

    void updateBestChild () {
        currentBestChild = findBestChildByUCB();
    }

    // called by child: parent->updateScoreFromChild
    void updateScoreFromChild (int scoreAdded, int trialAdded) {
        score += scoreAdded;
        trial += trialAdded;
        winRate = (double)score / trial;
        sqrtLogN = sqrt(log(trial));
        if (parent != nullptr) {
            parent->updateScoreFromChild(scoreAdded, trialAdded);
            parent->updateBestChild();
        }
    }

    // find maximum score in MAX_NODE, find minimum score in MIN_NODE
    // if no child, return -1
    int findBestChildByUCB () {
        if (nodeType == MAX_NODE) {
            return findMaxChildByUCB();
        }
        else {
            return findMinChildByUCB();
        }
    }

    int findMaxChildByUCB () {
        if (childCount == 0) { return -1; }
        else if (childCount == 1) { return 0; }
        else {
            int bestChild = 0;
            double bestScore = child[0]->UCBScore(sqrtLogN), s;
            for (int i = 0; i < childCount; ++i) {
                if ((s = child[i]->UCBScore(sqrtLogN)) > bestScore) {
                    bestChild = i;
                    bestScore = s;
                }
            }
            return bestChild;
        }
    }

    int findMinChildByUCB () {
        if (childCount == 0) { return -1; }
        else if (childCount == 1) { return 0; }
        else {
            int bestChild = 0;
            double bestScore = child[0]->UCBScore(sqrtLogN), s;
            for (int i = 0; i < childCount; ++i) {
                if ((s = child[i]->UCBScore(sqrtLogN)) < bestScore) {
                    bestChild = i;
                    bestScore = s;
                }
            }
            return bestChild;
        }
    }

    void runRandomTrial (int numTrial, bool ourPlayer) {
        for (int t = 0; t < numTrial; ++t) {
            int turns = 0;
            while (board.winner == 2) {
                pair<int, int> move = board.getRandomMove();
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
            if (board.winner == ourPlayer) {
                score += 1;
            }
            board.resetBoard();
        }
        trial += numTrial;
        winRate = (double)score / trial;
        sqrtLogN = sqrt(log(trial));
    }

    // return the best children
    void runRandomTrialForAllChildren (int numTrial, bool ourPlayer) {
        int scoreAdded = 0, trialAdded = 0;
        for (int c = 0; c < childCount; ++c) {
#ifdef LOG
            //flog << "running child " << c << endl << flush;
#endif
            child[c]->runRandomTrial(numTrial, ourPlayer);
            trial += numTrial;
            trialAdded += numTrial;
            score += child[c]->score;
            scoreAdded += child[c]->score;
        }
        // TODO: update winRate
        winRate = (double)score / trial;
        // TODO: update sqrtLogN
        sqrtLogN = sqrt(log(trial));
        if (parent != nullptr) {
            parent->updateScoreFromChild(scoreAdded, trialAdded);
        }
    }

    PII getRandomTrialScoreMove () {
        bool ourPlayer = board.turn;
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
        runRandomTrialForAllChildren (3000, ourPlayer);
        int bestMove = findBestChildByUCB();
#ifdef LOG
        flog << "Best move: Child " << bestMove << ", best score: " << child[bestMove]->UCBScore(sqrtLogN) << endl << flush;
#endif
        return possibleMoves[bestMove];
    }

    // return false if hit max depth, true otherwise
    bool pvSearchWithUCB (bool ourPlayer, int maxDepth) {
        if (maxDepth <= 0) { return false; }
        if (currentBestChild != -1) {
            child[currentBestChild]->pvSearchWithUCB(ourPlayer, maxDepth - 1);
            return true;
        }
        VII possibleMoves = board.getAllMoves();
        childCount = possibleMoves.size();
        if (childCount == 1) {
            currentBestChild = 0;
            return true;
        }
        for (int c = 0; c < childCount; ++c) {
            child[c] = new TreeNode(&board, possibleMoves[c], this, !nodeType);
        }
        runRandomTrialForAllChildren (300, ourPlayer);
        updateBestChild();
        return true;
    }

    PII getMonteCarloBasicMove () {
        double timerElapsed;
        decltype(chrono::steady_clock::now()) timerStart = chrono::steady_clock::now();
        while ((timerElapsed = chrono::duration_cast<chrono::duration<double>>(chrono::steady_clock::now() - timerStart).count()) 
                < timerMaxAllow) {
#ifdef LOG
            flog << "pvSearch" << endl << flush;
#endif
            if (!pvSearchWithUCB (board.turn, 100)) { break; }
            if (childCount == 1) { break; }
            updateBestChild();
        }
        int bestMove = findBestWinRate();
#ifdef LOG
        flog << printNode(0) << endl << flush;
#endif
        return child[bestMove]->move;
    }
    
    string printNode (int depth) {
        string ret;
        for (int i = 0; i < depth; ++i) {
            ret += "  ";
        }
        ret += "<" + to_string(move.first) + "," + to_string(move.second) + ">";
        ret += string((nodeType == MAX_NODE)? "+":"-") + " ";
        ret += to_string(score) + "/" + to_string(trial);
        ret += " " + to_string(winRate);
        ret += " best=" + to_string(currentBestChild);
        if (depth != 0) {
            ret += " ue" + to_string(UCBExploreTerm(parent->sqrtLogN));
        }
        ret += "\n";
        for (int c = 0; c < childCount; ++c) {
            ret += child[c]->printNode(depth + 1);
        }
        return ret;
    }

};

#endif
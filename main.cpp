/*
 * Main function for benzene-wrapper
 * Written by Luke Schultz
 * June 2023
 */

#include "wrapper.h"
#include <vector>
#include <cassert>

using namespace std;

void runTests() {
    uint8_t* board = new uint8_t[36];
    vector <bool> mustplay;
    vector <bool> expectedMustplay;

    // Test Case 1
    memset(board, BLANK, sizeof (uint8_t) * 36);
    mustplay = getMustplay(6, board, true);
    expectedMustplay = {0,0,0,0,0,0,
                        0,0,1,1,1,1,
                        0,1,1,1,1,1,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0};
    assert(mustplay == expectedMustplay);
    assert(isTerminal(6, board, true) == false);

    // Test Case 2
    memset(board, BLANK, sizeof (uint8_t) * 36);
    mustplay = getMustplay(6, board, false);
    expectedMustplay = {0,0,0,1,1,0,
                        0,0,1,1,1,0,
                        0,1,1,1,1,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0};
    assert(mustplay == expectedMustplay);
    assert(isTerminal(6, board, false) == false);

    // Test Case 3
    memset(board, BLANK, sizeof (uint8_t) * 36);
    board[4] = BLACK;
    board[15] = BLACK;
    board[26] = BLACK;
    mustplay = getMustplay(6, board, true);
    expectedMustplay = {0,0,0,0,0,0,
                        0,0,1,1,0,1,
                        0,1,1,0,1,1,
                        1,1,1,0,1,0,
                        1,0,0,0,0,0,
                        0,0,0,0,0,0};
    assert(mustplay == expectedMustplay);
    assert(isTerminal(6, board, true) == true);

    // Test Case 4
    memset(board, BLANK, sizeof (uint8_t) * 36);
    board[4] = WHITE;
    board[5] = WHITE;
    board[15] = BLACK;
    board[26] = BLACK;
    mustplay = getMustplay(6, board, true);
    expectedMustplay = {0,0,0,0,0,0,
                        0,0,1,1,0,0,
                        0,1,1,0,0,0,
                        1,1,0,0,0,0,
                        1,0,0,0,0,0,
                        0,0,0,0,0,0};
    assert(mustplay == expectedMustplay);
    assert(isTerminal(6, board, true) == false);

    // Test Case 5
    memset(board, BLANK, sizeof (uint8_t) * 36);
    board[5] = WHITE;
    board[14] = BLACK;
    board[16] = WHITE;
    board[27] = WHITE;
    mustplay = getMustplay(6, board, false);
    expectedMustplay = {0,0,0,1,0,0,
                        0,1,1,1,0,0,
                        0,0,0,0,0,0,
                        0,0,1,0,0,0,
                        0,1,1,0,0,0,
                        0,1,0,0,0,0};
    assert(mustplay == expectedMustplay);
    assert(isTerminal(6, board, false) == false);

    // Test Case 6
    memset(board, BLANK, sizeof (uint8_t) * 36);
    board[24] = WHITE;
    board[25] = WHITE;
    board[26] = WHITE;
    board[27] = WHITE;
    board[30] = BLACK;
    board[31] = BLACK;
    board[32] = BLACK;
    board[33] = BLACK;
    board[34] = BLACK;
    board[35] = BLACK;
    mustplay = getMustplay(6, board, false);
    expectedMustplay = {0,0,1,1,1,0,
                        0,1,1,1,1,0,
                        0,1,1,1,1,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0};
    assert(mustplay == expectedMustplay);
    assert(isTerminal(6, board, false) == true);

    // Test Case 7
    memset(board, BLANK, sizeof (uint8_t) * 36);
    board[0] = WHITE;
    board[1] = WHITE;
    board[2] = WHITE;
    board[3] = WHITE;
    board[4] = BLACK;
    board[5] = WHITE;
    board[6] = WHITE;
    board[7] = WHITE;
    board[8] = WHITE;
    board[9] = BLACK;
    board[10] = WHITE;
    board[11] = WHITE;
    board[12] = WHITE;
    board[13] = WHITE;
    board[15] = WHITE;
    board[16] = WHITE;
    board[17] = WHITE;
    board[18] = WHITE;
    board[19] = WHITE;
    board[20] = BLACK;
    board[21] = WHITE;
    board[22] = WHITE;
    board[23] = WHITE;
    board[24] = WHITE;
    board[25] = BLACK;
    board[26] = WHITE;
    board[27] = WHITE;
    board[28] = WHITE;
    board[29] = WHITE;
    board[30] = WHITE;
    board[31] = BLACK;
    board[32] = WHITE;
    board[33] = WHITE;
    board[34] = WHITE;
    board[35] = WHITE;
    mustplay = getMustplay(6, board, true);
    expectedMustplay = {0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,1,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0,
                        0,0,0,0,0,0};
    assert(mustplay == expectedMustplay);
    assert(isTerminal(6, board, true) == false);

    delete[] board;
}

int main() {
    initialize("/home/luke/benzene-vanilla-cmake/build/src/mohex/mohex");
    runTests();
}
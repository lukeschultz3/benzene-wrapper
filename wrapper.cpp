/*
 * A wrapper that gets mustplay information from benzene
 * Written by Luke Schultz
 * June 2023
 *
 * Coprocess code taken from Advanced Programming in the UNIX Environment
 * Section 15.4: Coprocesses
 */

#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>
#include <cassert>

#define MAXLINE 2000
#define BLANK   0
#define BLACK   1
#define WHITE   2

#define VERBOSE false

using namespace std;

int n, fd1[2], fd2[2];

void initialize() {
    pid_t pid;
    
    if (pipe(fd1) < 0 || pipe(fd2) < 0)
        cout << strerror(errno) << endl;

    if ((pid = fork()) < 0) {
        cout << strerror(errno) << endl;
    } else if (pid > 0) {  // parent
        close(fd1[0]);
        close(fd2[1]);
    } else {  // child
        close(fd1[1]);
        close(fd2[0]);
        if (fd1[0] != STDIN_FILENO) {
            if (dup2(fd1[0], STDIN_FILENO) != STDIN_FILENO)
                cout << strerror(errno) << endl;
            close(fd1[0]);
        }

        if (fd2[1] != STDIN_FILENO) {
            if (dup2(fd2[1], STDOUT_FILENO) != STDOUT_FILENO)
                cout << strerror(errno) << endl;
            close(fd2[1]);
        }
        
        char *benzenePath = "/home/luke/benzene-vanilla-cmake/build/src/mohex/mohex";
        if (execvp(benzenePath, NULL) < 0) {
            cout << strerror(errno) << endl;
        }

        exit(0);
    }
}

void runParent() {
    char line[MAXLINE];

    while (fgets(line, MAXLINE, stdin) != NULL) {
        n = strlen(line);
        if (write(fd1[1], line, n) != n)
            cout << strerror(errno) << endl;
        if ((n = read(fd2[0], line, MAXLINE)) < 0)
            cout << strerror(errno) << endl;
        if (n == 0) {
            cout << strerror(errno) << endl;
            break;
        }

        line[n] = 0;  // null terminate
        if (fputs(line, stdout) == EOF)
            cout << strerror(errno) << endl;
    }

    if (ferror(stdin))
        cout << strerror(errno) << endl;
    exit(0);

}

vector <bool> getMustplay(uint8_t sideLength, uint8_t *board,
                          bool blackToPlay) {
    /*
     * Gets the mustplay from benzene for a given (square) board position
     *
     * Parameters:
     *   uint8_t  sideLength:    Side length of the board.
     *   uint8_t* board:         Array of size sideLength**2 representing
     *                           the position.
     *                           ASSUMES that 0 is BLANK, 1 is BLACK,
     *                           and 2 is WHITE.
     *                           To change this, change the macros
     *   bool     blackToPlay:   True if black is to play.
     *                           Determines which player to find mustplay for.
     * 
     * Returns:
     *   vector <bool> mustplay: Array of size sideLength**2 where an element
     *                           is true if it corresponds to a cell in the
     *                           mustplay.
     */

    vector <bool> mustplay(sideLength*sideLength, true);

    //========================================================================
    // Generate commands
    vector <string> lines;
    lines.push_back("boardsize " + to_string(sideLength) + "\n");
    for (int i = 0; i < sideLength * sideLength; i++) {
        if (board[i] == BLANK) {
            continue;
        }

        mustplay[i] = false;  // can't play in tiled cells

        if (board[i] == BLACK) {
            lines.push_back("play b ");
        } else {  // WHTIE
            lines.push_back("play w ");
        }
        lines[lines.size()-1] += (char) (i%6) + 97;
        lines[lines.size()-1] += to_string((i/6)+1);
        lines[lines.size()-1] += "\n";
    }

    if (blackToPlay) {
        lines.push_back("vc-build b\n");
    } else {
        lines.push_back("vc-build w\n");
    }
    //========================================================================

    //========================================================================
    // Send commands
    char line[MAXLINE];
    for (int i = 0; i < lines.size(); i++) {
        if (VERBOSE)
            cout << lines[i];
        strcpy(line, lines[i].c_str());
        n = strlen(line);

        if (write(fd1[1], line, n) != n)
            cout << strerror(errno) << endl;
        // automatically overwrites benzene output because
        // we only care about the output from the final command
        // (vc-build)
        if ((n = read(fd2[0], line, MAXLINE)) < 0)
            cout << strerror(errno) << endl;
        if (n == 0) {
            cout << strerror(errno) << endl;
            break;
        }
    }
    //========================================================================

    // The parsing rule is as follows:
    // Split by spaces, form contiguous pairs.
    // The first element in a pair is a coordinate to be excluded.
    // Every coordinate that isn’t excluded is the mustplay.
    int index = 3;
    bool coord = true;
    while (index < n) {
        if (line[index] == ' ') {
            coord = !coord;
            index++;
        } else if (coord) {
            int a = ((line[index]-97)) + ((line[index+1]-48)-1)*sideLength;
            mustplay[a] = false;
            index += 2;
        } else {
            index++;
        }
    }

    return mustplay;
}

bool isTerminal(uint8_t sideLength, uint8_t *board, bool blackToPlay) {
    /*
     * Checks if there is a winning virtual connection for the current player
     *
     * Parameters:
     *   uint8_t  sideLength:    Side length of the board.
     *   uint8_t* board:         Array of size sideLength**2 representing
     *                           the position.
     *                           ASSUMES that 0 is BLANK, 1 is BLACK,
     *                           and 2 is WHITE.
     *                           To change this, change the macros
     *   bool     blackToPlay:   True if black is to play.
     *                           Determines which player to find mustplay for.
     * 
     * Returns:
     *   bool     isTerminal:    True if there is a winning VC for the player
     */

    //========================================================================
    // Generate commands
    vector <string> lines;
    lines.push_back("boardsize " + to_string(sideLength) + "\n");
    for (int i = 0; i < sideLength * sideLength; i++) {
        if (board[i] == BLANK) {
            continue;
        }

        if (board[i] == BLACK) {
            lines.push_back("play b ");
        } else {  // WHTIE
            lines.push_back("play w ");
        }
        lines[lines.size()-1] += (char) (i%6) + 97;
        lines[lines.size()-1] += to_string((i/6)+1);
        lines[lines.size()-1] += "\n";
    }

    if (blackToPlay) {
        lines.push_back("vc-build b\n");
        lines.push_back("vc-between-cells-full b north south\n");
    } else {
        lines.push_back("vc-build w\n");
        lines.push_back("vc-between-cells-full w east west\n");
    }
    //========================================================================

    //========================================================================
    // Send commands
    char line[MAXLINE];
    for (int i = 0; i < lines.size(); i++) {
        if (VERBOSE)
            cout << lines[i];
        strcpy(line, lines[i].c_str());
        n = strlen(line);

        if (write(fd1[1], line, n) != n)
            cout << strerror(errno) << endl;
        // automatically overwrites benzene output because
        // we only care about the output from the final command
        // (vc-between-cells-full)
        if ((n = read(fd2[0], line, MAXLINE)) < 0)
            cout << strerror(errno) << endl;
        if (n == 0) {
            cout << strerror(errno) << endl;
            break;
        }
    }
    //========================================================================

    if (n > 4)
        return true;

    return false;
}

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
}


int main() {
    initialize();
    runTests();
    //sleep(2);

    /*
    uint8_t board[] = {0, 0, 0, 0, 0, 0,
                       0, 0, 0, 1, 0, 0,
                       0, 1, 0, 0, 0, 0,
                       0, 0, 0, 0, 0, 0,
                       0, 0, 0, 2, 0, 0,
                       0, 0, 0, 0, 0, 0};
    vector <bool> mustplay = getMustplay(6, board, true);
    for (int i = 0; i < 36; i++) {
        cout << mustplay[i];
    }
    cout << endl;
    */
    //runParent();
}

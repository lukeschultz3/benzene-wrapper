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
#include <vector>
#include <cassert>

#define MAXLINE 2000
#define BLANK   0
#define BLACK   1
#define WHITE   2

#define VERBOSE false

using namespace std;

int n, fd1[2], fd2[2];

void initialize(string benzenePath) {
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
        
        if (execl(benzenePath.c_str(), NULL) < 0) {
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
        lines[lines.size()-1] += (char) (i%sideLength) + 97;
        lines[lines.size()-1] += to_string((i/sideLength)+1);
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

void getMustplayAndFillin(uint8_t sideLength, uint8_t *board, uint8_t *fillin,
                          bool* mustplay, bool blackToPlay) {
    /*
     * Gets the mustplay from benzene for a given (square) board position
     *
     * Parameters:
     *   uint8_t  sideLength:    Side length of the board.
     * 
     *   uint8_t* board:         Array of size sideLength**2 representing
     *                           the position.
     *                           ASSUMES that 0 is BLANK, 1 is BLACK,
     *                           and 2 is WHITE.
     *                           To change this, change the macros
     * 
     *   uint8_t* fillin:        Array of size sideLength**2 that will
     *                           contain the position as well as the fillins.
     * 
     *   bool*    mustplay:      Array of size sideLength**2 that will be
     *                           filled with the mustplay. An element will
     *                           be true if it corresponds to a cell in the
     *                           mustplay.
     * 
     *   bool     blackToPlay:   True if black is to play.
     *                           Determines which player to find mustplay for.
     * 
     */

    //vector <bool> mustplay(sideLength*sideLength, true);
    //vector <uint8_t> fillin(sideLength*sideLength, BLANK);

    //========================================================================
    // Generate commands
    vector <string> lines;
    lines.push_back("boardsize " + to_string(sideLength) + "\n");
    for (int i = 0; i < sideLength * sideLength; i++) {
        if (board[i] == BLANK) {
            continue;
        }

        mustplay[i] = false;  // can't play in tiled cells
        fillin[i] = board[i];

        if (board[i] == BLACK) {
            lines.push_back("play b ");
        } else {  // WHTIE
            lines.push_back("play w ");
        }
        lines[lines.size()-1] += (char) (i%sideLength) + 97;
        lines[lines.size()-1] += to_string((i/sideLength)+1);
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
    // 
    // Additionally, if a coordinate is followed by fb or fw then it
    // is a fillin
    int index = 3;
    int currentCoord = NULL;
    bool coord = true;
    while (index < n) {
        if (line[index] == ' ') {
            coord = !coord;
        } else if (coord) {
            currentCoord = ((line[index]-97)) + ((line[index+1]-48)-1)
                            * sideLength;
            mustplay[currentCoord] = false;
            index++;  // extra increment to skip next character
        } else if (!coord && line[index] == 'f') {
            if (line[index+1] == 'b') {
                fillin[currentCoord] = BLACK;
            } else if (line[index+1] == 'w') {
                fillin[currentCoord] = WHITE;
            }
        }

        index++;
    }
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
        lines[lines.size()-1] += (char) (i%sideLength) + 97;
        lines[lines.size()-1] += to_string((i/sideLength)+1);
        lines[lines.size()-1] += "\n";
    }

    if (blackToPlay) {
        lines.push_back("vc-build b\n");
        lines.push_back("vc-between-cells-semi b north south\n");
    } else {
        lines.push_back("vc-build w\n");
        lines.push_back("vc-between-cells-semi w east west\n");
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

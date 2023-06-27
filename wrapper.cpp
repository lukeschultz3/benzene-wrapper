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

#define MAXLINE 2000
#define BLANK   0
#define BLACK   1
#define WHITE   2

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

bool* getMustplay(uint8_t size, uint8_t *board, bool blackToPlay) {
    // how to show terminal??

    vector <string> lines;
    lines.push_back("boardsize " + to_string(size) + "\n");
    for (int i = 0; i < size * size; i++) {
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
    lines.push_back("showboard\n");

    if (blackToPlay) {
        lines.push_back("vc-build b\n");
    } else {
        lines.push_back("vc-build w\n");
    }


    char line[MAXLINE];
    for (int i = 0; i < lines.size(); i++) {
        cout << lines[i] << endl;
        strcpy(line, lines[i].c_str());
        n = strlen(line);

        if (write(fd1[1], line, n) != n)
            cout << strerror(errno) << endl;
        if ((n = read(fd2[0], line, MAXLINE)) < 0)
            cout << strerror(errno) << endl;
        if (n == 0) {
            cout << strerror(errno) << endl;
            break;
        }

        line[n] = 0; // null terminate
        if (fputs(line, stdout) == EOF)
            cout << strerror(errno) << endl;
    }

    return (bool*) true;
}


int main() {
    initialize();
    sleep(2);

    uint8_t board[] = {0, 0, 0, 0, 0, 0,
                       0, 0, 0, 1, 0, 0,
                       0, 1, 0, 0, 0, 0,
                       0, 0, 0, 0, 0, 0,
                       0, 0, 0, 2, 0, 0,
                       0, 0, 0, 0, 0, 0};
    getMustplay(6, board, true);
    //runParent();
}

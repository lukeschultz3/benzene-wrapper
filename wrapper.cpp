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

#define MAXLINE 240

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

void run_parent() {
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


int main() {
    initialize();
    run_parent();

    /*
    int     n, fd1[2], fd2[2];
    pid_t   pid;
    char    line[MAXLINE];

    if (pipe(fd1) < 0 || pipe(fd2) < 0)
        cout << strerror(errno) << endl;

    if ((pid = fork()) < 0) {
        cout << strerror(errno) << endl;
    } else if (pid > 0) {  // parent
        close(fd1[0]);
        close(fd2[1]);

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
    */
}

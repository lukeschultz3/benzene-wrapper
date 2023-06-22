/*
 * A wrapper that gets mustplay information from benzene
 * Written by Luke Schultz
 * June 2023
 */

#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;

int main() {

    int wbfd[2];  // wrapper to benzene pipe
    int bwfd[2];  // benzene to wrapper pipe
    if (pipe(wbfd) < 0)
        cout << strerror(errno) << endl;
    if (pipe(bwfd) < 0)
        cout << strerror(errno) << endl;

    pid_t pid = fork();
    if (pid > 0) {  // parent
        sleep(3);
        close(bwfd[1]);
        //close(wbfd[0]);
        char line[30];
        read(bwfd[0], line, 30);
        cout << "parent: writing" << endl;
        char send[] = "hello world";
        dup2(wbfd[0], STDOUT_FILENO);
        write(wbfd[1], send, sizeof send);
        memset(line, 0, sizeof line);
        //read(wbfd[0], line, 30);
        cout << "parent: reading" << endl;
        read(bwfd[0], line, 30);
        cout << "parent received: " << line << endl;

        for (int i = 0; i < 30; i++) {
            cout << (int) line[i] << " ";
        }
        cout << endl;

    } else {  // child
        /*
        int fd = open("./output.txt", O_WRONLY);
        if (fd < 0) {
            cout << strerror(errno) << endl;
        }
        dup2(fd, 1);
        */
        dup2(bwfd[1], 1);  // redirect the output (STDOUT to the pipe)
        dup2(wbfd[0], STDIN_FILENO);
        //close(fd);
        //close(bwfd[0]);
        //close(bwfd[1]);
        //close(wbfd[0]);
        //close(wbfd[1]);
        //cout << "child, executing" << endl;

        char *interpreter = "python3";
        char *pythonPath = "./test.py";
        char *pythonArgs[] = {interpreter, pythonPath, NULL};
        if (execvp(interpreter, pythonArgs) < 0) {
            cout << strerror(errno) << endl;
        }

        cout << "child, done executing" << endl;
    }

}

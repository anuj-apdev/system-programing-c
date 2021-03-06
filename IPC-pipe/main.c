#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // create pipe
    // fork
    //   child:
    //     close stdin & stderr
    //     dup pipe to stdout
    //     exec say or fork_say
    // fork again:
    //   child:  
    //     read from pipe until its end and pint out
    // waitpid for the first child and report
    // waitpid for the second child
    // exit
    
    if (argc != 2) {
        printf("executable is not specified\n");
        return 1;
    }

    printf("start\n");

    int fds[2];  // fds[0] read; fds[1] write
    if (0 != pipe(fds)) {
        perror("pipe() failed");
        return 1;
    }

    int pid1 = fork();
    if (pid1 < 0) {
        perror("fork() failed (1)");
        return 1;
    }

    //First child flow
    if (pid1 == 0) {
        printf("first child (pid=%d)\n", getpid());
        // Close read end of pipe in the child process
        // This child will just write it's STDOUT to write end of pipe
        close(fds[0]); 

        // Open a file descriptor for /dev/null
        int std_null_r = open("/dev/null", O_RDONLY);
        if (std_null_r < 0) {
            perror("open('/dev/null', O_RDONLY) failed");
            exit(1);
        }

        // Duplicate/redirect STDIN to null, 
        // no inputs will be taken from standard input for child1
        if (dup2(std_null_r, 0) < 0) {
            perror("dup2(STDIN) failed");
            exit(1);
        }

        // Open FD for null
        int std_null_w = open("/dev/null", O_WRONLY);
        if (std_null_w < 0) {
            perror("open('/dev/null', O_RDONLY) failed");
            exit(1);
        }

        // Pipe/Redirect STDERR to null
        if (dup2(std_null_w, 2) < 0) {
            perror("dup2(STDERR) failed");
            exit(1);
        }

        // Pipe/Redirect STDOUT to write end of fds shared with parent
        if (dup2(fds[1], 1) < 0) {
            perror("dup2(STDOUT) failed");
            exit(1);
        }

        // Execute the passed child process
        execl(argv[1], argv[1], NULL);
        _exit(127);
    }

    // Parent process flow
    // close the write end of pipe (Only child 1 should be able to arite to the pipe)
    close(fds[1]); 

    //fork second child
    int pid2 = fork();
    if (pid2 < 0) {
        perror("fork() failed (2)");
        return 1;
    }

    //Flow for second child process
    if (pid2 == 0) {
        printf("second child (pid=%d)\n", getpid());
        char buf[256];
        int nread = 0;
        // just read from Read and of pipe, in a loop
        do {
            nread = read(fds[0], buf, 254);
            buf[nread] = '\0';
            printf("second child read %d bytes: %s\n", nread, buf);
        } while (nread > 0);
        printf("Nothing left to read\n");
        sleep(5);
        printf("exiting second child\n");
        return 0;
    }

    // Wait for some child process to return
    int pid3 = waitpid(-1, NULL, 0);
    printf("waitpid returned %d\n", pid3);
    // Wait for some other (Second) child procccccess to finish
    int pid4 = waitpid(-1, NULL, 0);
    printf("waitpid returned %d\n", pid4);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void create_process_chain(int n) {
    if (n <= 0) {
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        printf("Enfant (PID: %d, PPID: %d) en cours d'exécution\n", getpid(), getppid());
        create_process_chain(n - 1);
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    create_process_chain(N);
    printf("Tous les enfants ont terminé\n");
    return 0;
}

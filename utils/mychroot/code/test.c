#include <stdio.h>
#include <unistd.h>
#include <sys/personality.h>
#include <errno.h>

int main() {
    // Récupère l'ID du processus
    pid_t pid = getpid();
    printf("L'ID du processus est: %d\n", pid);

    // Tente de changer la personnalité du processus
    int result = personality(PER_LINUX);

    if (result == -1) {
        perror("L'appel à personality a échoué");
    } else {
        printf("L'appel à personality a réussi (ce qui ne devrait pas arriver si seccomp est actif)\n");
    }

    return 0;
}


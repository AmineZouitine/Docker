#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <size_in_gigabytes>\n", argv[0]);
        return 1;
    }

    long size_in_gigabytes = atol(argv[1]);
    long size_in_bytes = size_in_gigabytes * (1024L * 1024L * 1024L);

    // Allocation de la mémoire
    char *memory = (char *)malloc(size_in_bytes);
    if (memory == NULL) {
        perror("Failed to allocate memory");
        return 1;
    }

    // Obtenir la taille de la page système
    long page_size = sysconf(_SC_PAGESIZE);

    // Écrire dans chaque page pour s'assurer que la mémoire est allouée
    for (long i = 0; i < size_in_bytes; i += page_size) {
        memory[i] = 0;
    }

    printf("Allocated and used %ld gigabytes of memory.\n", size_in_gigabytes);

    // Faire quelque chose avec la mémoire ici
    // ...

    // Libération de la mémoire
    free(memory);

    return 0;
}

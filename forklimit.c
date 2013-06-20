#define LIBC_NAME "libc.so.6"
#define FORK_LIMIT 50

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>

static void *libc_handle = NULL;
static pid_t (*fork_ptr)(void);

static int *fork_count;

static void init(void)
{
    // Get a pointer to the original fork function
    libc_handle = dlopen(LIBC_NAME, RTLD_LAZY);
    if (!libc_handle) {
        fprintf(stderr, "forklimit: cannot open libc: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    fork_ptr = dlsym(libc_handle, "fork");
    if (!fork_ptr) {
        fprintf(stderr, "forklimit: cannot find function fork: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    // Get shared memory for fork counter
    char *filename = getenv("LD_PRELOAD");
    key_t key = ftok(filename, 1337);
    if (key == -1) {
        perror(filename);
        exit(EXIT_FAILURE);
    }
    int shmid = shmget(key, sizeof(int), 0644 | IPC_CREAT);
    if (shmid == -1) {
        perror("forklimit: shmget");
        exit(EXIT_FAILURE);
    }
    fork_count = shmat(shmid, NULL, 0);
    if (fork_count == (void *)(-1)) {
        perror("forklimit: shmat");
        exit(EXIT_FAILURE);
    }
    *fork_count = 0;
}

// Replacement fork
pid_t fork(void)
{
    if (!libc_handle)
        init();

    // Fork limit has already been reached, exit silently
    if (*fork_count == -1)
        exit(EXIT_FAILURE);

    if (++*fork_count > FORK_LIMIT) {
        *fork_count = -1;
        fprintf(stderr, "forklimit: fork limit exceeded\n");
        exit(EXIT_FAILURE);
    }

    return (*fork_ptr)();
}

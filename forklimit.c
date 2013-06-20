#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>

#define LIBC_NAME "libc.so.6"
#define DEFAULT_LIMIT 50

static void *libc_handle = NULL;
static pid_t (*fork_ptr)(void);

static int *fork_count;
static int fork_limit;

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
    int shmid = shmget(IPC_PRIVATE, sizeof(int), 0644 | IPC_CREAT);
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

    // Get the fork limit
    char *env_limit = getenv("FORK_LIMIT");
    if (env_limit)
        fork_limit = atoi(env_limit);
    if (!fork_limit)
        fork_limit = DEFAULT_LIMIT;
}

// Replacement fork
pid_t fork(void)
{
    if (!libc_handle)
        init();

    // Fork limit has already been reached, exit silently
    if (*fork_count == -1)
        exit(EXIT_FAILURE);

    if (++*fork_count > fork_limit) {
        *fork_count = -1;
        fprintf(stderr, "forklimit: fork limit exceeded\n");
        exit(EXIT_FAILURE);
    }

    return (*fork_ptr)();
}

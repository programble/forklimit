#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>

#define OUT_PREFIX "forklimit: "
#define LIBC_NAME "libc.so.6"
#define DEFAULT_LIMIT 100

static pid_t (*fork_ptr)(void);
static int (*execve_ptr)(const char *, char *const [], char *const []);

static char *ld_preload;

static int *fork_count;
static int fork_limit;

static inline void *get_func_ptr(void *handle, char *sym)
{
    void *ptr = dlsym(handle, sym);
    if (!ptr) {
        fprintf(stderr, OUT_PREFIX "cannot find function %s: %s\n", sym, dlerror());
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static void __attribute__((constructor)) init(void)
{
    // Get pointers to original functions
    void *handle = dlopen(LIBC_NAME, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, OUT_PREFIX "cannot open libc: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    fork_ptr = get_func_ptr(handle, "fork");
    execve_ptr = get_func_ptr(handle, "execve");

    // Get shared memory for fork counter
    fork_count = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (fork_count == MAP_FAILED) {
        perror(OUT_PREFIX "mmap");
        exit(EXIT_FAILURE);
    }
    *fork_count = 0;

    // Get the fork limit
    char *env_limit = getenv("FORK_LIMIT");
    if (env_limit)
        fork_limit = atoi(env_limit);
    if (!fork_limit)
        fork_limit = DEFAULT_LIMIT;

    // Save initial value of LD_PRELOAD
    ld_preload = strdup(getenv("LD_PRELOAD"));
}

// Replacement functions

pid_t fork(void)
{
    // Fork limit has already been reached, exit silently
    if (*fork_count == -1)
        exit(EXIT_FAILURE);

    if (++*fork_count > fork_limit) {
        *fork_count = -1;
        fprintf(stderr, OUT_PREFIX "fork limit exceeded\n");
        exit(EXIT_FAILURE);
    }

    return (*fork_ptr)();
}

int execve(const char *filename, char *const argv[], char *const envp[])
{
    // Succeed only if LD_PRELOAD hasn't changed
    for (char *const *env = envp; *env; env++) {
        if (!strncmp(*env, "LD_PRELOAD", 10) && !strcmp(*env + 11, ld_preload))
            return (*execve_ptr)(filename, argv, envp);
    }
    fprintf(stderr, OUT_PREFIX "LD_PRELOAD changed\n");
    exit(EXIT_FAILURE);
}

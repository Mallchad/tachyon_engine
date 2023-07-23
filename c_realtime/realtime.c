
#include <sys/resource.h>
#include <sys/unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sched.h>
#include <time.h>

#include <stdio.h>
#include <errno.h>

pid_t g_pid                     = 0;
void* shared_mem                = NULL;
unsigned char* shared_mem_head  = NULL;
const int shared_mem_size       = 1000;

int* g_child_pid = NULL;
int* g_exec_starting = NULL;

int main(int argc, char** argv)
{
    const char* shed_error[10] = {};

    shared_mem = (int*)mmap(NULL,
                            shared_mem_size,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED | MAP_ANONYMOUS,
                            0,
                            0);
    if (shared_mem != MAP_FAILED || shared_mem != NULL)
    {
        shared_mem_head = shared_mem;

        g_child_pid = (int*)shared_mem_head;
        shared_mem_head += sizeof(int);
        *g_child_pid = -1;

        g_exec_starting = (int*)shared_mem_head;
        shared_mem_head += sizeof(int);
        *g_exec_starting = 0;
    }

    // Must have a target executable
    if (argc < 2)
    {
        printf("%s", "Must have a target executable \n");
        errno = EINVAL;
        return EXIT_FAILURE;
    }
    // Don't allow abitrarily large command lists
    if (argc > 100)
    {
        printf("%s", "There are too many arguments passed, stopping in case of malicious content \n");
        errno = E2BIG;
        return EXIT_FAILURE;
    }

    int safety_padding = 4;
    // Hard cap paramter list
    enum { target_arguments_size = 1000 };
    const char* target_arg_process = argv[1];
    char target_process[100] = {};

    for (int i_arg = 0; i_arg < argc; ++i_arg )
    {
        // Skip argument 0 and 1 its just self and a command
        for (int i_byte = 0; argv[i_arg][i_byte] != '\0' && i_byte < 100; ++i_byte)
        {
            if (i_byte >= 100)
            {
                perror("One of the arguments larger than 100 characters large, bailing.");
                errno = EINVAL;
            }
        }
    }
    for (int i_byte = 0; i_byte < 100 && target_arg_process[i_byte] != '\0'; ++i_byte)
    {
        target_process[i_byte] = target_arg_process[i_byte];
    }

    g_pid = fork();
    int error_code = -1;

    // Program Thread
    if (g_pid == 0)
    {
        *g_child_pid = getpid();

        *g_exec_starting = 1;
        // Replace the process and go away
        error_code = execvp((const char*)target_process, argv + 1);
    }
    // Wrapper Thread
    else
    {
        // Race mitigation
        while (g_exec_starting == 0) sleep(1);

        sleep(1);


        int thread_scheduler = -1;
        int thread_priority = -1;
        int thread_priority_errno = -1;
        int rt_error = -1;
        struct sched_param shed_test = { .sched_priority = -1 };

        thread_scheduler = sched_getscheduler(*g_child_pid);
        thread_priority_errno = sched_getparam(*g_child_pid, &shed_test);
        thread_priority = shed_test.sched_priority;

        printf("%s%d%s", "Target PID: ", *g_child_pid, "\n");
        printf("%s%d%s", "Initial scheduling policy of PID: ", *g_child_pid, "\n");
        printf("%d%s", thread_scheduler, "\n");
        printf("%s%d%s", "Initial scheduling priority of PID: ", *g_child_pid, "\n");
        printf("%d%s", thread_priority, "\n");

        struct sched_param args = { .sched_priority = 99 };
        rt_error = sched_setscheduler(*g_child_pid, SCHED_FIFO, &args);
        if (rt_error)
        {
            if (errno == EINVAL) perror("Argument for scheduler is invalid \n");
            if (errno == EPERM) perror("The calling thread does not have appropriate privileges \n");
            printf("Error code %d in '%s:%d' \n", errno, __FILE__, __LINE__);
            raise(SIGABRT);
        }

        thread_scheduler = sched_getscheduler(*g_child_pid);
        thread_priority_errno = sched_getparam(*g_child_pid, &shed_test);
        thread_priority = shed_test.sched_priority;
        printf("%s%d%s", "New scheduling policy of PID: ", *g_child_pid, "\n");
        printf("%d%s", thread_scheduler, "\n");
        printf("%s%d%s", "New scheduling priority of PID: ", *g_child_pid, "\n");
        printf("%d%s", thread_priority, "\n");

        // Hold onto process to help maintain the process tree
        wait(0);
        munmap(shared_mem, shared_mem_size);
    }

    return error_code;

}

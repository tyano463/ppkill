#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include <dirent.h>
#include <signal.h>

#include "dlog.h"

#ifndef PATH_MAX
#define PATH_MAX 512
#endif

#define RA2L1_UART_NUM 10
#define PROC_DIR "/proc"
#define CMDLINE_FILE "comm"

void chomp(char *s)
{
    for (int i = 0; s[i]; i++)
    {
        if (s[i] == '\n')
        {
            s[i] = '\0';
            break;
        }
    }
}

void usage(void)
{
    pid_t pid = getpid();
    char buf[PATH_MAX];
    FILE *fp;
    sprintf(buf, PROC_DIR "/%ld/comm", pid);
    fp = fopen(buf, "r");
    if (fp)
    {
        fgets(buf, PATH_MAX, fp);
        chomp(buf);
        printf("usage:\n\t%s [process_name]\n", buf);
        fclose(fp);
    }
}

bool chkarg(int argc, char *argv[])
{
    return argc == 2;
}

char *get_process_name(pid_t pid)
{
    static char name[PATH_MAX];
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        perror("fopen");
        return NULL;
    }
    if (fgets(name, sizeof(name), file) == NULL)
    {
        fclose(file);
        return NULL;
    }
    fclose(file);
    return name;
}

pid_t get_parent_pid(pid_t *_pid)
{
    pid_t ppid = getpid();
    char path[PATH_MAX];

    pid_t pid = *_pid;

    ERR_RETn(!pid);

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        perror("fopen");
        return -1;
    }
    fscanf(file, "%*d %*s %*c %d", &ppid);
    fclose(file);
error_return:
    return ppid;
}

bool is_shell(const char *s)
{
    int namelen = 0;
    bool ret = false;
    const char *shells[] = {
        "sh",
        "bash",
        "zsh",
        "dash",
        "ksh",
        "tcsh",
        "csh",
        "fish",
        NULL,
    };

    for (int i = 0; s[i]; i++)
    {
        if (s[i] == '\n')
        {
            namelen = i;
            break;
        }
    }

    ERR_RETn(!namelen);
    for (int i = 0; shells[i]; i++)
    {
        if (strncmp(s, shells[i], namelen) == 0)
        {
            ret = true;
            break;
        }
    }

error_return:
    return ret;
}

pid_t find_pid(const char *process_name)
{
    DIR *dir;
    struct dirent *entry;
    pid_t pid = 0;
    if ((dir = opendir(PROC_DIR)) == NULL)
    {
        perror("opendir");
        return pid;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            char *endptr;
            long lpid = strtol(entry->d_name, &endptr, 10);
            if (*endptr == '\0')
            { // It's a number, thus a PID directory
                char path[256];
                snprintf(path, sizeof(path), PROC_DIR "/%ld/" CMDLINE_FILE, lpid);
                FILE *fp = fopen(path, "r");
                if (fp)
                {
                    char cmdline[256];
                    if (fgets(cmdline, sizeof(cmdline), fp) != NULL)
                    {
                        if (strncmp(cmdline, process_name, strlen(process_name)) == 0)
                        {
                            // printf("%ld\n", lpid);
                            pid = lpid;
                        }
                    }
                    fclose(fp);
                }
            }
        }
        if (pid)
            break;
    }
    return pid;
}

int main(int argc, char *argv[])
{
    pid_t ppid = 0;
    pid_t pid = 0;
    char *s;

    if (!chkarg(argc, argv))
    {
        usage();
        goto error_return;
    }

    do
    {
        if (!pid)
            pid = find_pid(argv[1]);
        else
            pid = ppid;
        if (!pid) {
            printf("%s not found\n", argv[1]);
            return 1;
        }
        ppid = get_parent_pid(&pid);
        s = get_process_name(ppid);
    } while (!is_shell(s));

    // dlog("pid:%d", pid);
    // printf("%ld\n", pid);
    kill(pid, SIGKILL);

error_return:
    return 0;
}

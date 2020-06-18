#include "compare.h"

#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>

int compare_diff(const char *file1,const char *file2)
{
    char diff[1024];
    sprintf(diff,"diff -q -B -b -w --strip-trailing-cr %s %s", file1, file2);
    int d = system(diff);
    if (d) return OJ_WA;
    sprintf(diff,"diff -q -B --strip-trailing-cr %s %s", file1, file2);
    int p = system(diff);
    if (p) return OJ_PE;
    else return OJ_AC;
}

int compare(const char *file1, const char *file2)
{
    return compare_diff(file1, file2);
}

int compare_diff_spj(const char* spj_exe, const char* file1, const char* file2, const char* file3)
{
    int pid = fork();
    if(pid == 0)
    {
        execl(spj_exe, spj_exe, file1, file2, file3, NULL);
        exit(-1);
    }
    else if(pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
        int8_t ret = WEXITSTATUS(status);
        if(ret == -1) return OJ_SPJ_ERR;
        else if(ret == 0) return OJ_AC;
        else return OJ_WA;
    }
    else
    {
        return OJ_SPJ_ERR;
    }
}

int compare_spj(const char* spj_exe, const char* file1, const char* file2, const char* file3)
{
    return compare_diff_spj(spj_exe, file1, file2, file3);
}
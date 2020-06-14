#ifndef JUDGE_RNNNER
#define JUDGE_RNNNER

#include <string>
#include <vector>
#include <ostream>

enum
{
    SUCCESS = 0,
    WRONG_ANSWER = -1,
    CPU_TIME_LIMIT_EXCEEDED = 1,
    REAL_TIME_LIMIT_EXCEEDED = 2,
    MEMORY_LIMIT_EXCEEDED = 3,
    RUNTIME_ERROR = 4,
    SYSTEM_ERROR = 5
};

enum
{
    INVALID_CONFIG = -1,
    FORK_FAILED = -2,
    PTHREAD_FAILED = -3,
    WAIT_FAILED = -4,
    ROOT_REQUIRED = -5,
    LOAD_SECCOMP_FAILED = -6,
    SETRLIMIT_FAILED = -7,
    DUP2_FAILED = -8,
    SETUID_FAILED = -9,
    EXECVE_FAILED = -10,
    SPJ_ERROR = -11
};

struct run_params
{
    int max_cpu_time;
    int max_real_time;
    int max_memory;
    int max_stack;
    int max_output_size;
    int max_process_number;
    int memory_limit_check_only;

    int uid;
    int gid;

    std::string exe_path;
    std::string input_path;
    std::string output_path;
    std::string error_path;
    std::string log_path;

    std::string seccomp_rule_name;

    std::vector<std::string> argv;
    std::vector<std::string> env;

    friend std::ostream &operator<<(std::ostream &os, const run_params &params);
};

struct run_result
{
    int cpu_time;
    int real_time;
    int memory;
    int signal;
    int exit_code;
    int error;
    int result;

    friend std::ostream &operator<<(std::ostream &os, const run_result &result);
};

run_result run_command(const run_params& params);


#endif

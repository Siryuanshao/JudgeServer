#ifndef JUDGE_OPTION
#define JUDGE_OPTION

#include <string>
#include <vector>
#include "judge_runner.h"

enum
{
    LANG_C = 0,
    LANG_CPP = 1,
    LANG_JAVA = 2
};

enum
{
    Pending = 0,
    Judging = 1,
    Accepted = 2,
    PresentationError = 3,
    WrongAnswer = 4,
    RuntimeError = 5,
    TimeLimitExceeded = 6,
    MemoryLimitExceeded = 7,
    OutputLimitExceeded  = 8,
    CompileError = 9,
    SystemError = 10
};

struct judge_result
{
    int timeUsed;
    int memoryUsed;
    int result;
};


int mk_workdir(const std::string& work_dir);
void rm_workdir(const std::string& work_dir);
void write_solution(int lang, const std::string& path, const std::string& sourcecode);

int compile(int lang, const std::string& src_path, const std::string& output_dir);
struct judge_result run_solution(int lang,
                                 const std::string& exe_dir,
                                 const std::string& test_case,
                                 const std::string& output_file,
                                 int max_cpu_time,
                                 int max_memory,
                                 int max_output_size);

#endif

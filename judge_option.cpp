#include "judge_option.h"

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <memory>
#include <iostream>
#include <dirent.h>

#include "judge_languages.h"

#define COMPILER_LOG_PATH "/log/compile.log"
#define JUDGER_RUN_LOG_PATH "/log/judge.log"

#define OPTION_DEBUG


template<typename ... Args>
std::string string_format(const std::string& format, Args ... args )
{
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1;
    if( size <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    std::unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 );
}

std::vector<std::string> string_split(const std::string& str, const char dlm = ' ')
{
    std::vector<std::string> vec;
    std::istringstream iss(str);
    std::string temp;
    while(std::getline(iss, temp, dlm))
        vec.emplace_back(std::move(temp));
    return vec;
}

std::string join_path(const std::string& dir, const std::string& file)
{
    if(dir[dir.length() - 1] == '/')
        return dir + file;
    else
        return dir + "/" + file;
}

int mk_workdir(const std::string& work_dir)
{
    return mkdir(work_dir.c_str(), 0755);
}

void rm_workdir(const std::string& work_dir)
{
    struct dirent *entry = NULL;
    DIR *dir = NULL;
    dir = opendir(work_dir.c_str());
    entry = readdir(dir);
    while(entry)
    {
        if(*(entry->d_name) != '.')
        {
            std::string abs_path = work_dir + "/" + entry->d_name;

            struct stat s_buf;
            stat(abs_path.c_str(), &s_buf);

            if(S_ISDIR(s_buf.st_mode))
            {
                rm_workdir(abs_path);
            }
            else
            {
                remove(abs_path.c_str());
            }
        }
        entry = readdir(dir);
    }
    closedir(dir);
    remove(work_dir.c_str());
}

void write_solution(int lang, const std::string& path, const std::string& sourcecode)
{
    std::string src_name;

    switch (lang)
    {
        case LANG_C:
            src_name = c_lang_config::compile::src_name;
            break;
        case LANG_CPP:
            src_name = cpp_lang_config::compile::src_name;
            break;
        case LANG_JAVA:
            src_name = java_lang_config::compile::src_name;
            break;
        default:
            ;
    }

    std::ofstream ofs;
    ofs.open(join_path(path, src_name));
    ofs << sourcecode;
}

int compile(int lang, const std::string& src_path, const std::string& output_dir)
{
    run_params params;
    std::string command, src_file, exe_path;
    std::vector<std::string> split_commands;
    std::string comparer_out = join_path(output_dir, "compiler.out");
    switch (lang)
    {
        case LANG_C:
            command = c_lang_config::compile::compile_command;
            src_file =join_path(src_path, c_lang_config::compile::src_name);
            exe_path = join_path(output_dir, c_lang_config::compile::exe_name);
            command = string_format(command, src_file.c_str(), exe_path.c_str());

            params.max_cpu_time = c_lang_config::compile::max_cpu_time;
            params.max_real_time = c_lang_config::compile::max_real_time;
            params.max_memory = c_lang_config::compile::max_memory;

            break;
        case LANG_CPP:
            command = cpp_lang_config::compile::compile_command;
            src_file =join_path(src_path, cpp_lang_config::compile::src_name);
            exe_path = join_path(output_dir, cpp_lang_config::compile::exe_name);
            command = string_format(command, src_file.c_str(), exe_path.c_str());

            params.max_cpu_time = cpp_lang_config::compile::max_cpu_time;
            params.max_real_time = cpp_lang_config::compile::max_real_time;
            params.max_memory = cpp_lang_config::compile::max_memory;

            break;
        case LANG_JAVA:
            command = java_lang_config::compile::compile_command;
            src_file =join_path(src_path, java_lang_config::compile::src_name);
            exe_path = join_path(output_dir, java_lang_config::compile::exe_name);
            command = string_format(command, src_file.c_str(), output_dir.c_str());

            params.max_cpu_time = java_lang_config::compile::max_cpu_time;
            params.max_real_time = java_lang_config::compile::max_real_time;
            params.max_memory = java_lang_config::compile::max_memory;

            break;
        default:
            throw std::runtime_error("Compiler runtime error, info: no such language.");
    }

    split_commands = string_split(command, ' ');

    params.exe_path = split_commands[0];
    params.argv = std::vector<std::string>(split_commands.begin() + 1, split_commands.end());
    params.max_stack = 128 * 1024 * 1024;
    params.max_output_size = 1024 * 1024;
    params.max_process_number = -1;
    params.input_path = src_path;
    params.output_path = comparer_out;
    params.error_path = comparer_out;
    params.log_path = COMPILER_LOG_PATH;
    params.env = {std::string("PATH=") + getenv("PATH")};

    params.seccomp_rule_name.clear();
    params.uid = getuid();
    params.gid = getpid();
    params.memory_limit_check_only = 0;

    // chdir(output_dir.c_str());

    run_result result = run_command(params);
    if(result.result != SUCCESS)
    {
        std::ifstream ifs(comparer_out);
        std::string err;
        if(ifs.good())
        {
            err = std::string((std::istreambuf_iterator<char>(ifs)),std::istreambuf_iterator<char>());
        }
        ifs.close();
        throw std::runtime_error(err);
    }
    return 0;
}

struct judge_result run_solution(int lang,
                               const std::string& exe_dir,
                               const std::string& test_case,
                               const std::string& output_file,
                               int max_cpu_time,
                               int max_memory,
                               int max_output_size)
{
    run_params params;
    std::string command, exe_path;
    std::vector<std::string> split_commands;
    switch (lang)
    {
        case LANG_C:
            command = c_lang_config::run::command;
            exe_path = join_path(exe_dir, c_lang_config::compile::exe_name);
            command = string_format(command, exe_path.c_str());

            params.memory_limit_check_only = c_lang_config::run::memory_limit_check_only;
            params.seccomp_rule_name = c_lang_config::run::seccomp_rule;

            params.env = {std::string("PATH=") + getenv("PATH")};
            params.env.insert(params.env.end(), c_lang_config::run::env.begin(), c_lang_config::run::env.end());
            break;
        case LANG_CPP:
            command = cpp_lang_config::run::command;
            exe_path = join_path(exe_dir, cpp_lang_config::compile::exe_name);
            command = string_format(command, exe_path.c_str());

            params.memory_limit_check_only = cpp_lang_config::run::memory_limit_check_only;
            params.seccomp_rule_name = cpp_lang_config::run::seccomp_rule;

            params.env = {std::string("PATH=") + getenv("PATH")};
            params.env.insert(params.env.end(), cpp_lang_config::run::env.begin(), cpp_lang_config::run::env.end());

            break;
        case LANG_JAVA:
            command = java_lang_config::run::command;
            command = string_format(command, exe_dir.c_str(), max_memory, java_lang_config::compile::exe_name.c_str());

            params.memory_limit_check_only = java_lang_config::run::memory_limit_check_only;
            params.seccomp_rule_name = java_lang_config::run::seccomp_rule;

            params.env = {std::string("PATH=") + getenv("PATH")};
            params.env.insert(params.env.end(), java_lang_config::run::env.begin(), java_lang_config::run::env.end());
            break;
        default:
            throw std::runtime_error("Runner runtime error, info: no such language.");
    }

    split_commands = string_split(command, ' ');
    params.max_cpu_time = max_cpu_time;
    params.max_real_time = max_cpu_time * 3;
    params.max_memory = max_memory * 1024;
    params.max_stack = 128 * 1024 * 1024;
    params.max_process_number = -1;

    params.exe_path = split_commands[0];
    params.argv = std::vector<std::string>(split_commands.begin() + 1, split_commands.end());

    params.max_output_size = max_output_size;
    params.input_path = test_case;
    params.output_path = output_file;
    params.error_path = output_file;
    params.log_path = JUDGER_RUN_LOG_PATH;

    params.uid = getuid();
    params.gid = getpid();

    run_result _result = run_command(params);

    judge_result result;
    result.timeUsed = _result.cpu_time;
    result.memoryUsed = _result.memory / 1024;

    switch (_result.result)
    {
        case SUCCESS:
        case WRONG_ANSWER:
            result.result = -1;
            break;
        case CPU_TIME_LIMIT_EXCEEDED:
        case REAL_TIME_LIMIT_EXCEEDED:
            result.result = TimeLimitExceeded;
            break;
        case MEMORY_LIMIT_EXCEEDED:
            result.result = MemoryLimitExceeded;
            break;
        case RUNTIME_ERROR:
            result.result = RuntimeError;
            break;
        case SYSTEM_ERROR:
            result.result = SystemError;
            break;
        default:
            result.result = SystemError;
    }

    return result;
}
#include <iostream>
#include "judge_runner.h"


#define BUFFER_SIZE 1024
#define SECURE_PROC "/usr/lib/judger/libjudger.so"

#define SANDBOX_DEBUG

#define formatter(buf, format, ...) (sprintf(buf, format, ##__VA_ARGS__), buf)



inline int scan_d(FILE* fp)
{
    int ret = 0;
    char c;
    if (c = fgetc(fp), c == EOF)
        return -1;
    while (c != '-' && (c < '0' || c > '9'))
        c = fgetc(fp);
    int sgn = (c == '-') ? -1 : 1;
    ret = (c == '-') ? 0 : (c - '0');
    while (c = fgetc(fp), c >= '0' && c <= '9')
        ret = ret * 10 + (c - '0');
    ret *= sgn;
    return ret;
}

std::string build_command(const run_params& params)
{
    char buf[BUFFER_SIZE];

    std::vector<std::string> proc_args = {SECURE_PROC};

    proc_args.emplace_back(formatter(buf, "--%s=%s", "exe_path", params.exe_path.c_str()));
    proc_args.emplace_back(formatter(buf, "--%s=%s", "input_path", params.input_path.c_str()));
    proc_args.emplace_back(formatter(buf, "--%s=%s", "output_path", params.output_path.c_str()));
    proc_args.emplace_back(formatter(buf, "--%s=%s", "error_path", params.error_path.c_str()));
    proc_args.emplace_back(formatter(buf, "--%s=%s", "log_path", params.log_path.c_str()));

    if(params.max_cpu_time != -1)
        proc_args.emplace_back(formatter(buf, "--%s=%d", "max_cpu_time", params.max_cpu_time));
    if(params.max_real_time != -1)
        proc_args.emplace_back(formatter(buf, "--%s=%d", "max_real_time", params.max_real_time));
    if(params.max_memory != -1)
        proc_args.emplace_back(formatter(buf, "--%s=%d", "max_memory", params.max_memory));
    if(params.max_stack != -1)
        proc_args.emplace_back(formatter(buf, "--%s=%d", "max_stack", params.max_stack));
    if(params.max_output_size != -1)
        proc_args.emplace_back(formatter(buf, "--%s=%d", "max_output_size", params.max_output_size));
    if(params.max_process_number != -1)
        proc_args.emplace_back(formatter(buf, "--%s=%d", "max_process_number", params.max_process_number));
    if(params.uid != -1)
        proc_args.emplace_back(formatter(buf, "--%s=%d", "uid", params.uid));
    if(params.gid != -1)
        proc_args.emplace_back(formatter(buf, "--%s=%d", "gid", params.gid));
    if(params.memory_limit_check_only != -1)
        proc_args.emplace_back(formatter(buf, "--%s=%d", "memory_limit_check_only", params.memory_limit_check_only));

    if(!params.seccomp_rule_name.empty())
        proc_args.emplace_back(formatter(buf, "--seccomp_rule=%s", params.seccomp_rule_name.c_str()));

    int argc = params.argv.size();
    int envc = params.env.size();

    for(int i = 0; i < argc; i++)
        proc_args.emplace_back(formatter(buf, "--%s=%s", "args", params.argv[i].c_str()));

    for(int i = 0; i < envc; i++)
        proc_args.emplace_back(formatter(buf, "--%s=%s", "env", params.env[i].c_str()));

    std::string command;

    int size = proc_args.size();

    for(int i = 0; i < size; i++)
    {
        command += proc_args[i];
        if(i != size - 1) command += " ";
    }

    return command;
}

run_result run_command(const run_params& params)
{
    std::string command = build_command(params);

#ifdef SANDBOX_DEBUG
    std::cout << "run command :" << command << std::endl;
#endif

    FILE* out = popen(command.c_str(), "r");

    run_result result = {};
    result.cpu_time = scan_d(out);
    result.real_time = scan_d(out);
    result.memory = scan_d(out);
    result.signal = scan_d(out);
    result.exit_code = scan_d(out);
    result.error = scan_d(out);
    result.result = scan_d(out);

    pclose(out);

#ifdef SANDBOX_DEBUG
    std::cout << "run result :" << result << std::endl;
#endif

    return result;
}

std::ostream &operator<<(std::ostream &os, const run_params &params) {
    os << "max_cpu_time: " << params.max_cpu_time << " max_real_time: " << params.max_real_time << " max_memory: "
       << params.max_memory << " max_stack: " << params.max_stack << " max_output_size: " << params.max_output_size
       << " max_process_number: " << params.max_process_number << " memory_limit_check_only: "
       << params.memory_limit_check_only << " uid: " << params.uid << " gid: " << params.gid << " exe_path: "
       << params.exe_path << " input_path: " << params.input_path << " output_path: " << params.output_path
       << " error_path: " << params.error_path << " log_path: " << params.log_path << " seccomp_rule_name: "
       << params.seccomp_rule_name;
    for(const std::string& item : params.argv)
        os << " args: " << item;
    for(const std::string& item : params.env)
        os << " env: " << item;
    return os;
}

std::ostream &operator<<(std::ostream &os, const run_result &result) {
    os << "cpu_time: " << result.cpu_time << " real_time: " << result.real_time << " memory: " << result.memory
       << " signal: " << result.signal << " exit_code: " << result.exit_code << " error: " << result.error
       << " result: " << result.result;
    return os;
}

#ifndef JUDGE_LANGUAGE
#define JUDGE_LANGUAGE

#include <vector>
#include <string>

std::vector<std::string> default_env = {"LANG=en_US.UTF-8", "LANGUAGE=en_US:en", "LC_ALL=en_US.UTF-8"};

namespace c_lang_config
{
    namespace compile
    {
        const std::string src_name = "Main.c";
        const std::string exe_name = "Main";
        const int max_cpu_time = 3000;
        const int max_real_time = 5000;
        const int max_memory = 512 * 1024 * 1024;
        const std::string compile_command = "/usr/bin/gcc -O2 -w -fmax-errors=5 -std=c99 %s -lm -o %s";
    }
    namespace run
    {
        const std::string command = "%s";
        const std::string seccomp_rule = "c_cpp";
        const std::vector<std::string> env = default_env;
        const int memory_limit_check_only = 0;
    }
}

namespace cpp_lang_config
{
    namespace compile
    {
        const std::string src_name = "Main.cpp";
        const std::string exe_name = "Main";
        const int max_cpu_time = 3000;
        const int max_real_time = 5000;
        const int max_memory = 512 * 1024 * 1024;
        const std::string compile_command = "/usr/bin/g++ -O2 -w -fmax-errors=5 -std=c++11 %s -lm -o %s";
    }
    namespace run
    {
        const std::string command = "%s";
        const std::string seccomp_rule = "c_cpp";
        const std::vector<std::string> env = default_env;
        const int memory_limit_check_only = 0;
    }
}

namespace java_lang_config
{
    namespace compile
    {
        const std::string src_name = "Main.java";
        const std::string exe_name = "Main";
        const int max_cpu_time = 3000;
        const int max_real_time = 5000;
        const int max_memory = -1;
        const std::string compile_command = "/usr/bin/javac %s -d %s -encoding UTF8";
    }
    namespace run
    {
        const std::string command = "/usr/bin/java -cp %s -XX:MaxRAM=%dk -Djava.security.manager -Dfile.encoding=UTF-8 -Djava.security.policy==/etc/java_policy -Djava.awt.headless=true %s";
        const std::string seccomp_rule = "";
        const std::vector<std::string> env = default_env;
        const int memory_limit_check_only = 1;
    }
}

#endif

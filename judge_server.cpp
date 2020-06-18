#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <dirent.h>

#include <iostream>
#include <string>
#include <vector>
#include <set>

#include "httplib.h"
#include "json.hpp"


#include "judge_option.h"
#include "compare.h"

#include "PropertiesReader.h"
#include "RedisConnector.h"

#define LOCKFILE "/var/run/judge.pid"
#define BUFFER_SIZE 1024


#define JUDGER_DEBUG

static volatile int process_count = 0;
static volatile bool stop = false;


static std::string judge_host;
static int judge_port;
static int max_running;
static std::string token;
static redisConf conf;
static std::string work_base_dir;
static std::string test_case_base_dir;
static std::string uploadUrl;
static std::string uploadPath;
static int  uploadPort;


int daemon_init()
{
    pid_t pid;
    if((pid = fork()) < 0)
        return -1;
    else if(pid != 0)
        exit(0);

    setsid();
    chdir("/");
    umask(0);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int fd = open("/dev/null", O_RDWR);
    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);
    if(fd > 2)
        close(fd);

    return 0;
}


int get_file_size(const std::string& filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

void addsig(int sig, void (*sig_handler)(int), bool restart = true)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_handler;
    if(restart)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    sigaction(sig, &sa, NULL);
}

int lockfile(int fd)
{
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return fcntl(fd, F_SETLK, &fl);
}

int is_running()
{
    char buf[16];
    const int mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
    int fd = open(LOCKFILE, O_WRONLY | O_CREAT, mode);
    if(fd < 0)
    {
        printf("can't open %s: %s.\n", LOCKFILE, strerror(errno));
        exit(-1);
    }
    if (lockfile(fd) < 0)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            close(fd);
            return 1;
        }
        printf("can't lock %s: %s.\n", LOCKFILE, strerror(errno));
        exit(-1);
    }
    ftruncate(fd, 0);
    lseek(fd, 0, SEEK_SET);
    sprintf(buf, "%d\n", getpid());
    write(fd, buf, strlen(buf) + 1);
    return 0;
}

int read_testcase(const std::string& path, std::vector<std::string>& vec)
{
    vec.clear();
    std::set<std::string> container;
    DIR* dirp = opendir(path.c_str());

    if(!dirp) return -1;

    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL)
    {
        std::string file = dp->d_name;
        struct stat s_buf;
        stat((path + "/" + file).c_str(), &s_buf);
        if(S_ISREG(s_buf.st_mode))
            container.insert(file);
    }
    closedir(dirp);
    for(const std::string& value : container)
    {
        if(value.rfind(".in") == value.size() - 3)
        {
            std::string test_id = value.substr(0, value.size() - 3);
            if(container.find(test_id + ".out") != container.end())
            {
                vec.push_back(test_id);
            }
        }
    }
    return 0;
}

int read_testcase_spj(const std::string& path, std::vector<std::string>& vec)
{
    vec.clear();
    std::set<std::string> container;
    DIR* dirp = opendir(path.c_str());

    if(!dirp) return -1;

    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL)
    {
        std::string file = dp->d_name;
        struct stat s_buf;
        stat((path + "/" + file).c_str(), &s_buf);
        if(S_ISREG(s_buf.st_mode))
            container.insert(file);
    }
    closedir(dirp);

    if(container.find("spj") == container.end()) return -1;

    for(const std::string& value : container)
    {
        if(value.rfind(".in") == value.size() - 3)
        {
            std::string test_id = value.substr(0, value.size() - 3);
            vec.push_back(test_id);
        }
    }
    return 0;
}

void init_conf(const std::string& conf_file)
{
    propertiesReader reader;
    reader.loadResource(!conf_file.empty() ? conf_file : "/etc/judge.conf");

    judge_host = reader.getProperty("judge_host");
    judge_port = atoi(reader.getProperty("judge_port").c_str());
    max_running = atoi(reader.getProperty("max_running").c_str());
    token = reader.getProperty("token");

    work_base_dir = reader.getProperty("work_base_dir");
    test_case_base_dir = reader.getProperty("test_case_base_dir");

    conf.host = reader.getProperty("redis_host");
    conf.port = atoi(reader.getProperty("redis_port").c_str());
    conf.problemChannel = reader.getProperty("problemChannel");
    conf.contestChannel = reader.getProperty("contestChannel");

    uploadUrl = reader.getProperty("uploadUrl");
    uploadPath = reader.getProperty("uploadPath");
    uploadPort = atoi(reader.getProperty("uploadPort").c_str());
}

int init_server()
{
    printf("judge listen on : %s %d.\n", judge_host.c_str(), judge_port);

    int udpfd = socket(PF_INET, SOCK_DGRAM, 0);
    if(udpfd < 0) exit(-1);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, judge_host.c_str(), &address.sin_family);
    address.sin_port = htons(judge_port);

    int ret = bind(udpfd, (struct sockaddr*)&address, sizeof(address));

    if(ret < 0)
    {
        printf("socket error :%s.\n", strerror(errno));
        exit(-1);
    }

    return udpfd;
}

void wait_request(int fd)
{
    char buf[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_addrlength = sizeof(client_addr);
    recvfrom(fd, buf, BUFFER_SIZE - 1, 0, (struct sockaddr*)&client_addr, &client_addrlength);
}

void notify_server()
{
    int notify_fd = socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, judge_host.c_str(), &address.sin_family);
    address.sin_port = htons(judge_port);
    sendto(notify_fd, "1", 1, 0, (sockaddr*)&address, sizeof(address));
}

void update_submission_result(const nlohmann::json& data)
{
#ifdef JUDGER_DEBUG
    std::cout << data << std::endl;
#endif
    std::ostringstream ss;
    ss << data;

    std::string body = ss.str();
    httplib::Client cli(uploadUrl, uploadPort);
    httplib::Headers headers = {{"secure_token", token}};
    auto res = cli.Post(uploadPath.c_str(), headers, body, "application/json");
    if(res && res->status == 200)
    {
        printf("update submission success!\n");
    }
    else if(res)
    {
        printf("request error!\n");
        printf("error : %d %s\n", res->status, res->body.c_str());
    }
    else
    {
        printf("internal error!\n");
    }
}

void run_client(struct runRequest& req)
{
    std::string work_dir = work_base_dir + "/" + std::to_string(req.submissionId);
    std::string test_case_dir;

    if(req.contestId == -1)
        test_case_dir = test_case_base_dir + "/problem/" + std::to_string(req.problemId);
    else
        test_case_dir = test_case_base_dir + "/contest/" + std::to_string(req.contestId) + "/problem/" + std::to_string(req.problemId);


    int ok = mk_workdir(work_dir);
    if(ok == -1)
    {
        printf("subprocess exit for mkdir error :%s.\n", strerror(errno));
        exit(-1);
    }

#ifdef JUDGER_DEBUG
    std::cout << req << std::endl;
    std::cout << "work_dir : "  << work_dir << std::endl;
    std::cout << "test_case_dir : " << test_case_dir << std::endl;
#endif

    {
        nlohmann::json data;
        data["runId"] = req.submissionId;
        data["result"] = (int)Judging;
        update_submission_result(data);
    }

    try
    {
        // 这里可以预先检查一下%lld, %I64d的合法性问题
        write_solution(req.language, work_dir, req.sourcecode);
        compile(req.language, work_dir, work_dir);
        std::vector<std::string> test_set;
        int success = !req.isSpj ? read_testcase(test_case_dir, test_set) : read_testcase_spj(test_case_dir, test_set);
        if(success != -1)
        {
            judge_result tmp = {-1, -1, Accepted};

            for(const std::string& value : test_set)
            {
                std::string test_case_id = test_case_dir + "/" + value;
                std::string output_file = work_dir + "/" + value;

#ifdef JUDGER_DEBUG
                std::cout << "test_cast_id : " << test_case_id << std::endl;
                std::cout << "output_file : " << output_file << std::endl;
#endif

                int test_case_size = get_file_size(test_case_id + ".out");

                judge_result result = run_solution(req.language, work_dir, test_case_id + ".in", output_file, req.timeLimit, req.memoryLimit, std::max(test_case_size * 2, 1024 * 1024 * 16));

                tmp.timeUsed = std::max(tmp.timeUsed, result.timeUsed);
                tmp.memoryUsed = std::max(tmp.memoryUsed, result.memoryUsed);

                if(result.result != -1)
                {
                    tmp.result = result.result;
                }
                else
                {
#ifdef JUDGER_DEBUG
                    std::cout << "output file : " << output_file << std::endl;
                    std::cout << "test_case_file : " << (test_case_id + ".out") << std::endl;
#endif
                    int ret, flag;
                    if(!req.isSpj)
                        ret = compare(output_file.c_str(), (test_case_id + ".out").c_str());
                    else
                        ret = compare_spj((test_case_dir + "/" + "spj").c_str(), (test_case_id + ".in").c_str(), (test_case_id + ".out").c_str(), output_file.c_str());

                    switch (ret)
                    {
                        case OJ_AC:
                            flag = Accepted;
                            break;
                        case OJ_PE:
                            flag = PresentationError;
                            break;
                        case OJ_WA:
                            flag = WrongAnswer;
                            break;
                        case OJ_RE:
                            flag = RuntimeError;
                            break;
                        case OJ_SPJ_ERR:
                            flag = SystemError;
                            break;
                        default:
                            flag = -1;
                    }

                    if(flag != Accepted)
                    {
                        tmp.result = flag;
                    }
                }

#ifdef JUDGER_DEBUG
                std::cout << "submission result : " << tmp.result << std::endl;
#endif

                if(tmp.result != Accepted)
                    break;
            }
            nlohmann::json data;
            data["runId"] = req.submissionId;
            data["timeUsed"] = tmp.timeUsed;
            data["memoryUsed"] = tmp.memoryUsed;
            data["result"] = tmp.result;
            update_submission_result(data);
        }
        else
        {
            nlohmann::json data;
            data["runId"] = req.submissionId;
            data["result"] = (int)SystemError;
            update_submission_result(data);
        }
    }
    catch (const std::exception& e)
    {
        printf("judge run error : \n %s\n", e.what());

        nlohmann::json data;
        data["runId"] = req.submissionId;
        data["result"] = (int)CompileError;
        data["ext"] = e.what();
        update_submission_result(data);
    }

    rm_workdir(work_dir);
    exit(0);
}

void work()
{
    int udpfd = init_server();
    int status;
    redisConnector redisConn = redisConnector(conf);

    while(!stop)
    {
        printf("wait request from client...\n");
        wait_request(udpfd);

        while(waitpid(-1, &status, WNOHANG) > 0)
            process_count--;

        if(stop) continue;

        std::vector<runRequest> requests;

        while(true)
        {
            redisConn.popFromContestProblem(requests, 20);
            redisConn.popFromProblem(requests, 10);
            if(requests.empty()) break;

            for(runRequest request : requests)
            {
                while(process_count >= max_running)
                {
                   if(waitpid(-1, &status, 0) != -1)
                       process_count--;
                }
                pid_t pid = fork();
                if(pid == 0)
                {
                    run_client(request);
                }
                else if(pid > 0)
                {
                    process_count++;
                }
            }

            requests.clear();
        }
    }
}

void sig_handle(int sig)
{
    switch (sig)
    {
        case SIGQUIT:
        case SIGINT:
        case SIGTERM:
            stop = true;
            printf("receive sig for exit, process exit.\n");
            notify_server();
            break;
        default:
            ;
    }
}

int main(int argc, char **argv)
{
    const char* conf_file = NULL;

    if(argc < 2)
    {
        printf("please specify the configuration file path.\n");
        exit(-1);
    }

    conf_file = argv[1];

    addsig(SIGQUIT, sig_handle);
    addsig(SIGINT, sig_handle);
    addsig(SIGTERM, sig_handle);
    addsig(SIGCHLD, sig_handle);


    init_conf(conf_file);

    if (is_running())
    {
        printf("already has one judged on it!\n");
        exit(-1);
    }

    work();

    return 0;
}
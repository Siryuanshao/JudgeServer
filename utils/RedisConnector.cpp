#include "RedisConnector.h"
#include "hiredis/hiredis.h"
#include "json.hpp"

using json = nlohmann::json;

redisConnector::redisConnector(const redisConf& conf)
{
    context = redisConnect(conf.host.c_str(), conf.port);
    if(context == nullptr)
    {
        printf("RedisConnection Error!\n");
    }
    else if(context->err)
    {
        printf("RedisConnection Error : %s!\n", context->errstr);
    }
    else
    {
        problemChannel = conf.problemChannel;
        contestChannel = conf.contestChannel;
        printf("RedisConnection Success!\n");
    }
}

redisConnector::~redisConnector()
{
    if(context != nullptr)
    {
        redisFree(context);
    }
}

int redisConnector::popFromProblem(std::vector<runRequest>& requests, int maxPop)
{
    int ret = 0;
    if(context == nullptr || context->err)
    {
        printf("RedisConnection not build!\n");
    }
    else
    {
        for(int i = 0; i < maxPop; i++)
        {
            redisReply* reply = nullptr;
            reply = (redisReply*) redisCommand(context, "LPOP %s", problemChannel.c_str());
            if(reply == nullptr)
            {
                redisFree(context);
                context = nullptr;
                printf("RedisConnection Error: Maybe redis server is down!\n");
                break;
            }
            else if(reply->len <= 0)
            {
                freeReplyObject(reply);
                break;
            }
            else
            {
                try
                {
                    json _runRequest = json::parse(reply->str);

                    runRequest runRequest {
                            _runRequest["submissionId"].get<int>(),
                            _runRequest["problemId"].get<int>(),
                            _runRequest["contestId"].get<int>(),
                            _runRequest["language"].get<int>(),
                            _runRequest["timeLimit"].get<int>(),
                            _runRequest["memoryLimit"].get<int>(),
                            _runRequest["isSpj"].get<int>(),
                            _runRequest["sourcecode"].get<std::string>()
                    };

                    freeReplyObject(reply);
                    requests.push_back(runRequest);
                    ret++;
                }
                catch (const std::exception& e)
                {
                    printf("json parse error : \n %s\n", e.what());
                }
            }
        }
    }
    return ret;
}

int redisConnector::popFromContestProblem(std::vector<runRequest>& requests, int maxPop)
{
    int ret = 0;
    if(context == nullptr || context->err)
    {
        printf("RedisConnection not build!\n");
    }
    else
    {
        for(int i = 0; i < maxPop; i++)
        {
            redisReply* reply = nullptr;
            reply = (redisReply*) redisCommand(context, "LPOP %s", contestChannel.c_str());
            if(reply == nullptr)
            {
                redisFree(context);
                context = nullptr;
                printf("RedisConnection Error: Maybe redis server is down!\n");
                break;
            }
            else if(reply->len <= 0)
            {
                freeReplyObject(reply);
                break;
            }
            else
            {
                try
                {
                    json _runRequest = json::parse(reply->str);

                    runRequest runRequest {
                            _runRequest["submissionId"].get<int>(),
                            _runRequest["problemId"].get<int>(),
                            _runRequest["contestId"].get<int>(),
                            _runRequest["language"].get<int>(),
                            _runRequest["timeLimit"].get<int>(),
                            _runRequest["memoryLimit"].get<int>(),
                            _runRequest["isSpj"].get<int>(),
                            _runRequest["sourcecode"].get<std::string>()
                    };

                    freeReplyObject(reply);
                    requests.push_back(runRequest);
                    ret++;
                }
                catch (const std::exception& e)
                {
                    printf("json parse error : \n %s\n", e.what());
                }
            }
        }
    }
    return ret;
}

std::ostream &operator<<(std::ostream &os, const runRequest &request) {
    os << "submissionId: " << request.submissionId << " problemId: " << request.problemId << " contestId: "
       << request.contestId << " language: " << request.language << " timeLimit: " << request.timeLimit
       << " memoryLimit: " << request.memoryLimit << " isSpj: " << request.isSpj << " sourcecode: "
       << request.sourcecode;
    return os;
}

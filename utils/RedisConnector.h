#ifndef REDIS_CONNECTOR
#define REDIS_CONNECTOR


#include <vector>
#include <memory>
#include <string>
#include <ostream>

struct redisContext;
struct redisReply;
struct redisConf
{
    std::string problemChannel;
    std::string contestChannel;
    std::string host;
    int  port;
};

struct runRequest
{
    int submissionId;
    int problemId;
    int contestId;
    int language;
    int timeLimit;
    int memoryLimit;
    int isSpj;
    std::string sourcecode;

    friend std::ostream &operator<<(std::ostream &os, const runRequest &request);
};

class redisConnector : public std::enable_shared_from_this<redisConnector>
{
public:
    explicit redisConnector(const redisConf& conf);
    int popFromContestProblem(std::vector<runRequest>& requests, int maxPop);
    int popFromProblem(std::vector<runRequest>& requests, int maxPop);
public:
    ~redisConnector();
private:
    redisContext* context;
    std::string problemChannel;
    std::string contestChannel;
};

#endif

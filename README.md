# JudgeServer

## need to be deploy in docker

### environment
ubuntu:16.04

### require
git gcc g++ openjdk-8-jdk cmake libseccomp-dev

### dependency
https://github.com/redis/hiredis.git
https://github.com/Siryuanshao/Judger.git --branch newnew

### setup
```
mkdir /log
touch /log/compile.log
touch /log/judge.log
cd JudgeServer
cp ./deploy/java_policy /etc/
cp ./deploy/judge.conf /etc/ (first edit conf like oj-redis, oj-backend, work_base_dir(like /dev/shm which need to be volume mounted)....)
mkdir build && cd build && cmake .. && make
nohup ./Judger /etc/judge.conf &
```

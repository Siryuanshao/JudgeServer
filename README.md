# JudgeServer

## need to be deploy in docker

### environment

```shell
docker pull ubuntu:16.04
```

### require

```shell
apt get update
apt get install -y git gcc g++ openjdk-8-jdk cmake libseccomp-dev
```

### dependency

```shell
(hiredis)
git clone https://github.com/redis/hiredis.git
cd hiredis && make && make install

(Judger)
git clone -b newnew  --depth 1 https://github.com/Siryuanshao/Judger.git
cd Judger && mkdir build && cd build && cmake .. && make && make install
```

### setup

```shell
git clone https://github.com/Siryuanshao/JudgeServer.git
mkdir /log
touch /log/compile.log
touch /log/judge.log
cd JudgeServer
cp ./deploy/java_policy /etc/
cp ./deploy/judge.conf /etc/ (首先编辑配置文件)
mkdir build && cd build && cmake .. && make
nohup ./Judger /etc/judge.conf &
```

### configuration file

```
vim /etc/judge.conf

## 判题服务监听的ip地址
judge_host=
## 判题服务监听的端口
judge_port=
## 最大同时判题数量
max_running=
## 密匙和服务器后端配置一致
token=
## 判题临时文件存放位置，建议挂载并放置于/dev/shm
work_base_dir=
## 判题数据存放的位置, 一般在docker宿主机上挂载到此目录，如果修改则需要和java_policy一起修改
test_case_base_dir=/test_case_base_dir
## redis的ip地址
redis_host=
## redis的运行端口
redis_port=
## redis普通题目的判题列表，需要和服务器后端一致
problemChannel=
## redis比赛题目的判题列表，需要和服务器后端一致
contestChannel=
## 服务器后端运行的ip地址
uploadUrl=
## 上传判题路径的路径
uploadPath=/api/admin/updateSubmission
## 服务器后端运行的端口
uploadPort=
```






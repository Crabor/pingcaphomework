# PingCAP homework

## 问题描述

Transaction 调度器

KV store 里有 1000 个 Key，只有 Put / Get / Delete 接口，KV store 是线程安全的。随机选择 10 个 key 做为事务的 key，Get 这 10 个 key 的 value, Sleep 100 毫秒，把其中 5 个 key 的 value 减 1，并把另外 5 个 key 的 value 加 1，Put 到 KV Store。当所有事务执行停止的时候，要保证所有 value 之和是 0。

实现一个调度器，在并发数不限的条件下，让每分钟执行最多的事务。需要完成可以运行的代码。

## 运行平台

CentOS 7.5 64bit 2vCPUs 4GB 40GB 华为云服务器

## 解决思路

假设store里面所有key初始相加总和为0，由于每个事务对这随机的10个key的总贡献为0，所以所有事务顺利执行完后，所有value之和定为0。所以问题变为：

1. 线程安全，保证并发条件下各事务对key的``“get->change->set”``操作的原子性。

1. 事务回滚，在事务执行阶段发生错误时（如被其他进程误kill），能够回滚到事务执行之初状态。

为了追求效率与高并发，这里使用redis左右kv存储引擎。

针对问题1，由于redis是单线程执行命令的，所以借助redis自带的INCR、DECR命令即可解决传统自增操作的线程安全问题。

针对问题2，可以通过实现undo log,

```bash
gcc main.c -o main -I /usr/local/include/hiredis -lhiredis -lpthread -std=c99
```
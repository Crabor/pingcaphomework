# PingCAP homework

## 问题描述

Transaction 调度器
KV store 里有 1000 个 Key，只有 Put / Get / Delete 接口，KV store 是线程安全的。
随机选择 10 个 key 做为事务的 key，Get 这 10 个 key 的 value, Sleep 100 毫秒，把其中 5 个 key 的 value 减 1，并把另外 5 个 key 的 value 加 1，Put 到 KV Store。
当所有事务执行停止的时候，要保证所有 value 之和是 0。
实现一个调度器，在并发数不限的条件下，让每分钟执行最多的事务。
需要完成可以运行的代码。

## 解决思路


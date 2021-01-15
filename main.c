#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <hiredis.h>
#include <pthread.h>

#define MAX_KEYNAME_LEN 5
#define MAX_HOSTNAME_LEN 15

char hostname[MAX_HOSTNAME_LEN + 1];
int port;

void ThreadTransaction(void *ptr)
{
    //delay
    sleep(1);

    //connect to server
    redisContext *conn;
    redisReply *reply;
    conn = redisConnect(hostname, port);
    if (conn == NULL || conn->err)
    {
        if (conn)
        {
            printf("Connection error: %s\n", conn->errstr);
            redisFree(conn);
        }
        else
        {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    //decr 5 keys
    for (int i = 0; i < 5; ++i)
    {
        //get random key
        char keyname[MAX_KEYNAME_LEN + 1];
        reply = redisCommand(conn, "RANDOMKEY");
        strcpy(keyname, reply->str);
        freeReplyObject(reply);

        //decrease key value
        reply = redisCommand(conn, "DECR %s", keyname);
        freeReplyObject(reply);
    }

    //incr 5 keys
    for (int i = 0; i < 5; ++i)
    {
        //get random key
        char keyname[MAX_KEYNAME_LEN + 1];
        reply = redisCommand(conn, "RANDOMKEY");
        strcpy(keyname, reply->str);
        freeReplyObject(reply);

        //increase key value
        reply = redisCommand(conn, "INCR %s", keyname);
        freeReplyObject(reply);
    }

    redisFree(conn);
}

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        printf("Usage: example {instance_ip_address} {instance_port} {thread_number}\n");
        exit(0);
    }

    //set hostname & port & threadnum
    strcpy(hostname, argv[1]);
    port = atoi(argv[2]);
    int threadnum = atoi(argv[3]);

    //connect to server
    redisContext *conn;
    redisReply *reply;
    conn = redisConnect(hostname, port);
    if (conn == NULL || conn->err)
    {
        if (conn)
        {
            printf("Connection error: %s\n", conn->errstr);
            redisFree(conn);
        }
        else
        {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    //set 1000 keys
    for (int i = 0; i < 1000; ++i)
    {
        reply = redisCommand(conn, "SET %d 0", i);
        freeReplyObject(reply);
    }

    //create many threads, one thread equal to one transaction
    pthread_t *tid = (pthread_t *)malloc(threadnum * sizeof(pthread_t));
    if (!tid)
    {
        printf("malloc tid failed!\n");
        exit(1);
    }
    for (int i = 0; i < threadnum; ++i)
    {
        if (pthread_create(&tid[i], NULL, ThreadTransaction, NULL) != 0)
        {
            printf("Create pthread %i error!\n",i);
            exit(1);
        }
    }

    //wait all threads
    for (int i = 0; i < threadnum; ++i)
    {
        pthread_join(tid[i], NULL);
    }

    //check if all keys' sum equal to 0
    long long sum = 0;
    for (int i = 0; i < 1000; ++i)
    {
        reply = redisCommand(conn, "GET %d", i);
        sum += reply->integer;
        freeReplyObject(reply);
    }
    if (sum == 0)
    {
        printf("Congratulation!\n");
    }
    else
    {
        printf("Oops!\n");
    }

    redisFree(conn);
    return 0;
}
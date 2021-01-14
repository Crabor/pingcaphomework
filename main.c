#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_KEYNAME_LEN 5
#define MAX_HOSTNAME_LEN 15

typedef struct SERVER_INFO{
    char hostname[MAX_HOSTNAME_LEN+1];
    int port;
}ServerInfo_t;

void ThreadTransaction(void *ptr){
    pthread_detach(pthread_self());

    //connect to server
    ServerInfo_t server;
    strcpy(server.hostname,((ServerInfo_t*)ptr)->hostname);
    server.port = atoi(((ServerInfo_t*)ptr)->port);
    redisContext *conn;
    redisReply *reply;
    struct timeval timeout = {1, 500000}; // 1.5 seconds
    conn = redisConnectWithTimeout(server.hostname, server.port, timeout);
    if (conn == NULL || conn->err){
        if (conn){
            printf("Connection error: %s\n", conn->errstr);
            redisFree(conn);
        }else{
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    //decr 5 keys
    for(int i=0;i<5;++i){
        //get random key
        char keyname[MAX_KEYNAME_LEN+1];
        reply = redisCommand(conn,"RANDOMKEY");
        printf("KEY: %s\n", reply->str);
        strcpy(keyname,reply->str);
        freeReplyObject(reply);

        //decrease key value
        reply = redisCommand(conn,"DECR %s",keyname);
        printf("DECR: %s\n", reply->str);
        freeReplyObject(reply);
    }

    //incr 5 keys
    for(int i=0;i<5;++i){
        //get random key
        char keyname[MAX_KEYNAME_LEN+1];
        reply = redisCommand(conn,"RANDOMKEY");
        printf("KEY: %s\n", reply->str);
        strcpy(keyname,reply->str);
        freeReplyObject(reply);

        //increase key value
        reply = redisCommand(conn,"INCR %s",keyname);
        printf("INCR: %s\n", reply->str);
        freeReplyObject(reply);
    }

    redisFree(conn);
    pthread_exit(0) ;
}

int main(int argc, char **argv){
    if (argc < 4){
        printf("Usage: example {instance_ip_address} {instance_port} {thread number}\n");
        exit(0);
    }
    //set hostname & port
    ServerInfo_t server;
    strcpy(server.hostname,argv[1]);
    server.port = atoi(argv[2]);
    int threadnum = atoi(argv[3]);
    //connect to server
    redisContext *conn;
    redisReply *reply;
    struct timeval timeout = {1, 500000}; // 1.5 seconds
    conn = redisConnectWithTimeout(server.hostname, server.port, timeout);
    if (conn == NULL || conn->err){
        if (conn){
            printf("Connection error: %s\n", conn->errstr);
            redisFree(conn);
        }else{
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }
    //set 1000 keys
    for(int i=0;i<10;++i){
        reply=redisCommand(conn,"SET %d 0",i);
        freeReplyObject(reply);
    }
    redisFree(conn);

    //create many threads, one thread equal one transaction
    pthread_t tid;
    for(int i=0;i<threadnum;++i){
        if(pthread_create(&tid,NULL,ThreadTransaction,&server) != 0){
            printf ("Create pthread error!/n");
            exit (1);
        }
    }
    sleep(2);

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <hiredis.h>
#include "make_log.h"
#include "redis_op.h"

#define REIDS_TEST_MODULE "test"
#define REIDS_TEST_PROC "redis_test"

int main(int argc, char *argv[])
{
    redisContext *redis_conn = NULL;
    int ret = 0;
    char buf[2048]={0};
    redis_conn = rop_connectdb_nopwd("127.0.0.1", "6379");
    if (redis_conn == NULL) {
        LOG(REIDS_TEST_MODULE, REIDS_TEST_PROC, "conn error");
        exit(1);
    }


    ret = rop_set_string(redis_conn, "gailun", "lol");
    if (ret == -1) {
        LOG(REIDS_TEST_MODULE, REIDS_TEST_PROC, "set %s %s error", "gailun", "lol");
        exit(1);
    }
    
    LOG(REIDS_TEST_MODULE, REIDS_TEST_PROC, "set %s %s succ", "gailun", "lol");
    ret=rop_get_string(redis_conn,"gailun",buf);
    printf("%s\n",buf);
    rop_disconnect(redis_conn);


	return 0;
}

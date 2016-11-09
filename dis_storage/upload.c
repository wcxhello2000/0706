#include "fcgi_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "fcgi_stdio.h"
#include "util_cgi.h"
//fdfs_client
#include "make_log.h"
#include <pthread.h>
#define FDFS_LOG_MODULE "test"
#define FDFS_LOG_PROC   "fdfs_test"
#define FILE_ID_LEN 4096
//redis
#include <hiredis.h>
#include "make_log.h"
#include "redis_op.h"
#define  REDIS_TEST_MODULE "test"
#define  REDIS_TEST_PROC   "redis_test"
#include <time.h>


int main ()
{
    char *file_buf = NULL;
    char boundary[256] = {0};
    char content_text[256] = {0};
    char filename[256] = {0};
    char fdfs_file_path[256] = {0};
    char fdfs_file_stat_buf[256] = {0};
    char fdfs_file_host_name[30] = {0};
    char fdfs_file_url[512] = {0};
    //char *redis_value_buf = NULL;
    //time_t now;;
    //char create_time[25];
    //char suffix[10];
    


    while (FCGI_Accept() >= 0) {
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;

        printf("Content-type: text/html\r\n"
                "\r\n");

        if (contentLength != NULL) {
            len = strtol(contentLength, NULL, 10);
        }
        else {
            len = 0;
        }

        if (len <= 0) {
            printf("No data from standard input\n");
        }
        else {
            int i, ch;
            char *begin = NULL;
            char *end = NULL;
            char *p, *q, *k;

            //==========> 开辟存放文件的 内存 <===========

            file_buf = malloc(len);
            if (file_buf == NULL) {
                printf("malloc error! file size is to big!!!!\n");
                return -1;
            }

            begin = file_buf;
            p = begin;

            for (i = 0; i < len; i++) {
                if ((ch = getchar()) < 0) {
                    printf("Error: Not enough bytes received on standard input<p>\n");
                    break;
                }
                //putchar(ch);
                *p = ch;
                p++;
            }

            //===========> 开始处理前端发送过来的post数据格式 <============
            //begin deal
            end = p;

            p = begin;

            //get boundary
            p = strstr(begin, "\r\n");
            if (p == NULL) {
                printf("wrong no boundary!\n");
                goto END;
            }

            strncpy(boundary, begin, p-begin);
            boundary[p-begin] = '\0';
            //printf("boundary: [%s]\n", boundary);

            p+=2;//\r\n
            //已经处理了p-begin的长度
            len -= (p-begin);

            //get content text head
            begin = p;

            p = strstr(begin, "\r\n");
            if(p == NULL) {
                printf("ERROR: get context text error, no filename?\n");
                goto END;
            }
            strncpy(content_text, begin, p-begin);
            content_text[p-begin] = '\0';
            //printf("content_text: [%s]\n", content_text);

            p+=2;//\r\n
            len -= (p-begin);

            //get filename
            // filename="123123.png"
            //           ↑
            q = begin;
            q = strstr(begin, "filename=");
            
            q+=strlen("filename=");
            q++;

            k = strchr(q, '"');
            strncpy(filename, q, k-q);
            filename[k-q] = '\0';

            trim_space(filename);
            //printf("filename: [%s]\n", filename);

            //get file
            begin = p;     
            p = strstr(begin, "\r\n");
            p+=4;//\r\n\r\n
            len -= (p-begin);

            begin = p;
            // now begin -->file's begin
            //find file's end
            p = memstr(begin, len, boundary);
            if (p == NULL) {
                p = end-2;    //\r\n
            }
            else {
                p = p -2;//\r\n
            }
        
            //begin---> file_len = (p-begin)
            int fd = 0;
            fd = open(filename, O_CREAT|O_WRONLY, 0644);
            if (fd < 0) {
                printf("open %s error\n", filename);

            }

            ftruncate(fd, (p-begin));
            write(fd, begin, (p-begin));
            close(fd);
//***********************************************************************
            //=> 将该文件存入fastDFS中,并得到文件的file_id <==
            //char *file_name=NULL;
            char file_id[FILE_ID_LEN]={0};
            int j=0;
            pid_t pid;
            int pfd[2];
            if(pipe(pfd)<0){
                LOG(FDFS_LOG_MODULE,FDFS_LOG_PROC,"pipe err");
                exit(1);
            }
            pid=fork();
            if(pid==0){
                //child
                close(pfd[0]);
                dup2(pfd[1],STDOUT_FILENO);
                execlp("fdfs_upload_file","fdfs_upload_file","./conf/client.conf",filename,NULL);
                
            }
            else{
                //parent
                close(pfd[1]);
                wait(NULL);
                //从管道读数据
                read(pfd[0],file_id,FILE_ID_LEN);
                j=strlen(file_id);
                file_id[j-1]='\0';
            }
            //================ > 得到文件所存放storage的host_name <=================
//*****************************************************************************************************************
//创建redis表，将数据上传文件的fileid、filename、createtime存放至redis表中
            redisContext *redis_conn=NULL;
            int ret=0;
            //char buf[2048]={0};
            redis_conn=rop_connectdb_nopwd("127.0.0.1","6379");
            if(redis_conn==NULL){
                LOG(REDIS_TEST_MODULE,REDIS_TEST_PROC,"conn err");
                exit(1);
            }
            //ret=rop_set_string(redis_conn,filename,file_id);
            //if(ret==-1){
            //    LOG(REDIS_TEST_MODULE,REDIS_TEST_PROC,"set %s %s error","gailun","lol");
            //}
            //建FILE_INFO_LIST表存放file_id
            char buf[1024]={0};
            sprintf(buf,"lpush FILE_INFO_LIST %s",file_id);
            ret=rop_redis_command(redis_conn, buf);
            if(ret==-1){
                LOG(REDIS_TEST_MODULE,REDIS_TEST_PROC,"lpush FILE_INFO_LIST %s error",file_id);
            }
            //建哈希表FILEID_NAME_HASH存放file_id和filename
            //int rop_hash_set(redisContext *conn, char *key, char *field, char *value);
            ret=rop_hash_set(redis_conn, "FILEID_NAME_HASH", file_id, filename);
            if(ret==-1){
                LOG(REDIS_TEST_MODULE,REDIS_TEST_PROC,"hset FILEID_NAME_HASH %s %serror",file_id,filename);
            }
            //建哈希表FILEID_TIME_HASH存放file_id和createtime
						char time_cur[1024]={0};
						time_t cur_t;
						time(&cur_t);
						strcpy(time_cur,asctime(gmtime(&cur_t)));
						ret=rop_hash_set(redis_conn, "FILEID_TIME_HASH", file_id, time_cur);
            if(ret==-1){
                LOG(REDIS_TEST_MODULE,REDIS_TEST_PROC,"hset FILEID_TIME_HASH %s %serror",file_id,time_cur);
            }
            //建哈希表 FILEID_URL_HASH存放file_id和文件路径（http://192.168.22.103/file_id）
            char file_path[1024]={0};
            sprintf(file_path,"http://192.168.22.103/%s",file_id);
            ret=rop_hash_set(redis_conn, "FILEID_URL_HASH", file_id, file_path);
            if(ret==-1){
                LOG(REDIS_TEST_MODULE,REDIS_TEST_PROC,"hset FILEID_TIME_HASH %s %serror",file_id,file_path);
            }
						//建哈希表  FILEID_USR_HASH存放file_id和文件所属用户名
						ret=rop_hash_set(redis_conn, "FILEID_USR_HASH", file_id, "group1");
            if(ret==-1){
                LOG(REDIS_TEST_MODULE,REDIS_TEST_PROC,"hset FILEID_TIME_HASH %s %serror",file_id,"group1");
            }
END:

            memset(boundary, 0, 256);
            memset(content_text, 0, 256);
            memset(filename, 0, 256);
            memset(fdfs_file_path, 0, 256);
            memset(fdfs_file_stat_buf, 0, 256);
            memset(fdfs_file_host_name, 0, 30);
            memset(fdfs_file_url, 0, 512);

            free(file_buf);
            //printf("date: %s\r\n", getenv("QUERY_STRING"));
        }
    } /* while */

    return 0;
}

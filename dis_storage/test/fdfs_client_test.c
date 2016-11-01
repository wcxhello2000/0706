#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/wait.h>

#include "make_log.h"

#define FDFS_LOG_MODULE       "test"
#define FDFS_LOG_PROC           "fdfs_test"

#define FILE_ID_LEN   4096

int main(int argc, char *argv[])
{
    char *file_name = NULL;
    char file_id[FILE_ID_LEN] = {0};
    int i=0;

    if (argc < 2) {
        printf("usage: ./a.out <filename path>\n");
        exit(1);
    }

    file_name = argv[1];

    

    pid_t pid;

    int pfd[2];//管道fd

    if (pipe(pfd) < 0) {
        LOG(FDFS_LOG_MODULE, FDFS_LOG_PROC, "pip error");
        exit(1);
    }

    pid = fork();
    if (pid == 0) {
        //child
        //关闭读端
        close(pfd[0]);
        //将stdout ---》pfd[1]
        dup2(pfd[1], STDOUT_FILENO);

        //exec
        execlp("fdfs_upload_file", "fdfs_upload_file", "./conf/client.conf", file_name, NULL);
        LOG(FDFS_LOG_MODULE, FDFS_LOG_PROC, "exec error");
    }
    else {
        //parent
        close(pfd[1]);

        wait(NULL);

        //从管道读数据
        read(pfd[0], file_id, FILE_ID_LEN);
        i=strlen(file_id);
        file_id[i-1]='\0';

        LOG(FDFS_LOG_MODULE, FDFS_LOG_PROC, "upload file_id[%s] succ!", file_id);
    }

    


	return 0;
}

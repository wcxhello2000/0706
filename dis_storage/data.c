#include <stdlib.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "fcgi_stdio.h"
//#include "fdfs_client.h"
#include "fcgi_stdio.h"
#include "fcgi_config.h"
#include "cJSON.h"
#include "util_cgi.h"
#include "redis_op.h"
#include "make_log.h"

#define DATA_LOG_MODULE     "cgi"
#define DATA_LOG_PROC       "data"
#define USER_NAME_LEN		256
//#define FILE_NAME_LEN				1024
#define REDIS_SERVER_IP     "127.0.0.1"
#define REDIS_SERVER_PORT   "6379"

void print_file_list_json1(int fromId, int count, char *cmd, char *username)
{
    int i=0;
    cJSON *root=NULL;
    cJSON *array=NULL;
    char *out;
    char filename[FILE_NAME_LEN]={0};
    char create_time[256]={0};
    char picurl[PIC_URL_LEN]={0};
    char suffix[8]={0};
    char pic_name[PIC_NAME_LEN]={0};
    char file_url[FILE_NAME_LEN]={0};
    char fileid_list[256]={0};
    //	char user[256]={0};
    int retn=0;
    int endId=fromId+count-1;
    //	int score=0;
    //	char shared_status[2]={0};

    strcpy(fileid_list,"FILE_INFO_LIST");
    RVALUES fileid_list_values = NULL;
    fileid_list_values = malloc(count*VALUES_ID_SIZE);
    int value_num;

    redisContext *redis_conn = NULL;
    redis_conn = rop_connectdb_nopwd(REDIS_SERVER_IP, REDIS_SERVER_PORT);
    if (redis_conn == NULL) {
        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "redis connected error");
        return;
    }
    LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "fromId:%d, count:%d",fromId, count);
    /*
       if (strcmp(cmd, "shareFile") == 0) {
       sprintf(fileid_list, "%s", "FILE_PUBLIC_LIST");
       }
       else if (strcmp(cmd, "newFile") == 0) {
       char user_id[10] = {0};
       rop_hash_get(redis_conn, USER_USERID_HASH, username, user_id);
       sprintf(fileid_list, "%s%s", FILE_USER_LIST, user_id);
       }
       */
    retn = rop_range_list(redis_conn, fileid_list, fromId, endId, fileid_list_values, &value_num);
    if (retn < 0) {
        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "redis range %s error", "FILE_PUBLIC_LIST");
        rop_disconnect(redis_conn);
        return;
    }  
    LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "value_num=%d\n", value_num);
    root = cJSON_CreateObject();
    array = cJSON_CreateArray();
    for(i=0;i<value_num;i++){
        //array[i]:
        cJSON* item=cJSON_CreateObject();
        //id
        cJSON_AddStringToObject(item, "id", fileid_list_values[i]);
        //kind
        cJSON_AddNumberToObject(item, "kind", 2);
        //title_m(filename)
        rop_hash_get(redis_conn, "FILEID_NAME_HASH", fileid_list_values[i], filename);
        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "filename=%s\n", filename);
        cJSON_AddStringToObject(item, "title_m", filename);

        //title_s(username)
        //rop_hash_get(redis_conn, FILEID_USER_HASH, fileid_list_values[i], user);
        cJSON_AddStringToObject(item, "title_s", username);

        //time
        rop_hash_get(redis_conn, "FILEID_TIME_HASH", fileid_list_values[i], create_time);
        cJSON_AddStringToObject(item, "descrip", create_time);
        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "create_time=%s\n", create_time);

        //picurl_m
        memset(picurl, 0, PIC_URL_LEN);
        strcat(picurl, "http://192.168.22.103");
        strcat(picurl, ":");
        strcat(picurl, "80");
        strcat(picurl, "/static/file_png/");

        get_file_suffix(filename, suffix);
        sprintf(pic_name, "%s.png", suffix);
        strcat(picurl, pic_name);
        cJSON_AddStringToObject(item, "picurl_m", picurl);
        //url
        rop_hash_get(redis_conn, "FILEID_URL_HASH", fileid_list_values[i], file_url);
        cJSON_AddStringToObject(item, "url", file_url);
        LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "file_url=%s\n", file_url);

        //pv
        //score = rop_zset_get_score(redis_conn, FILE_HOT_ZSET, fileid_list_values[i]);
        cJSON_AddNumberToObject(item, "pv", 1);

        //hot (ÎÄ¼þ¹²Ïí×´Ì¬)
        //rop_hash_get(redis_conn, FILEID_SHARED_STATUS_HASH, fileid_list_values[i], shared_status);
        cJSON_AddNumberToObject(item, "hot", 0);

        cJSON_AddItemToArray(array, item);

    }

    cJSON_AddItemToObject(root, "games", array);

    out = cJSON_Print(root);

    LOG(DATA_LOG_MODULE, DATA_LOG_PROC,"%s", out);
    printf("%s\n", out);

    free(fileid_list_values);
    free(out);

    rop_disconnect(redis_conn);
    return ;
}

int main()
{
    char fromId[5];
    char count[5];
    char cmd[20];
    char user[USER_NAME_LEN];
    char fileId[FILE_NAME_LEN];

    while(FCGI_Accept()>=0){
        char *query=getenv("QUERY_STRING");
        memset(fromId, 0, 5);
        memset(count, 0, 5);
        memset(cmd, 0, 20);
        memset(user, 0, USER_NAME_LEN);
        memset(fileId, 0, FILE_NAME_LEN);

        query_parse_key_value(query, "cmd", cmd, NULL);

        if(strcmp(cmd,"newFile")==0){
            query_parse_key_value(query, "fromId", fromId, NULL);
            query_parse_key_value(query, "count", count, NULL);
            query_parse_key_value(query, "user", user, NULL);
            LOG(DATA_LOG_MODULE, DATA_LOG_PROC, "=== fromId:%s, count:%s, cmd:%s, user:%s", fromId, count, cmd, user);
            cgi_init();	
            printf("Content-type: text/html\r\n");
            printf("\r\n");
            print_file_list_json1(atoi(fromId), atoi(count), cmd, user);
        }

    }




    return 0;	
}

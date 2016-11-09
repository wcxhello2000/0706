#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "cJSON.h"

/*
   {
        "name" :"zhang3",
        "age":18,
        "id":200,
        "girls":[
            "marong",
            "fengjie",
            "chunbro"
        ]
   } 
 
 */

void test_create_json(char *str_out)
{
    cJSON *root;//{}
    cJSON *girls_array;//[]

    root = cJSON_CreateObject();//{}

    //name = zhang3 --> {}
    cJSON_AddStringToObject(root, "name", "zhang3");
    //cJSON_AddItemToObject(root, "name", cJSON_CreateString("zhang3"));

    //age = 18 --> {}
    cJSON_AddNumberToObject(root, "age", 18);

    //id = 200 -->{}
    cJSON_AddNumberToObject(root, "id", 200);


    //[]  
    girls_array = cJSON_CreateArray();//[]

    //"marong" ==> []
    cJSON_AddItemToArray(girls_array, cJSON_CreateString("marong"));

    //"fengjie" ==> []
    cJSON_AddItemToArray(girls_array, cJSON_CreateString("fengjie"));

    //"chunbro" ==> []
    cJSON_AddItemToArray(girls_array, cJSON_CreateString("chunbro")); //["marong", "fengjie", "chunbro"]

    //[] －－－＞｛｝

    cJSON_AddItemToObject(root, "girls", girls_array);
    
    //root 
    char *out = NULL;

    out =cJSON_Print(root);
    cJSON_Delete(root);


    strcpy(str_out, out);

    free(out);
    
}

/*
   {
        "name" :"zhang3",
        "age":18,
        "id":200,
        "girls":[
            "marong",
            "fengjie",
            "chunbro"
        ]
   } 
 
 */
void test_parse_json(char *str)
{
    cJSON*root = NULL;

    //将json字符串 转换成 内存 cJSON格式中。
    root = cJSON_Parse(str);

    //name
    cJSON *name;
    name =cJSON_GetObjectItem(root, "name");

    printf("%s = %s\n", name->string, name->valuestring);

    //age
    cJSON* age;
    age = cJSON_GetObjectItem(root, "age");
    printf("%s = %d\n", age->string, age->valueint);

    //id
    cJSON* id;
    id = cJSON_GetObjectItem(root, "id");
    printf("%s = %d\n", id->string, id->valueint);


    //girls
    cJSON *girls_array;
    girls_array = cJSON_GetObjectItem(root, "girls");

    int num = cJSON_GetArraySize(girls_array);
    int i = 0;
    for (i = 0; i < num; i++) {
        cJSON *temp = cJSON_GetArrayItem(girls_array, i);
        printf("arra[%d]:%s\n",i, temp->valuestring);
    }


    cJSON_Delete(root);
}

int main(int argc, char *argv[])
{
    char str[4096] = {0};

    //创建一个json格式的数据
    test_create_json(str);

    printf("%s\n", str);

    
    printf("===================\n"); 
    //解析一个json格式的数据
    test_parse_json(str);

        return 0;
}

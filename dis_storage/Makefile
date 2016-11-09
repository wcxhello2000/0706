
CC=gcc
CPPFLAGS= -I./include -I/usr/local/include/hiredis/ -I/usr/local/include 
CFLAGS=-Wall 
LIBS= -lhiredis -lfcgi -lm

#找到当前目录下所有的.c文件
src = $(wildcard *.c)

#将当前目录下所有的.c  转换成.o给obj
obj = $(patsubst %.c, %.o, $(src))


test_fdfs_client = ./test/fdfs_client_test
main = main_test
redis= ./test/redis_test
upload= upload_test
echo= echo_test
json= ./test/cJSON_test
data= data_cgi


target=$(test_fdfs_client)  $(main) $(redis) $(upload) $(echo) $(json) $(data) 


ALL:$(target)


#生成所有的.o文件
$(obj):%.o:%.c
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) 


#fdfs_client_test程序
$(test_fdfs_client):./test/fdfs_client_test.o  make_log.o
	$(CC) $^ -o $@ $(LIBS)

#main程序
$(main):main.o make_log.o
	$(CC) $^ -o $@ $(LIBS)

#redis程序
$(redis):./test/redis_test.o make_log.o redis_op.o
	$(CC) $^ -o $@ $(LIBS)

#upload程序
$(upload):upload.o util_cgi.o make_log.o redis_op.o
	$(CC) $^ -o $@ $(LIBS)

#echo程序
$(echo):echo.o make_log.o
	$(CC) $^ -o $@ $(LIBS)

#cJSON_test程序
$(json):./test/cJSON_test.o cJSON.o
	$(CC) $^ -o $@ $(LIBS)

#data_cgi程序
$(data):data.o util_cgi.o  cJSON.o redis_op.o make_log.o
	$(CC) $^ -o $@ $(LIBS)

#clean指令

clean:
	-rm -rf $(obj) $(target) ./test/*.o

distclean:
	-rm -rf $(obj) $(target) ./test/*.o

#将clean目标 改成一个虚拟符号
.PHONY: clean ALL distclean

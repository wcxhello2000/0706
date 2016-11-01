#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "make_log.h"

int main(int argc, char *argv[])
{
    LOG("111", "222", "nihao");

	return 0;
}

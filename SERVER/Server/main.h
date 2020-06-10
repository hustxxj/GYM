#ifndef MAIN_H
#define MAIN_H

#include "IDRec.h"
#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>

static IDRec* idrec=new IDRec();
//存储每个文件描述符关联的客户端类型和编号
//vec[0][]表示PAD端
//vec[1][]表示闸机端
//vec[2][]表示前台端
static std::vector<std::vector<int>> vec(3,std::vector<int>(100,0));
static std::map<int,int> fd_door;
static pthread_mutex_t mutexs=PTHREAD_MUTEX_INITIALIZER;


#endif // MAIN_H

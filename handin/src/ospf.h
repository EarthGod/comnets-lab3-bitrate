#ifndef _OSPF_H
#define _OSPF_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#define NAME_LENGTH 20

typedef struct _MyList MyList;
struct _MyList{
	void* data;
	MyList* prev;
	MyList* next;
};

typedef struct _MyQueue {
	MyList* head;
	MyList* tail;
	unsigned int length;
} MyQueue;


typedef struct node_s {
	char name[NAME_LENGTH];
	MyList* neighbors; 
	int seq_num;
	int mark; // bfs
} node_t; 

typedef struct LSA_config_s {
	char *servers;
	char *LSAs;
	uint32_t num_servs;
} LSA_config_t;

typedef struct rt_s {
	node_t* clit;
	node_t* serv;
} rt_t;

void OSPF_init(char *servers, char *LSAs, int rr_flag);
char* route(char* clit_name, int rr_flag);

#endif
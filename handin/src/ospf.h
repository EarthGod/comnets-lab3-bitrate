#ifndef _OSPF_H
#define _OSPF_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>


#define NAME_LENGTH 20

typedef int (*MyCmpFunc) (const void* a, const void* b);
typedef void (*MyFunc) (void* data, void* func_data);

typedef struct _MyList MyList;
struct _MyList {
  void* data;
  MyList* prev;
  MyList* next;
};

MyList* mylist_append(MyList* list, void* data);
void* mylist_getdata(MyList* list, unsigned int n);
MyList* mylist_find(MyList* list, const void* data);
MyList* mylist_find_custom(MyList* list, const void* data, MyCmpFunc func);
unsigned int mylist_length(MyList *list);
void mylist_foreach(MyList* list, MyFunc func, void* func_data);
void mylist_free(MyList *list);


/* functions of MyList */

MyList* mylist_append(MyList* list, void* data) 
{
    MyList *new_list;
    MyList *last;

    new_list = mylist_new();
    new_list->data = data;
    new_list->next = NULL;

    if (list) {
        last = mylist_last(list);
        last->next = new_list;
        new_list->prev = last;

        return list;
    } else {
        new_list->prev = NULL;
        return new_list;
    }
}


void* mylist_getdata(MyList* list, unsigned int n) 
{
    while (n-- > 0 && list)
        list = list->next;

    return list ? list->data : NULL;
}

MyList* mylist_find(MyList* list, const void* data) 
{
    while (list) {
        if (list->data == data)
            break;
        list = list->next;
    }

    return list;
}

MyList* mylist_find_custom(MyList* list, const void* data, MyCmpFunc func) 
{
    if (func == NULL) return NULL;

    while (list) {
        if (!(*func)(list->data, data))
            return list;
        list = list->next;
    }
    return NULL;
}


unsigned int mylist_length(MyList *list) 
{
    unsigned int length = 0;
    while (list) {
        length++;
        list = list->next;
    }

    return length;
}

void mylist_foreach(MyList* list, MyFunc func, void* func_data) 
{
    while (list) {
        MyList *next = list->next;
        (*func)(list->data, func_data);
        list = next;
    }
}

void mylist_free(MyList *list) 
{
    if (list == NULL) {
        return;
    }
    mylist_free(list->next);
    free(list);
    return;
}

/* functions of MyList end */


typedef struct _MyQueue MyQueue;
struct _MyQueue {
  MyList* head;
  MyList* tail;
  unsigned int length;
};

MyQueue* myqueue_new(void);
bool myqueue_is_empty(MyQueue* queue);
void myqueue_push_tail(MyQueue* queue, void* data);
void* myqueue_pop_head(MyQueue *queue);

/* functions of Mqueue */
MyQueue* myqueue_new(void) 
{
    return (MyQueue*)calloc(1, sizeof(MyQueue));
}


bool myqueue_is_empty(MyQueue* queue) 
{
    return queue->head == NULL;
}



void myqueue_push_tail(MyQueue* queue, void* data) 
{
    if (queue == NULL) return;

    queue->tail = mylist_append(queue->tail, data);
    if (queue->tail->next)
        queue->tail = queue->tail->next;
    else
        queue->head = queue->tail;
    queue->length++;
}

void* myqueue_pop_head(MyQueue *queue) 
{
    if (queue == NULL) return NULL;

    if (queue->head) 
	{
        MyList* node = queue->head;
        void* data = node->data;

        queue->head = node->next;
        if (queue->head)
            queue->head->prev = NULL;
        else
            queue->tail = NULL;
        free(node);
        queue->length--;

        return data;
    }

    return NULL;
}

/* functions of Mqueue end */

extern MyList *servs; // data is a MyList* of nodes
extern MyList *clits; // data is a MyList* of nodes
extern MyList *nodes; // data is a node_t*
extern MyList *routing_table;  // data is a rt_t*

typedef struct LSA_config_s {
	char *servers;
	char *LSAs;
	uint32_t num_servs;
} LSA_config_t;

typedef struct node_s {
	char name[NAME_LENGTH];
	MyList* neighbors; 
	int seq_num;
	int mark; // bfs
} node_t; 

typedef struct rt_s {
	node_t* clit;
	node_t* serv;
} rt_t;



void OSPF_init(char *servers, char *LSAs, int rr_flag);
char* route(char* clit_name, int rr_flag);

#endif
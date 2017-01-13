#include "ospf.h"
#include <assert.h>
#include <string.h>

#define BUFSIZE 100
static int parse_servs(char *servers);
static int parse_LSA(char *LSAs);

//static void print_topo(void* data, void* func_data);
//static void print_table(void* data, void* func_data);

static void calc_sp(void* data, void* func_data);
static void unmark(void* data, void* func_data);


typedef int (*MyCmpFunc) (const void* a, const void* b);
typedef void (*MyFunc) (void* data, void* func_data);

MyList* mylist_new(void);
MyList* mylist_last(MyList *list); 
MyList* mylist_append(MyList* list, void* data);
void* mylist_getdata(MyList* list, unsigned int n);
MyList* mylist_find(MyList* list, const void* data);
MyList* mylist_find_custom(MyList* list, const void* data, MyCmpFunc func);
unsigned int mylist_length(MyList *list);
void mylist_foreach(MyList* list, MyFunc func, void* func_data);
void mylist_free(MyList *list);


/* functions of MyList */
MyList* mylist_new(void) 
{
    return (MyList*)calloc(1,sizeof(MyList));
}
MyList* mylist_last(MyList *list) 
{
    if (list) 
	{
        while (list->next)
            list = list->next;
    }

    return list;
}

MyList* mylist_append(MyList* list, void* data) 
{
	//fprintf(stderr, "entering append!\n");
    MyList *new_list;
    MyList *last;

    new_list = mylist_new();
    new_list->data = data;
    new_list->next = NULL;

    if (list) 
	{
		
        last = mylist_last(list);
        last->next = new_list;
        new_list->prev = last;
		//fprintf(stderr, "%d %s\n", list, list->data);
        return list;
    } 
	else 
	{
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
    while (list) 
	{
        if (list->data == data)
            break;
        list = list->next;
    }

    return list;
}

MyList* mylist_find_custom(MyList* list, const void* data, MyCmpFunc func) 
{
    if (func == NULL) return NULL;

    while (list) 
	{
        if (!(*func)(list->data, data))
            return list;
        list = list->next;
    }
    return NULL;
}


unsigned int mylist_length(MyList *list) 
{
    unsigned int length = 0;
    while (list) 
	{
        length++;
        list = list->next;
    }

    return length;
}


void mylist_foreach(MyList* list, MyFunc func, void* func_data) 
{
    while (list) 
	{
        MyList *next = list->next;
        (*func)(list->data, func_data);
        list = next;
    }
}

void mylist_free(MyList *list) 
{
    if (list == NULL) 
        return;
    mylist_free(list->next);
    free(list);
    return;
}

/* functions of MyList end */



MyQueue* myqueue_new(void);
bool myqueue_is_empty(MyQueue* queue);
void myqueue_push_tail(MyQueue* queue, void* data);
void* myqueue_pop_head(MyQueue *queue);

/* functions of MyQueue */
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




static MyList* add_new_node(const char* name);
static void add_new_neighbor(node_t* node, const MyList* neighbor);
static int find_node_by_name(const void* a, const void* b);
static int find_entry_by_name(const void* a, const void* b);
static int startsWith(char* base, char* str);

MyList *servs; // data is a MyList* of nodes
MyList *clits; // data is a MyList* of nodes
MyList *nodes; // data is a node_t*
MyList *routing_table;  // data is a rt_t*
uint32_t query_count;

void OSPF_init(char *servers, char *LSAs, int rr_flag) 
{
	parse_servs(servers);
	if (!rr_flag)
		parse_LSA(LSAs);
	else
		return;
	//mylist_foreach(nodes, print_topo, NULL);
	mylist_foreach(clits, calc_sp, NULL);
	//mylist_foreach(routing_table, print_table, NULL);
}


char* route(char* clit_name, int rr_flag) 
{	
	//fprintf(stderr,"entering route!\n");
	if (rr_flag) 
	{
		uint32_t size = mylist_length(nodes);
		//fprintf(stderr,"size = %d\n", size);
		node_t* node = (node_t*)mylist_getdata(nodes, query_count % size);
		query_count++;
		return node->name;
	}
	else 
	{
		MyList* list = mylist_find_custom(routing_table, clit_name, find_entry_by_name);
		rt_t* entry = (rt_t*)list->data;
		return entry->serv->name;
	}
	return NULL;	
}


static void print_table(void* data, void* func_data) 
{
	rt_t* table = (rt_t*)(data);
	fprintf(stderr, "%s  --->  %s\n", table->clit->name, table->serv->name);
	return;
}

static void print_topo(void* data, void* func_data) 
{
	node_t* node = (node_t*)data;
	fprintf(stderr, "%s : ", node->name);
	MyList* neighbor_iter = node->neighbors;
	while (neighbor_iter) 
	{
		node_t* tmp = (node_t*)((MyList*)neighbor_iter->data)->data;
		fprintf(stderr, "\t%s", tmp->name);
		neighbor_iter = neighbor_iter->next;
	}
	fprintf(stderr, "\n");
}

// a func that is for foreach
static void calc_sp(void* data, void* func_data) 
{
	node_t* clit = (node_t*)((MyList*)data)->data;
	node_t* this_node = NULL;
	mylist_foreach(nodes, unmark, NULL);

	MyQueue* q = myqueue_new();
	myqueue_push_tail(q, (void*)clit);

	rt_t* new_entry = (rt_t*)malloc(sizeof(rt_t));
	routing_table = mylist_append(routing_table, (void*)new_entry);
	new_entry->clit = clit;
	new_entry->serv = NULL;

	clit->mark = 1; // this node has reached
	while (!myqueue_is_empty(q)) 
	{
		this_node = (node_t*)myqueue_pop_head(q);
		MyList* nei_tmp = this_node->neighbors;
		while (nei_tmp) 
		{
			node_t* tmp = (node_t*)((MyList*)nei_tmp->data)->data;

			if (tmp->mark) 
			{	
				// this node has reached before
				nei_tmp = nei_tmp->next;
				continue;
			}
			if (mylist_find(servs, nei_tmp->data)) 
			{
				// a server find, search complete
				new_entry->serv = tmp;
				return;
			}
			myqueue_push_tail(q, (void*)tmp);
			tmp->mark = 1;
		}
	}
	// should never reach here
	assert(0);
}


static void unmark(void* data, void* func_data) 
{
	((node_t*)data)->mark = 0;
	return;
}


static int parse_servs(char *servers) 
{
	//fprintf(stderr, "parsing servs from:%s\n", servers);
	FILE *fserv;
	char name[BUFSIZE];
	MyList* tmp;
	
	fserv = fopen(servers, "r");
	if (fserv == NULL) return -1;

	while (fgets(name, BUFSIZE, fserv)) 
	{
		name[strlen(name) - 1] = '\0'; //remove ending '\n'
		tmp = add_new_node(name);
		servs = mylist_append(servs, (void*)tmp);
	}
	fclose(fserv);
	return 0;
}


static int parse_LSA(char *LSAs) 
{
	FILE *fLSA;
	char buf[BUFSIZE];
	MyList* tmp;
	node_t* tmp_node;

	char name[NAME_LENGTH];
	int seq_num;
	char neighbors[BUFSIZE];
	char* neighbor;


	fLSA = fopen(LSAs, "r");
	if (fLSA == NULL) return -1;

	while (fgets(buf, BUFSIZE, fLSA)) 
	{
		sscanf(buf, "%s %d %s", name, &seq_num, neighbors);

		tmp = add_new_node(name);
		tmp_node = (node_t*)tmp->data;

		if (!mylist_find(servs, (void*)tmp)) 
		{
			if (!startsWith(name, "router")) 
			{
				if (!mylist_find(clits, (void*)tmp)) 
					clits = mylist_append(clits, (void*)tmp);
			}
		}

		if (tmp_node->seq_num >= seq_num) continue; 
		tmp_node->seq_num = seq_num;

		mylist_free(tmp_node->neighbors);
		tmp_node->neighbors = NULL; 

		neighbor = strtok(neighbors, ",");
		while (neighbor) 
		{
			tmp = add_new_node(neighbor); 
			add_new_neighbor(tmp_node, tmp);
			neighbor = strtok(NULL, ",");
		}
	}
	fclose(fLSA);
	return 0;
}


static MyList* add_new_node(const char* name) 
{
	MyList* tmp;

	tmp = mylist_find_custom(nodes, name, find_node_by_name);
	if (tmp == NULL) 
	{
		node_t* new_node = (node_t*)malloc(sizeof(node_t));
		strcpy(new_node->name, name);
		new_node->neighbors = NULL;
		new_node->seq_num = -1;
		nodes = mylist_append(nodes, (void*)new_node);
		tmp = mylist_find_custom(nodes, name, find_node_by_name);
	}
	return tmp;
}

static void add_new_neighbor(node_t* node, const MyList* neighbor) 
{
	MyList* tmp;

	tmp = mylist_find(node->neighbors, (void*)neighbor);
	if (tmp == NULL) 
	{
		node->neighbors = mylist_append(node->neighbors, (void*)neighbor);
	}
	return;
}


static int find_node_by_name(const void* a, const void* b) 
{
	// a should be a node_t
	// b should be a char*
	const node_t* this_node = (const node_t*)a;
	const char* that_name = (const char*)b;
	return strcmp(this_node->name, that_name);
}

static int find_entry_by_name(const void* a, const void* b) 
{
	// a should be a rt_t*
	// b should be a char*
	const rt_t* entry = (rt_t*)a;
	const char* name = (char*)b;
	return strcmp(entry->clit->name, name);
}


static int startsWith(char* base, char* str) 
{
    int blen = strlen(base);
    int slen = strlen(str);
    return (blen >= slen) && (0 == strncmp(base, str, slen));
}

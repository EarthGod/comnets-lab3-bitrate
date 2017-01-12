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

static MList* add_new_node(const char* name);
static void add_new_neighbor(node_t* node, const MList* neighbor);
static int find_node_by_name(const void* a, const void* b);
static int find_entry_by_name(const void* a, const void* b);
static int startsWith(char* base, char* str);

MList *servs; // data is a MList* of nodes
MList *clits; // data is a MList* of nodes
MList *nodes; // data is a node_t*
MList *routing_table;  // data is a rt_t*
uint32_t query_count;

void OSPF_init(char *servers, char *LSAs, int rr_flag) 
{
	parse_servs(servers);
	if (!rr_flag)
		parse_LSA(LSAs);
	else
		return;

	mylist_foreach(clits, calc_sp, NULL);

}


char* route(char* clit_name, int rr_flag) 
{	
	if (rr_flag) 
	{
		uint32_t size = mylist_length(nodes);
		node_t* node = (node_t*)mylist_getdata(nodes, query_count % size);
		query_count++;
		return node->name;
	}
	else 
	{
		MList* list = mylist_find_custom(routing_table, clit_name, find_entry_by_name);
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
	MList* neighbor_iter = node->neighbors;
	while (neighbor_iter) 
	{
		node_t* tmp = (node_t*)((MList*)neighbor_iter->data)->data;
		fprintf(stderr, "\t%s", tmp->name);
		neighbor_iter = neighbor_iter->next;
	}
	fprintf(stderr, "\n");
}

// a func that is for foreach
static void calc_sp(void* data, void* func_data) 
{
	node_t* clit = (node_t*)((MList*)data)->data;
	node_t* this_node = NULL;
	mylist_foreach(nodes, unmark, NULL);

	MQueue* q = myqueue_new();
	myqueue_push_tail(q, (void*)clit);

	rt_t* new_entry = (rt_t*)malloc(sizeof(rt_t));
	routing_table = mylist_append(routing_table, (void*)new_entry);
	new_entry->clit = clit;
	new_entry->serv = NULL;

	clit->mark = 1; // this node has reached
	while (!myqueue_is_empty(q)) 
	{
		this_node = (node_t*)myqueue_pop_head(q);
		MList* nei_tmp = this_node->neighbors;
		while (nei_tmp) 
		{
			node_t* tmp = (node_t*)((MList*)nei_tmp->data)->data;

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
	FILE *fserv;
	char name[BUFSIZE];
	MList* tmp;
	
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
	MList* tmp;
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


static MList* add_new_node(const char* name) 
{
	MList* tmp;

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

static void add_new_neighbor(node_t* node, const MList* neighbor) 
{
	MList* tmp;

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

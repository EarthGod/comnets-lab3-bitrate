#include "xml_parser.h"

bit_t* parse_xml(char* xml_buf, int length) 
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	int size = 0;
	int bitrate = 0;
	bit_t* head = NULL;
	bit_t* tail = NULL;
	bit_t* temp = NULL;

	doc = xmlReadMemory(xml_buf, length, "noname.xml", NULL, 0);
	if (doc == NULL) 
	{
		// failed to parse xml file
		return NULL;
	}

	fprintf(stderr, "start parsing!\n");
	/* get root node */
	cur = xmlDocGetRootElement(doc);
	cur = cur -> xmlChildrenNode;
	while (cur != NULL) 
	{
		if ((!xmlStrcmp(cur->name,(const xmlChar*) "media"))) 
		{
			bitrate = getBitrate(doc,cur);
			if (bitrate == -1) 
			{
				cur = cur->next;
				continue;
			}

			if(head == NULL) 
			{
				head = (bit_t*)malloc(sizeof(bit_t));
				head->bitrate = bitrate;
				head->next = NULL;
				tail = head;
			} 
			else 
			{
				temp = (bit_t*)malloc(sizeof(bit_t));
				temp->bitrate = bitrate;
				temp->next = NULL;
				tail->next = temp;
				tail = temp;
			}
			size++;
		}
		cur = cur->next;
	}
	if (size == 0)
		return NULL;
	else
		return head;
}

int getBitrate(xmlDocPtr doc, xmlNodePtr cur) 
{
	xmlChar* birate;
	int bit = 0;
	birate = xmlGetProp(cur,"bitrate");
	if (birate == NULL)
		return -1;
	bit = atoi((char*)birate);
	//fprintf(stderr, "bitrate:%d\n",bit);
	xmlFree(birate);
	return bit;
}
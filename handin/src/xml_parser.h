#ifndef _XML_PARSER_H
#define _XML_PARSER_H

#include <libxml/parser.h>
#include <libxml/xmlIO.h>
#include <libxml/xinclude.h>

typedef struct bit_s 
{
	int bitrate;
	struct bit_s* next;
}bit_t;

//bit_t* parse_xml(char* filename);
bit_t* parse_xml(char* xml_buf, int length);
int getBitrate(xmlDocPtr doc, xmlNodePtr cur);

#endif
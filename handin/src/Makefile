
CFLAGS = -Wall -g -Ilibxml2 -I/usr/local/include/libxml2/
LDFLAGS = -Llibs -lxml2 -lm
CC = gcc

objects = media.o connection.o xml_parser.o timer.o log.o pool.o sock.o mydns.o proxy.o 
.PHONY: default clean clobber handin



default: proxy

mydns: mydns.o
		$(CC) -o $@ $^ $(LDFLAGS)

nameserver: nameserver.o mydns.o ospf.o
	    $(CC) -o $@ $^ $(LDFLAGS) 

dnstest: dnstest.o mydns.o
		$(CC) -o $@ $^ $(LDFLAGS)

ospf: ospf.o
		$(CC) -o $@ $^ $(LDFLAGS) 
		
proxy: $(objects)
		$(CC) -o $@ $^ $(LDFLAGS)		
		


xml_parser.o: xml_parser.c xml_parser.h
timer.o: timer.c timer.h
log.o: log.c log.h
pool.o: pool.c pool.h
connection.o: connection.c connection.h
sock.o: sock.c sock.h
media.o: media.c media.h
proxy.o: proxy.c pool.h mydns.h debug.h sock.h media.h connection.h timer.h log.h xml_parser.h
mydns.o: mydns.c mydns.h debug.h
nameserver.o: nameserver.c nameserver.h mydns.h debug.h ospf.h
ospf.o: ospf.c ospf.h

dnstest.o: dnstest.c mydns.h

%.o: %.ce
		$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f *.o

clobber: clean
	rm -f proxy mydns nameserver dnstest ospf
	
handin:
	make clean; cd ..; tar cvf handin.tar handin --exclude test

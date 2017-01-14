// modified from ics proxy lab  

#include "sock.h"



ssize_t io_sendn(int fd, const char *ubuf, size_t n) 
{
	size_t nleft = n;
	ssize_t nsend;
	const char *buf = ubuf;

	while (nleft > 0) 
	{
		if ((nsend = send(fd, buf, nleft, 0)) <= 0) 
		{
			if (errno == EINTR)  // interrupted by sig handler return
				nsend = 0;    // and call send() again
			else if (errno == EPIPE) 
			{
				DEBUGPRINT("EPIPE handled\n");
				return -1;
			} 
			else if (errno == EAGAIN) 
			{
				nsend = 0;
			} 
			else 
			{
				/* errorno set by send() */
				DEBUGPRINT("send error on %s\n", strerror(errno));
				return -1;
			}
		}
		nleft -= nsend;
		buf += nsend;
	}
	return n;
}

ssize_t io_recvn(int fd, char *buf, size_t n) 
{
	size_t res = 0;
	int nread;
	size_t nleft = n;

	while (nleft > 0 && (nread = recv(fd, buf + res, nleft, 0)) > 0) 
	{
		nleft -= nread;
		res += nread;
	}
	if (nread == -1) 
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			DEBUGPRINT("read entire buffer\n");
			DEBUGPRINT("read:%d\n",res );
			return res;
		}
		else 
		{
			DEBUGPRINT("recv error on %s\n", strerror(errno));
			return -1;
		}
	}
	if (nread == 0) 
	{
		DEBUGPRINT("recv error on 0\n");
		return -1;
	}

	return res;
}

ssize_t io_recvn_block(int fd, char *buf, int n) 
{
	size_t res = 0;
	int nread;
	size_t nleft = n;

	while (1) 
	{
		nread = recv(fd, buf + res, nleft, 0);
		if (nread == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			continue;
		else if (nread == -1) 
		{
			DEBUGPRINT("BLock recv error on %s\n", strerror(errno));
			return -1;
		}
		res += nread;
		if (res == n)
			return res;
	}
}


ssize_t io_recvlineb(int fd, void *usrbuf, size_t maxlen) 
{
	int n, rc;
	char c, *bufp = usrbuf;

	for (n = 1; n < maxlen; n++) 
	{ 
		if ((rc = recv(fd, &c, 1, 0)) == 1) 
		{
			*bufp++ = c;
			if (c == '\n')
			break;
		} 
		else if (rc == 0) 
		{
			DEBUGPRINT("recv hdr = 0!\n");	
			if (n == 1)
				return 0; /* EOF, no data read */
			else
				return n; /* EOF, some data was read */
		} 
		else 
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN) 
			{
				DEBUGPRINT("read entire buffer once");
				break;
			}
			/* error */
			DEBUGPRINT("recv error on %s\n", strerror(errno));
			return -1;
		}
	}
	*bufp = 0;
	return n;
}


ssize_t io_recvline_block(int fd, void *usrbuf, size_t maxlen) 
{
	int n, rc;
	char c, *bufp = usrbuf;

	for (n = 1; n < maxlen; n++) 
	{ 
		if ((rc = recv(fd, &c, 1, 0)) == 1) 
		{
			*bufp++ = c;
			if (c == '\n')
			break;
		} 
		else if (rc == 0) 
		{
			DEBUGPRINT("recv hdr = 0!\n");	
			if (n == 1) 
			{
				DEBUGPRINT("recv() return 0, with no data read\n");
				return 0; /* EOF, no data read */
			}
			else
			{
				DEBUGPRINT("recv() return 0, with some data read\n");
				return n; /* EOF, some data was read */
			}
		} 
		else 
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN) 
			{
				DEBUGPRINT("read entire buffer once\n");
				continue;
			}
			/* error */
			DEBUGPRINT("recv error on %s\n", strerror(errno));
			return -1;
		}
	}
	*bufp = 0;
	return n;
}

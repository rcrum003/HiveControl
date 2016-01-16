/*
 * This file is part of libphidget21
 *
 * Copyright 2006-2015 Phidgets Inc <patrick@phidgets.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see 
 * <http://www.gnu.org/licenses/>
 */

#include "../stdafx.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <assert.h>
#if !defined(_WINDOWS) || defined(__CYGWIN__)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
//#include <arpa/inet.h>
#include <sys/poll.h>
#else
#include "wincompat.h"
#endif
#if defined(__CYGWIN__) || defined(__MINGW32_VERSION)
#include "getaddrinfo.h"
#endif
#ifndef _MSC_EXTENSIONS
#include <unistd.h>
#endif
#include "utils.h"

#if defined(_MACOSX) || defined(_LINUX)
// Use IPv6 on Mac OS and Linux
#define _ENABLE_IPv6
#endif

/* character escaping */
#define PASSTHRU(c) (isalnum((unsigned char)c)/* || c == ' '*/ || c == '.' || c == '/' || c == ':' || c == '%')

#ifndef _WINDOWS
#define SET_ERRDESC(errdesc, errdesclen) (errdesc ? snprintf(errdesc, \
	errdesclen, "%s", strerror(errno)) : 0);
#else
#define SET_ERRDESC(errdesc, errdesclen) wsa_set_errdesc(errdesc, errdesclen);
#endif

/* size of first buffer malloc; start small to exercise grow routines */
#define	FIRSTSIZE	128

int logging_enabled = FALSE;
#ifndef USE_PHIDGET21_LOGGING
static FILE *log_stream = 0;
static const char *log_pname = 0;
#endif

#if defined(_WINDOWS) && !defined(_CYGWIN)
static void
wsa_set_errdesc(char *errdesc, int len)
{
	int err;

	if (!errdesc)
		return;
	switch (err = WSAGetLastError()) {
		case 10038:
			snprintf(errdesc, len, "Socket operation on non-socket");
			return;
		case 10061:
			snprintf(errdesc, len, "Connection refused");
			return;
		default:
			snprintf(errdesc, len, "WSA error %d", err);
			return;
	}
}
#endif


/* Modified to work properly on ALL systems, not just UNIX */
int
pvasprintf(char **ret, const char *fmt, va_list ap)
{
	char *buf = NULL;
	size_t bufsize = 0;
	char *newbuf = NULL;
	size_t nextsize = 0;
	int outsize = 0;

	bufsize = 0;
	for (;;) {
		if (bufsize == 0) {
			if ((buf = (char *)malloc(FIRSTSIZE)) == NULL) {
				*ret = NULL;
				return -1;
			}
			bufsize = 1;
		} else if ((newbuf = (char *)realloc(buf, nextsize)) != NULL) {
			buf = newbuf;
			bufsize = nextsize;
		} else {
			free(buf); buf = NULL;
			*ret = NULL;
			return -1;
		}

#ifdef _WINDOWS
		outsize = _vsnprintf(buf, bufsize, fmt, ap);
#else
		{
			va_list copy;
			va_copy(copy, ap);
			outsize = vsnprintf(buf, bufsize, fmt, copy);
			va_end(copy);
		}
#endif

		if (outsize == -1) {
			/* Clear indication that output was truncated, but no
			* clear indication of how big buffer needs to be, so
			* simply double existing buffer size for next time.
			*/
			nextsize = (int)bufsize * 2;

		} else if (outsize == (int)bufsize) {
			/* Output was truncated (since at least the \0 could
			* not fit), but no indication of how big the buffer
			* needs to be, so just double existing buffer size
			* for next time.
			*/
			nextsize = (int)bufsize * 2;

		} else if (outsize > (int)bufsize) {
			/* Output was truncated, but we were told exactly how
			* big the buffer needs to be next time. Add two chars
			* to the returned size. One for the \0, and one to
			* prevent ambiguity in the next case below.
			*/
			nextsize = outsize + 2;

		} else if (outsize == (int)bufsize - 1) {
			/* This is ambiguous. May mean that the output string
			* exactly fits, but on some systems the output string
			* may have been trucated. We can't tell.
			* Just double the buffer size for next time.
			*/
			nextsize = (int)bufsize * 2;

		} else {
			/* Output was not truncated */
			break;
		}
	}
	*ret = buf;
	return (int)strlen(buf);
}

int
pasprintf(char **ret, const char *fmt, ...)
{
	va_list args;
	int outsize = 0;

	va_start(args, fmt);
	outsize = pvasprintf(ret, fmt, args);
	va_end(args);

	return outsize;
}

int
pd_getline(char *buf, unsigned int bufsize, int *bufcur,
		   int *buflen, int(*readfunc)(int, void *, unsigned int, char *,
		   int), int(*closefunc)(int, char *, int), int readfd, char **line,
		   char *errdesc, int errlen)
{
	char *eol;
	unsigned int srcsize;
	int linelen;
	int linesize = 1024;

	linelen = 0;
	if (!(*line = malloc(linesize))) {
		/* no memory */
		if (closefunc)
			(void) closefunc(readfd, errdesc, errlen);
		return 0;
	}
	**line = 0;

top:
	/* this loops until buf is empty, or we hit the end of a line */
	while (*bufcur < *buflen) {

		/* looking for eol - \n or \0 
		* eol points to the line ending \0, or NULL if line continues.
		* srcsize set to number of chars to copy into line. */
		if (!(eol = strchr(buf + *bufcur, '\n')))
		{
			if (!(eol = memchr(buf + *bufcur, '\0', *buflen - *bufcur)))
			{
				/* couldn't find '\n' or '\0' */
				srcsize = *buflen - *bufcur; 
			}
			else
			{
				/* found '\0' */
				srcsize = eol - buf - *bufcur + 1;
			}
		}
		else
		{ 
			/* found '\n' */
			srcsize = eol - buf - *bufcur + 1;
			/* change \n into \0 */
			*eol = 0;
		}

		/* double size of *line until it's big enough for srcsize characters */
		while ((int)srcsize > (linesize - linelen))
		{
			char *newline;

			linesize *= 2;

			if (!(newline = realloc(*line, linesize))) {
				/* not enough memory */
				free(*line);
				*line = NULL;
				if (closefunc)
					(void) closefunc(readfd, errdesc, errlen);
				return 0;
			}

			*line = newline;
		}

		/* copy buf into line and modify pointers */
		memcpy(*line + linelen, buf + *bufcur, srcsize);
		linelen += srcsize;
		*bufcur += srcsize;

		/* Are we at the end of the string? */
		if (eol)
		{
			/* change the first \r into a \0
			* This handles when they send \r\n but not \n\r */
			if ((eol = strchr(*line, '\r')) != NULL)
				*eol = 0;

			/* This is the (only) happy return from this function */
			return 1;
		}
	} //while (*bufcur < *buflen)

	/* we get here when there wasn't the end of a string in the buffer */
	*bufcur = 0;
	/* read in some data */
	if ((*buflen = readfunc(readfd, buf, bufsize - 1, errdesc, errlen)) <= 0)
	{
		/* read either errored, or the socket is closed */
		free(*line);
		*line = NULL;
		if (closefunc)
			(void) closefunc(readfd, errdesc, errlen);
		return 0;
	}
	/* null terminate - so strchr knows when to stop */
	buf[*buflen] = 0;
	/* go process it */
	goto top;
}

/*int
pd_getline_simple(int fd, char **line)
{
int bufcur = 0;
int buflen = 0;
char buf[2];

return pd_getline(buf, sizeof (buf), &bufcur, &buflen, pu_read, 0, fd,
line, 0, 0);
}*/

int
getmatchsub(const char *line, char **subp, const regmatch_t pmatch[], int n)
{
	int len;

	len = (pmatch[n].rm_so >= 0) ? pmatch[n].rm_eo - pmatch[n].rm_so : 0;
	if (subp) {
		if (len) {
			if (!(*subp = malloc(len + 1)))
				return len;
			memcpy(*subp, line + pmatch[n].rm_so, len);
			subp[0][len] = 0;
		} else {
			*subp = NULL;
		}
	}

	return len;
}

static int waitForConnect(int s, int cancel)
{
	int 		err;
	fd_set 		readFDs;
	fd_set 		writeFDs;
	fd_set 		errorFDs;
	int			selectResult;
	int			maxFD = s;

	FD_ZERO(&readFDs);
	FD_ZERO(&writeFDs);
	FD_ZERO(&errorFDs);

	// We always want the cancel socket in the read FD set.
#ifndef _WINDOWS
	if (cancel >= 0) 
#else
	if (cancel != INVALID_SOCKET) 
#endif
		FD_SET(cancel, &readFDs);
	FD_SET(s, &readFDs);
	FD_SET(s, &writeFDs);
	FD_SET(s, &errorFDs);
	
#ifndef _WINDOWS
	if (cancel >= 0 && cancel > s) 
#else
	if (cancel != INVALID_SOCKET && cancel > s) 
#endif
		maxFD = cancel;

	// Do the select, looping while we get EINTR errors.
	err = 0;
	do {
#ifndef _WINDOWS
		selectResult = select(maxFD + 1, &readFDs, &writeFDs, NULL, NULL);
#else
		//Windows reports a failed connect in errorFDs
		selectResult = select(maxFD + 1, &readFDs, &writeFDs, &errorFDs, NULL);
#endif

		if (selectResult < 0) {
#ifndef _WINDOWS
			err = errno;
#else
			err = WSAGetLastError();
#endif
		}
	} while (err == EINTR);

	if (err == 0) {
		// We have an infinite timeout, so a result of 0 should be impossible, 
		// so assert that selectResult is positive.
		assert(selectResult > 0);

		// Check for cancellation first.
#ifndef _WINDOWS
		if (cancel >= 0) 
#else
		if (cancel != INVALID_SOCKET) 
#endif
		{
			if ( FD_ISSET(cancel, &readFDs) ) {
				err = ECANCELED;
			}
		}
	}

	return err;
}

int cancelConnect(int cancelSocket)
{
	int	err;
	ssize_t bytesWritten;
	static const char kCancelMessage = 0;

	err = 0;

#ifndef _WINDOWS
	bytesWritten = write(cancelSocket, &kCancelMessage, sizeof(kCancelMessage));
#else
	bytesWritten = send(cancelSocket, &kCancelMessage, sizeof(kCancelMessage), MSG_NOSIGNAL);
#endif
	if (bytesWritten < 0) {
		err = errno;
	} 
	return err;
}

int CCONV
stream_server_connect(const char *dest, const char *svcname, int *fdp, int *cancelSocket, char *errdesc, int errdesclen)
{
	struct addrinfo hints, *res, *res0 = NULL;
	int err, cancelled = 0;
	SOCKET s = INVALID_SOCKET;
	int tmpSock[2];
	int cancelRecvSocket = INVALID_SOCKET;
#ifdef _WINDOWS
	u_long nNonBlocking = TRUE;
#endif
#ifdef _MACOSX
	int opt;
#endif

	memset(&hints, 0, sizeof(hints));
	//hints.ai_family = AF_INET;
	hints.ai_family = PF_UNSPEC; //any family (including IPV6)
	hints.ai_socktype = SOCK_STREAM;
#ifndef WINCE
	hints.ai_flags = AI_ADDRCONFIG;
#endif

	if ((err = getaddrinfo(dest, svcname, &hints, &res0)) != 0) {
		if (errdesc)
			snprintf(errdesc, errdesclen, "getaddrinfo: %s", gai_strerror(err));
		freeaddrinfo(res0);
		return 0;
	}

	//setup cancel socket
	err = socketpair(AF_UNIX, SOCK_STREAM, 0, tmpSock);
	if (err < 0) {
		SET_ERRDESC(errdesc, errdesclen);
		*cancelSocket = INVALID_SOCKET;
		pu_log(PUL_WARN, 0, "Unable to create a cancel socket: %s",errdesc);
		return 0;
	} else {
		*cancelSocket = tmpSock[0];
		cancelRecvSocket = tmpSock[1];
	}

	for (res = res0; res; res = res->ai_next) {
		if(cancelled)
			break;

		// sometimes on mac, we get 0.0.0.0 ??!! - just ignore it
		// I think this is IPv6 - don't ignore...
		//if(((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr == 0) 
		//{
		//	continue;
		//}

		s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
#ifndef _WINDOWS
		if (s < 0) 
#else
		if (s == INVALID_SOCKET) 
#endif
		{
			SET_ERRDESC(errdesc, errdesclen);
			continue;
		}

		//Make the socket non-blocking
#ifndef _WINDOWS
		if (fcntl(s, F_SETFL, O_NONBLOCK) < 0) {
			SET_ERRDESC(errdesc, errdesclen);
			s = INVALID_SOCKET;	
			continue;
		}
#else
		if (ioctlsocket(s, FIONBIO, (u_long FAR *) & nNonBlocking))
		{
			s = INVALID_SOCKET;	
			continue;
		}
#endif

		if((err = connect(s, res->ai_addr, (int) res->ai_addrlen)) != 0)
		{
			//Connection in progress - wait for completion or cancelation
#ifndef _WINDOWS
			if(errno == EINPROGRESS)
#else
			err = WSAGetLastError();
			if(err == WSAEINPROGRESS || err == WSAEWOULDBLOCK)
#endif
			{
				err = waitForConnect(s, cancelRecvSocket);

				// Not cancelled, so must have either connected or failed.  
				// Check to see if we're connected by calling getpeername.
				if (err == 0) {
					socklen_t 	len = sizeof(struct sockaddr);
					struct sockaddr name;
					err = getpeername(s, &name, &len);

					// The connection failed.  Get the error associated with 
					// the connection attempt.
					if (err < 0) {
						char tmpErr;
#ifdef _WINDOWS
						err = WSAGetLastError();
#endif
						len = sizeof(tmpErr);
						err = getsockopt(s, SOL_SOCKET, SO_ERROR, &tmpErr, &len);
						if (err < 0) {
#ifdef _WINDOWS
							err = WSAGetLastError();
#else
							err = errno;
#endif
						} else {
							assert(tmpErr != 0);
							err = tmpErr;
						}
					}
					//connection good
					else
						break;
				}
				//cancelled
				else if(err==ECANCELED)
				{
					cancelled = 1;
				}

#ifdef _WINDOWS
				WSASetLastError(err);
#else
				errno = err;
#endif
			}
			//Error
			SET_ERRDESC(errdesc, errdesclen);
#ifndef _WINDOWS
			close(s);
#else
			closesocket(s);
#endif
			s = INVALID_SOCKET;	
			continue;
		}
		break;
	}

	//cleanup cancel socket - not needed anymore
	if (tmpSock[0] != -1)
#ifndef _WINDOWS
		close(tmpSock[0]);
#else
		closesocket(tmpSock[0]);
#endif
	if (tmpSock[1] != -1)
#ifndef _WINDOWS
		close(tmpSock[1]);
#else
		closesocket(tmpSock[1]);
#endif
	*cancelSocket = INVALID_SOCKET;

#ifndef _WINDOWS
	if (s < 0)
#else
	if (s == INVALID_SOCKET)
#endif
	{
		SET_ERRDESC(errdesc, errdesclen);
		freeaddrinfo(res0);
		return 0;
	}
	
#ifdef _MACOSX
	opt = 1;
	setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, &opt, sizeof(opt));
#endif
	if (fdp)
		*fdp = s;
	freeaddrinfo(res0);
	return 1;
}


#ifndef _ENABLE_IPv6
/*
* Accepts a TCP connection on a given port and invokes the given
* callback to handle it. This is where the main server thread sits.
*	IPv4 Version
*/
int
stream_server_accept(int port, void(*clfunc)(int fd, const char *addr,
					 int port), char *errdesc, int errdesclen)
{
	struct sockaddr_in sin = { 0 };
	struct sockaddr_in cin = { 0 };
	const int opt = 1;
	socklen_t cl;
	char *addr;
	int fd;
	int s;
	int errcount=0;
	//#ifdef _WINDOWS
	//	u_long nNonBlocking = TRUE;
	//#endif

	if (!(s = (int)socket(PF_INET, SOCK_STREAM, 0))) {
		SET_ERRDESC(errdesc, errdesclen);
		return 0;
	}
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof (opt));
#ifdef _MACOSX
	setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, (void *)&opt, sizeof (opt));
#endif
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;
	if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) != 0) {
		SET_ERRDESC(errdesc, errdesclen);
		return 0;
	}
	if (listen(s, 5) != 0) {
		SET_ERRDESC(errdesc, errdesclen);
		return 0;
	}
	for (;;) {
		cl = sizeof(cin);
		while ((fd = (int)accept(s, (struct sockaddr *)&cin, &cl)) < 0 &&
			errno == EAGAIN)
			;
		if (fd < 0) {
			SET_ERRDESC(errdesc, errdesclen);
			pu_log(PUL_WARN, 0, "Error in stream_server_accept: %s",errdesc);
			if(errcount < 10)
			{
				SLEEP(50);
				continue;
			}
			else
			{
				pu_log(PUL_CRIT, 0, "Too many errors in stream_server_accept, exiting.");
				return 0;
			}
		}
		errcount = 0;
		setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof (opt));

#ifdef _MACOSX
		// otherwise we get SIGPIPEs that aren't important
		setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&opt, sizeof (opt));
#endif

		addr = strdup(inet_ntoa(cin.sin_addr));
		clfunc(fd, addr, ntohs(cin.sin_port));
		free(addr); addr = NULL;
	}
	/*NOTREACHED*/
}

#else

/*
* Accepts a TCP connection on a given port and invokes the given
* callback to handle it. This is where the main server thread sits.
* 	IPv6 version! - doesn't work with Windows yet...
*/
int
stream_server_accept(int port, 
					 void(*clfunc)(int fd, const char *addr,int port), 
					 char *errdesc, 
					 int errdesclen)
{
	struct addrinfo *ai, *runp;
	struct addrinfo hints;
	int e, nfds;
	char portStr[20];
	struct pollfd *fds;
	int opt;

	// Get a list of interfaces to Bind to
	memset(&hints, '\0', sizeof(hints));
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	hints.ai_socktype = SOCK_STREAM;
	//hints.ai_family = PF_UNSPEC;

	snprintf(portStr, 20, "%d", port);
	if((e = getaddrinfo(NULL, portStr, &hints, &ai)) != 0)
	{
		(errdesc ? snprintf(errdesc, errdesclen, "getaddrinfo: %s", gai_strerror(e)) : 0);
		return 0;
	}

	// Malloc array
	nfds = 0;
	runp = ai;
	while(runp)
	{
		nfds++;
		runp = runp->ai_next;
	}
	fds = (struct pollfd *)malloc(nfds * sizeof(struct pollfd));
	
	pu_log(PUL_INFO, 0, "Found %d interfaces to Bind to.", nfds);

	// Bind to interfaces
	for(nfds = 0, runp = ai; runp; runp = runp->ai_next)
	{
		pu_log(PUL_INFO, 0, "Setting up interface %d: Family:%d, Socktype:%d, Protocol:%d", nfds, runp->ai_family, runp->ai_socktype, runp->ai_protocol);
		
		if((fds[nfds].fd = socket(runp->ai_family, runp->ai_socktype, runp->ai_protocol)) < 0)
		{
			pu_log(PUL_WARN, 0, "Error on socket: %s", strerror(errno));
			continue;
		}

		fds[nfds].events = POLLIN;
		opt = 1;
		setsockopt(fds[nfds].fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#ifdef _MACOSX
		opt = 1;
		setsockopt(fds[nfds].fd, SOL_SOCKET, SO_NOSIGPIPE, &opt, sizeof(opt));
#endif

		if(bind(fds[nfds].fd, runp->ai_addr, runp->ai_addrlen) != 0)
		{
			if(errno != EADDRINUSE)
			{
				pu_log(PUL_WARN, 0, "Error on bind: %s", strerror(errno));
				continue;
			}
			else
				close(fds[nfds].fd);
		}
		else //bind was successfull
		{
			if(listen(fds[nfds].fd, SOMAXCONN) != 0)
			{
				pu_log(PUL_WARN, 0, "Error on listen: %s", strerror(errno));
				continue;
			}
			else //listen was successfull
				nfds++;
		}

	}
	freeaddrinfo(ai);

	// didn't bind to anything!
	if(nfds == 0)
	{
		(errdesc ? snprintf(errdesc, errdesclen, "Couldn't bind to any interfaces!") : 0);
		return 0;
	}

	while(1)
	{
		if(poll(fds, nfds, -1) > 0)
		{
			int i;
			for(i = 0; i < nfds; i++)
			{
				if(fds[i].revents & POLLIN)
				{
					struct sockaddr_storage rem;
					socklen_t remlen = sizeof(rem);
					int fd;
					char addr[200];

					while((fd = accept(fds[i].fd, (struct sockaddr *) &rem, &remlen)) < 0 && errno == EAGAIN);

					if(fd < 0)
					{
						SET_ERRDESC(errdesc, errdesclen);
						return 0;
					}

					opt = 1;
					setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof (opt));
#ifdef _MACOSX
					opt = 1;
					setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&opt, sizeof (opt));
#endif

					(void)getnameinfo((struct sockaddr *)&rem, remlen, addr, sizeof(addr), NULL, 0, NI_NUMERICHOST);
					clfunc(fd, addr, ntohs(((struct sockaddr_in *)&rem)->sin_port));
				}
			}
		}
	}
	/*NOTREACHED*/
}
#endif

int
pu_write(int fd, const void *buf, unsigned int len, char *errdesc, int edlen)
{
	//why a static mutex instead of a per-socket one???
	//Why a mutex at all?? I'm oging to disable and see what happens...
	//static pthread_mutex_t *writelock = 0;
	int res = 0;
	int olen = len;
	int timeout = 20000; //20 seconds
#ifdef _WINDOWS
	int bytesSent = 0;
	WSABUF DataBuf;
#endif

	if (fd == -1)
		return len;

	pu_log(PUL_VERB, 0, "Sending: \"%s\"",(char *)buf);

	//if (!writelock) {
	//	if (!(writelock = malloc(sizeof (*writelock))))
	//		return 0;
	//	pthread_mutex_init(writelock, NULL);
	//}
	//pthread_mutex_lock(writelock);
	do {
		if (res > 0) {
#ifdef _WINDOWS
			(unsigned char *)buf += res;
#else
			buf += res;
#endif
			len -= res;
		}
		if (len)
		{
tryagain:
			if(!timeout)
				return 0;
#ifdef _WINDOWS
			DataBuf.buf = (char *)buf;
			DataBuf.len = len;

			res = WSASend(fd, &DataBuf, 1, &bytesSent, 0, NULL, NULL);
			if(res)
				res = -1;
			else
				res = bytesSent;
#else
			res = send(fd, buf, len, MSG_NOSIGNAL);
#endif
			//Error
			if(res == -1)
			{
#ifdef _WINDOWS
				switch(WSAGetLastError())
				{
					//non-blocking, try again
				case WSAEWOULDBLOCK:
					timeout-=10;
					SLEEP(10);
					goto tryagain;
					//any other error, don't try again
				default:
					break;
				}
#else
				switch(errno)
				{
					//Interrupted, try again
				case EINTR:
					//non-blocking, try again
				case EAGAIN:
					timeout-=10;
					SLEEP(10);
					goto tryagain;
					//any other error, don't try again
				default:
					break;
				}
#endif
			}
		}
		else
			res = olen;
	} while (len && res >= 0);
	//pthread_mutex_unlock(writelock);

	if (res != olen) {
		SET_ERRDESC(errdesc, edlen);
		return 0;
	} else
		return 1;
}

int
pu_read(int fd, void *buf, unsigned int len, char *errdesc, int edlen)
{
	int res=0;
	if (fd == -1)
		return 0;

tryagain:
	res = recv(fd, buf, len, 0);
	//Error
	if(res == -1)
	{
#ifdef _WINDOWS
		switch(WSAGetLastError())
		{
			//non-blocking, try again
		case WSAEWOULDBLOCK:
			SLEEP(10);
			goto tryagain;
			//any other error, don't try again
		default:
			break;
		}
#else
		switch(errno)
		{
			//Interrupted, try again
		case EINTR:
			//non-blocking, try again
		case EAGAIN:
			SLEEP(10);
			goto tryagain;
			//any other error, don't try again
		default:
			break;
		}
#endif
	}
	if (res <= 0) {
		SET_ERRDESC(errdesc, edlen);
		return 0;
	} else
	{
		pu_log(PUL_VERB,0,"Received: \"%s\"", (char *)buf);
		return res;
	}
}

int
pu_close(int fd, char *errdesc, int edlen)
{
	int res=0;
	if(fd == -1) return 0;
#ifndef _WINDOWS
	res = close(fd);
#else
	res = closesocket(fd);
#endif
	SET_ERRDESC(errdesc, edlen);
	return res;
}

// We want to use phidget21 logging not utils.c logging
#ifndef USE_PHIDGET21_LOGGING
static const char *
pu_log_level_str(pu_log_level_t l)
{
	switch (l) {
		case PUL_ERR:
			return "ERR";
		case PUL_CRIT:
			return "CRIT";
		case PUL_WARN:
			return "WARN";
		case PUL_INFO:
			return "INFO";
		case PUL_DEBUG:
			return "DEBUG";
		case PUL_VERB:
			return "VERBOSE";
	}
	return "???";
}

void
pu_log_stream(FILE *str)
{
	log_stream = str;
}

void
pu_log_pname(const char *newname)
{
	log_pname = newname;
}

void
pu_log(pu_log_level_t l, int s, const char *fmt, ...)
{
	static pthread_mutex_t logLock;
	static int logLockInitialized = FALSE;
	va_list va;

	if(!logLockInitialized)
	{
		pthread_mutex_init(&logLock, NULL);
		logLockInitialized = TRUE;
	}
	pthread_mutex_lock(&logLock);

#ifndef DEBUG
	if(logging_enabled)
#endif
	{
#ifdef WINCE
		if (!log_stream)
			log_stream = stdout;
		fprintf(log_stream, "%s%s%d/%s ", log_pname ? log_pname : "",
			log_pname ? " " : "", s, pu_log_level_str(l));
		va_start(va, fmt);
		vfprintf(log_stream, fmt, va);
		va_end(va);
		fprintf(log_stream, "\n");
		fflush(log_stream);
#else
		char date[50];
		struct tm *tm;
		time_t t;

		if (!log_stream)
			log_stream = stdout;

		time(&t);
		tm = localtime(&t);
		if (!strftime(date, sizeof (date), "%c", tm))
			strncpy(date, "?", sizeof (date));
		fprintf(log_stream, "%s %s%s%d/%s ", date, log_pname ? log_pname :
			"", log_pname ? " " : "", s, pu_log_level_str(l));
		va_start(va, fmt);
		vfprintf(log_stream, fmt, va);
		va_end(va);
		fprintf(log_stream, "\n");
		fflush(log_stream);
#endif
	}
	pthread_mutex_unlock(&logLock);
}
#else
void
pu_log(pu_log_level_t l, int s, const char *fmt, ...)
{
	CPhidgetLog_level level;
	char msg[2048], id[10];
	va_list va;

	switch(l)
	{
		case PUL_ERR:
			level = PHIDGET_LOG_ERROR;
			break;
		case PUL_CRIT:
			level = PHIDGET_LOG_CRITICAL;
			break;
		case PUL_WARN:
			level = PHIDGET_LOG_WARNING;
			break;
		case PUL_INFO:
			level = PHIDGET_LOG_INFO;
			break;
		case PUL_DEBUG:
			level = PHIDGET_LOG_DEBUG;
			break;
		case PUL_VERB:
		default:
			level = PHIDGET_LOG_VERBOSE;
			break;
	}

	va_start(va, fmt);
#ifdef WINCE
	vsprintf(msg, fmt, va);
#else
	vsnprintf(msg, 2048, fmt, va);
#endif
	va_end(va);

	snprintf(id, 10, "%d", s);
	CPhidget_log(level, id, msg);
}
#endif

int
hexval(unsigned char c)
{
	if (isdigit(c))
		return c - '0';
	c = tolower(c);
	if (c <= 'f' && c >= 'a')
		return c - 'a' + 10;
	return -1;
}

int
unescape(const char *src, char **dstp, unsigned int *dlenp)
{
	char *dst;
	size_t slen;
	size_t dlen;
	size_t i;

	for (i = 0, dlen = 0, slen = strlen(src); i < slen; i++, dlen++)
		if (src[i] == '\\')
			i += 3;

	if (!(dst = malloc(dlen + 1)))
		return 0;
	for (i = 0, dlen = 0, slen = strlen(src); i < slen; i++, dlen++)
		if (src[i] == '\\') {
			dst[dlen] = hexval((unsigned)src[i + 2]) * 16 +
				hexval((unsigned)src[i + 3]);
			i += 3;
		} else
			dst[dlen] = src[i];
		if (dlenp)
			*dlenp = (unsigned int)dlen;
		dst[dlen] = 0;

		*dstp = dst;
		return 1;
}

static unsigned char
hexchar(unsigned char c)
{
	if (c > 16)
		return 'f';
	if (c >= 10)
		return c - 10 + 'a';
	return c + '0';
}


int
escape2(const char *src, unsigned int slen, char **dstp, int escbacks)
{
	char *dst;
	size_t dlen;
	size_t i;

	if (!slen)
		slen = (int)strlen(src);
	for (i = 0, dlen = 0; i < slen; i++)
		if (PASSTHRU(src[i]))
			dlen++;
		else
			if(escbacks)
				dlen += 6;
			else
				dlen += 4;

	if (!(dst = malloc(dlen + 1)))
		return 0;
	for (i = 0, dlen = 0; i < slen; i++)
		if (!PASSTHRU(src[i])) {
			dst[dlen++] = '\\';
			if(escbacks)
				dst[dlen++] = '\\';
			dst[dlen++] = 'x';
			dst[dlen++] = hexchar((unsigned char)src[i] / 16);
			dst[dlen++] = hexchar((unsigned char)src[i] % 16);
		} else {
			dst[dlen++] = src[i];
		}
		dst[dlen++] = 0;

		*dstp = dst;
		return 1;
}

int
escape(const char *src, unsigned int slen, char **dstp)
{
	return escape2(src, slen, dstp, PFALSE);
}

int byteArrayToString(unsigned char *bytes, int length, char *string)
{
	int i;
	for(i=0;i<length;i++)
	{
		sprintf(string+(i*2), "%02x", bytes[i]);
	}
	return EPHIDGET_OK;
}

int stringToByteArray(char *string, unsigned char *bytes, int *length)
{
	int i;

	if(strlen(string) > (size_t)((*length)*2))
		return EPHIDGET_INVALIDARG;

	for(i=0;i<(int)strlen(string);i+=2)
	{
		if(hexval(string[i])==-1)
			break;
		if((i/2)>*length)
			return EPHIDGET_INVALIDARG;
		bytes[i/2] = (hexval(string[i])<<4)+hexval(string[i+1]);
	}
	*length = i/2;

	return EPHIDGET_OK;
}

//up to 1048575 (0xFFFFF) per int (not full 32-bit!)
//IR max is 327670
int wordArrayToString(int *words, int length, char *string)
{
	int i;
	for(i=0;i<length;i++)
	{
		if(words[i] > 0xFFFFF)
			sprintf(string+(i*5), "fffff");
		else
			sprintf(string+(i*5), "%05x", words[i]);
	}
	return EPHIDGET_OK;
}

//up to 1048575 (0xFFFFF) per int (not full 32-bit!)
int stringToWordArray(char *string, int *words, int *length)
{
	int i;

	for(i=0;i<(int)strlen(string);i+=5)
	{
		if(hexval(string[i])==-1)
			break;
		if((i/5)>*length)
			return EPHIDGET_INVALIDARG;
		words[i/5] = (hexval(string[i])<<16)+
			(hexval(string[i+1])<<12)+
			(hexval(string[i+2])<<8)+
			(hexval(string[i+3])<<4)+
			hexval(string[i+4]);
		if(words[i/5] == 0xfffff)
			words[i/5] = PUNK_INT;
	}
	*length = i/5;

	return EPHIDGET_OK;
}

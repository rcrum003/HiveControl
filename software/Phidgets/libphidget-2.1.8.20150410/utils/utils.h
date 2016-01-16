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

#ifndef _UTILS_H_
#define _UTILS_H_
#include <regex.h>
#include <stdio.h>
#include <stdarg.h>

typedef enum {
	PUL_ERR = 1,
	PUL_CRIT,
	PUL_WARN,
	PUL_INFO,
	PUL_DEBUG,
	PUL_VERB
} pu_log_level_t;

int pasprintf(char **ret, const char *fmt, ...);
int pvasprintf(char **ret, const char *fmt, va_list ap);

int pd_getline(char *buf, unsigned int bufsize, int *bufcur, int *buflen,
    int(*readfunc)(int, void *, unsigned int, char *errdesc, int errlen),
    int(*closefunc)(int, char *errdesc, int errlen), int readfd,
    char **line, char *errdesc, int errlen);
//int pd_getline_simple(int fd, char **line);

int getmatchsub(const char *line, char **subp, const regmatch_t pmatch[],
    int n);
int stream_server_accept(int port, void(*clfunc)(int fd, const char *addr,
    int port), char *errdesc, int errlen);
int CCONV stream_server_connect(const char *dest, const char *svcname, 
								int *fdp, int *cancelSocket, char *errdesc, int errdesclen);

int pu_write(int fd, const void *buf, unsigned int len, char *errdesc,
    int edlen);
int pu_read(int fd, void *buf, unsigned int len, char *errdesc, int edlen);
int pu_close(int fd, char *errdesc, int edlen);
extern int logging_enabled;
void pu_log_stream(FILE *);
void pu_log(pu_log_level_t l, int s, const char *fmt, ...);
int escape(const char *src, unsigned int srclen, char **dstp);
int escape2(const char *src, unsigned int slen, char **dstp, int escbacks);
int unescape(const char *src, char **dstp, unsigned int *dstlenp);
int cancelConnect(int cancelSocket);

int byteArrayToString(unsigned char *bytes, int length, char *string);
int stringToByteArray(char *string, unsigned char *bytes, int *length);
int wordArrayToString(int *words, int length, char *string);
int stringToWordArray(char *string, int *words, int *length);

#endif

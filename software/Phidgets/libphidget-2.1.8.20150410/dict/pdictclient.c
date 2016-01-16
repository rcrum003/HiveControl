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
#include <stdlib.h>
#include <stdio.h>
#ifndef _MSC_EXTENSIONS
#include <unistd.h>
#endif
#include <assert.h>
#if !defined(_WINDOWS) || defined(_CYGWIN)
#include <pthread.h>
#include <signal.h>
#else
#include "wincompat.h"
#endif
#include "pdictclient.h"
#include "ptree.h"
#include "utils.h"
#include "md5.h"

/*
 * TODO XXX:
 * - notification arguments and reason
 */

#ifdef DEBUG_PROTOCOL
int debug_protocol = 0;
#endif

#define ABORT() ((*(int *)0 = 1), abort())
#define PENDING_PATTERN "report 200-lid([0-9]*) is pending, key (.*)" \
    " latest value \".*\" \\((.*)\\)"

extern const char *ws_protocol_ver;

typedef struct result {
	int r_tag;
	void (*r_notify)(pdc_session_t *, struct result *, int code,
	    int final, const char *line);
	void *r_arg;
} result_t;

struct pdc_session {
	int pdcs_rfd;
	int pdcs_wfd;
	int (*pdcs_read)(int, void *, unsigned int, char *, int);
	int (*pdcs_write)(int, const void *, unsigned int, char *, int);
	int (*pdcs_close)(int, char *, int);
	void (*pdcs_cleanup)(void *);
	void *pdcs_cleanup_ptr;
	ptree_node_t *pdcs_listeners;
	char pdcs_readbuf[2048];
	int pdcs_bufcur;
	int pdcs_buflen;
	pthread_mutex_t pdcs_listeners_lock;
	pthread_mutex_t pdcs_pending_lock;
	ptree_node_t *pdcs_pending;
	pthread_t pdcs_resultreader;
	char pdcs_resultreader_errdesc[2048];
};

typedef struct {
	int l_id;
	void (*l_cb)(const char *key, const char *val, unsigned int len,
	    pdict_reason_t, void *);
	void *l_arg;
} listener_t;

typedef struct {
	pthread_mutex_t grcba_lock;
	pthread_cond_t grcba_cv;
	int grcba_code;
	int grcba_desired;
	char *grcba_line;
} getresult_cb_arg_t;

typedef struct {
	pthread_mutex_t grcba_lock;
	pthread_cond_t grcba_cv;
	int grcba_code;
	int grcba_reply_len;
	char *grcba_reply;
	char *grcba_reply_ptr;
} jgetresult_cb_arg_t;

static int events;
regex_t pendingex;
static int initialized;

static void *read_results(void *);
static pdict_reason_t _pdict_reason_from_str(const char *s);
static int cmd_timeout = 10000; //ms

/*
 * Stops the read thread and frees the pdc session
 */
void pdc_session_free(pdc_session_t *pdcs)
{
	if(pdcs)
	{
		//make sure the thread is not running...
		pdc_readthread_join(pdcs, NULL);
		
		//free
		pthread_mutex_destroy(&pdcs->pdcs_pending_lock);
		pthread_mutex_destroy(&pdcs->pdcs_listeners_lock);
		free(pdcs); pdcs = NULL;
	}
	return;
}

/*
 * Allocates a new pdc session and starts a thread reading on readfd
 */
pdc_session_t * CCONV
pdc_session_alloc(int readfd, int(*readfunc)(int, void *, unsigned int, char *,
    int), int writefd, int(*writefunc)(int, const void *, unsigned int, char *,
    int), int (*closefunc)(int, char *, int), void *cleanupPtr, void (*cleanupFunc)(void *))
{
	pdc_session_t *pdcs;
#ifndef _WINDOWS
	sigset_t new, old;
#endif
#ifdef _WINDOWS
	int opt = 1;
	int ssoret;
#endif

	if (!initialized)
		pdc_init();

	if (!(pdcs = malloc(sizeof (*pdcs))))
		return NULL;
	memset(pdcs, 0, sizeof (*pdcs));
	pdcs->pdcs_rfd = readfd;
	pdcs->pdcs_read = readfunc;
	pdcs->pdcs_wfd = writefd;
	pdcs->pdcs_write = writefunc;
	pdcs->pdcs_close = closefunc;
	pdcs->pdcs_cleanup_ptr = cleanupPtr;
	pdcs->pdcs_cleanup = cleanupFunc;
	if (pthread_mutex_init(&pdcs->pdcs_pending_lock, 0) != 0
		|| pthread_mutex_init(&pdcs->pdcs_listeners_lock, 0) != 0) {
		free(pdcs);
		return NULL;
	}
#ifndef _WINDOWS
	sigfillset(&new);
	pthread_sigmask(SIG_BLOCK, &new, &old);
#endif

#ifdef _WINDOWS
	//Disable Nagle
	opt = TRUE;
	ssoret = setsockopt(readfd, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof (opt));
	//set send buffer to 0 (send data right away)
	opt = 0;
	ssoret = setsockopt(readfd, SOL_SOCKET, SO_SNDBUF, (void *)&opt, sizeof (opt));
#endif

	if (pthread_create(&pdcs->pdcs_resultreader, 0, read_results, pdcs) !=
	    0) {
		pthread_mutex_destroy(&pdcs->pdcs_pending_lock);
		pthread_mutex_destroy(&pdcs->pdcs_listeners_lock);
		free(pdcs); pdcs = NULL;
		return NULL;
	}
#ifdef _WINDOWS
	//pthread_up_priority(pdcs->pdcs_resultreader);
#endif
#ifndef _WINDOWS
	pthread_sigmask(SIG_SETMASK, &old, NULL);
#endif

	return pdcs;
}

/*
 * Sets up regex for PENDING_PATTERN
 */
int CCONV
pdc_init(void)
{
	int res;

	if ((res = regcomp(&pendingex, PENDING_PATTERN, REG_EXTENDED)) != 0) {
		LOG_STDERR(PHIDGET_LOG_CRITICAL, "pending report pattern compilation error %d", res);
		abort();
	}
	initialized = 1;
	return 1;
}

/*
 * Compares 2 listener IDs
 */
static int
lcmp(const void *sv, const void *tv)
{
	return ((const listener_t *)sv)->l_id - ((const listener_t *)tv)->l_id;
}

/*
 * Compares 2 tag IDs
 */
static int
tagcmp(const void *sv, const void *tv)
{
	return ((result_t *)sv)->r_tag - ((result_t *)tv)->r_tag;
}

/*
 * Handles a report from the read thread
 */
static void
handle_report(pdc_session_t *pdcs, char *line)
{
	regmatch_t pmatch[7];
	listener_t *lp;
	listener_t l;
	char *idstr = NULL;
	char *key = NULL;
	char *val = NULL;
	char *eoval = NULL; //end of val
	char *soval = NULL; //start of val
	char *reason_str = NULL;
	pdict_reason_t r;
	int res;
	//char err[1024];

	events++;

//Disable this, I think it's causing bad problems!
#if 0
#ifdef _WINDOWS
	//Send something back if we're on Windows, so this report gets ACKed right away
	// otherwise, we face 200ms delayed ack -> clumps of events every 200ms
	pdcs->pdcs_write(pdcs->pdcs_wfd, "report ack\n", strlen("report ack\n"), err, sizeof (err));
#endif
#endif

	/* handle value without regex, due to crappy regex implementations */
	/* val is bounded by double quotes */

	if (!(val = strchr(line, '\"')))
		goto ignore;
	soval = ++val;
	if (!(eoval = strchr(val, '\"')))
		goto ignore;
	
	/* end of val \" = NULL */
	*eoval = '\0';
	
	/* make sure there are no more double quotes */
	if (strchr(eoval + 1, '\"') != NULL)
		goto ignore;
	
	/* create a copy of val, or undo my changes if there is not enough memory */
	if (!(val = strdup(val))) {
		*eoval = '\"';
		pu_log(PUL_WARN, pdcs->pdcs_rfd,
		    "report dropped due to low memory");
		goto ignore;
	}
	
	/* put back what I changed and shift anything after val to the start of val */
	*eoval = '\"';
	memmove(soval, eoval, strlen(eoval) + 1);

	if ((res = regexec(&pendingex, line, 6, pmatch, 0)) != 0) {
ignore:
#ifdef DEBUG_PROTOCOL
		pu_log(PUL_DEBUG, pdcs->pdcs_rfd,
		    "ignoring invalid report (see next line):");
		pu_log(PUL_DEBUG, pdcs->pdcs_rfd, line);
#endif
		goto end;
	}
	if (!getmatchsub(line, &idstr, pmatch, 1) || !idstr)
	{
		goto end;
	}
	if (!getmatchsub(line, &key, pmatch, 2) || !key)
	{
		goto end;
	}
	if (!getmatchsub(line, &reason_str, pmatch, 3) || !reason_str ||
	    !(r = _pdict_reason_from_str(reason_str)))
	{
		goto end;
	}
	free(reason_str); reason_str = NULL;

	l.l_id = atoi(idstr);
	pthread_mutex_lock(&pdcs->pdcs_listeners_lock);
	if (ptree_contains(&l, (ptree_node_t *)pdcs->pdcs_listeners, lcmp,
	    (void **)&lp) && lp->l_cb) {
		char *ueval;
		unsigned int uevlen;

		pthread_mutex_unlock(&pdcs->pdcs_listeners_lock);
		if (!unescape(val, &ueval, &uevlen)) {
			/* XXX log */
			goto end;
		}
		lp->l_cb(key, ueval, uevlen, r, lp->l_arg);
		free(ueval);
	}
	else
	{
		pthread_mutex_unlock(&pdcs->pdcs_listeners_lock);
		LOG(PHIDGET_LOG_WARNING, "No callback found for: %s",line);
	}
end:
	free(val);
	free(idstr);
	free(key);
}

/*
 * Called by read thread for any pending commands that didn't complete before the socket was closed.
 */
static ptree_walk_res_t
finish_pending_async(const void *node, int level, void *arg, void *pwra)
{
	pdc_session_t *pdcs = arg;
	result_t *r;
	char msg[1024];

	ptree_inorder_walk_remove(&pdcs->pdcs_pending, (void **)&r, pwra, tagcmp);
	assert(node == r);

	snprintf(msg, 1024, "Socket was closed before command T%d finished.",r->r_tag);

	//notify frees r
	if(r->r_notify)
		r->r_notify(pdcs, r, 500, PTRUE, msg);

	//free(r);

	return PTREE_WALK_CONTINUE;
}
static ptree_walk_res_t
free_node(const void *node, int level, void *arg, void *pwra)
{
	pdc_session_t *pdcs = arg;
	void *ov;
	ptree_inorder_walk_remove(&pdcs->pdcs_listeners, &ov, pwra, lcmp);
	assert(node == ov);
	free(ov);	
	return PTREE_WALK_CONTINUE;
}

/*
 * Runs in a thread started by pdc_session_alloc
 */
static void *
read_results(void *arg)
{
	pdc_session_t *pdcs = arg;
	char *line = 0;
	result_t *r;
	result_t rk;
	char *startp;
	int linelen;
	int final;
	int tag;

next:
	if (line) {
		free(line); line = NULL;
	}
	pdcs->pdcs_resultreader_errdesc[0] = 0;
	if (!pd_getline((char *)pdcs->pdcs_readbuf, sizeof (pdcs->pdcs_readbuf),
	    &pdcs->pdcs_bufcur, &pdcs->pdcs_buflen, pdcs->pdcs_read, pdcs->pdcs_close,
	    pdcs->pdcs_rfd, &line, pdcs->pdcs_resultreader_errdesc,
	    sizeof (pdcs->pdcs_resultreader_errdesc)))
	{
		/* socket closed */
		free(line); line=NULL;
		
		//respond for any pending commands (failed)...
		pthread_mutex_lock(&pdcs->pdcs_pending_lock);
		ptree_walk(pdcs->pdcs_pending, PTREE_POSTORDER, finish_pending_async, tagcmp, pdcs);
		assert(!pdcs->pdcs_pending);
		//ptree_clear(&pdcs->pdcs_pending);
		//pdcs->pdcs_pending = NULL;
		pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
		
		//remove listeners
		pthread_mutex_lock(&pdcs->pdcs_listeners_lock);
		ptree_walk(pdcs->pdcs_listeners, PTREE_POSTORDER, free_node, lcmp, pdcs);
		assert(!pdcs->pdcs_listeners);
		pthread_mutex_unlock(&pdcs->pdcs_listeners_lock);
		//ptree_clear(&pdcs->pdcs_listeners);
		
		//do cleanup - this handles stuff outside on the dictionary code, etc.
		if (pdcs->pdcs_cleanup)
			pdcs->pdcs_cleanup(pdcs->pdcs_cleanup_ptr);
		if(fptrJavaDetachCurrentThread)
			fptrJavaDetachCurrentThread();
		return (void *)(size_t)(-1 * errno); // XXX pdcs errno function
	}

	//LOG(PHIDGET_LOG_DEBUG, "read_results: %s",line);

	if (line[0] == 'r') {
		handle_report(pdcs, line);
		goto next;
	} else if (line[0] == 'T') {
		tag = atoi(line + 1);
		startp = strchr(line, ' ');
		while (startp && *startp) {
			if (isdigit(*startp))
				goto notify;
			startp++;
		}
		// XXX log malformed line
		goto next;
	} else {
		startp = line;
		tag = 0;
	}
notify:
	if ((linelen = (int)strlen(line)) < 4) {
		// XXX log malformed line
		goto next;
	}
	final = (startp[3] == ' ');
	pthread_mutex_lock(&pdcs->pdcs_pending_lock);
	rk.r_tag = tag;
	if (final) {
		if (!ptree_remove(&rk, &pdcs->pdcs_pending, tagcmp,
		    (void **)&r)) {
			//ABORT();
			pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
			// XXX log unanticipated result
			goto next;
		}
	} else {
		if (!ptree_contains(&rk, pdcs->pdcs_pending, tagcmp,
		    (void **)&r)) {
			//ABORT();
			pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
			// XXX log unanticipated result
			goto next;
		}
	}
	pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
	r->r_notify(pdcs, r, atoi(startp), final, startp + 4);
	r = NULL;
	goto next;
//	return  (void *)(0);
}

/*
 * Callback from cmd
 */
static void
getresult_cb(pdc_session_t *pdcs, result_t *r, int code, int final,
    const char *line)
{
	getresult_cb_arg_t *grcba = r->r_arg;

	if (final) {
		if (pthread_mutex_lock(&grcba->grcba_lock))
			abort();
		grcba->grcba_code = code;
		if (grcba->grcba_code != grcba->grcba_desired)
			grcba->grcba_line = strdup(line);
		if (pthread_mutex_unlock(&grcba->grcba_lock))
			abort();
		if (pthread_cond_signal(&grcba->grcba_cv))
			abort();
	}
}

/*
 * Callback from jcmd
 */
static void
jgetresult_cb(pdc_session_t *pdcs, result_t *r, int code, int final,
    const char *line)
{
	jgetresult_cb_arg_t *grcba = r->r_arg;
	int spaceleft = grcba->grcba_reply_len - (grcba->grcba_reply_ptr-grcba->grcba_reply);

	if (pthread_mutex_lock(&grcba->grcba_lock))
		abort();

	if (final) 
	{
		grcba->grcba_code = code;
	}

	//add more space if needed
	if((size_t)spaceleft < strlen(line)+1)
	{
		int offset = grcba->grcba_reply_ptr-grcba->grcba_reply;
		if(!(grcba->grcba_reply = (char *)realloc(grcba->grcba_reply, strlen(line) + offset + 100)))
		{
			LOG(PHIDGET_LOG_WARNING,"Couldn't realloc!");
			return;
		}
		grcba->grcba_reply_len = strlen(line) + offset + 100;
		grcba->grcba_reply_ptr = grcba->grcba_reply + offset;
		spaceleft = grcba->grcba_reply_len - (grcba->grcba_reply_ptr-grcba->grcba_reply);
	}

	strncpy(grcba->grcba_reply_ptr, line,
	    spaceleft - 1);
	if(!final)
		grcba->grcba_reply_ptr[strlen(line)] = '\n';
	grcba->grcba_reply_ptr+=(strlen(line)+1);

	if (pthread_mutex_unlock(&grcba->grcba_lock))
		abort();
		
	if (final) 
	{
		if (pthread_cond_signal(&grcba->grcba_cv))
			abort();
	}
}

static int wait_timeout(pthread_mutex_t *grcba_lock, pthread_cond_t *grcba_cv, int timeout_ms)
{
	int retval;
#ifdef _WINDOWS
	int timeout = timeout_ms;
#else
	struct timespec timeout;
	struct timeval now;
	
    // Lock the mutex.
    //pthread_mutex_lock(&grcba->grcba_lock);
	
	gettimeofday(&now,0);
	timeout.tv_sec = now.tv_sec + timeout_ms/1000;
	timeout.tv_nsec = now.tv_usec*1000 + (timeout_ms%1000 * 1000000);
	if(timeout.tv_nsec >= 1000000000)
	{
		timeout.tv_sec++;
		timeout.tv_nsec -= 1000000000;
	}
#endif
	retval = pthread_cond_timedwait(grcba_cv, grcba_lock, &timeout);
	//pthread_mutex_unlock(&grcba->grcba_lock); 
	return retval;
}

static int
cmd(pdc_session_t *pdcs, 
	int desired, 
	const char *cmd, 
	char *errdesc,
    int errlen)
{
	int len = (int)strlen(cmd);
	getresult_cb_arg_t *grcba;
	result_t r;
	result_t *or;

	if (!(grcba = malloc(sizeof (*grcba)))) {
		if (errdesc)
			(void) snprintf(errdesc, errlen, "%s", strerror(errno));
		return 0;
	}
	r.r_tag = 0;
	r.r_notify = getresult_cb;
	r.r_arg = grcba;
	memset(grcba, 0, sizeof (*grcba));
	grcba->grcba_code = -1;
	grcba->grcba_desired = desired;
		
	if (pthread_mutex_init(&grcba->grcba_lock, NULL))
		abort();
	if (pthread_cond_init(&grcba->grcba_cv, 0))
		abort();
	if (pthread_mutex_lock(&pdcs->pdcs_pending_lock))
		abort();
	if (!ptree_replace(&r, &pdcs->pdcs_pending, tagcmp, (void **)&or)) {
		pthread_mutex_destroy(&grcba->grcba_lock);
		pthread_cond_destroy(&grcba->grcba_cv);
		pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
		free(grcba); grcba = NULL;
		if (errdesc)
			(void) snprintf(errdesc, errlen,
			    "result replacement failure");
		return 0;
	}
	
	// if or != NULL then pdcs_pending already contained a node with tag == r.r_tag
	assert(!or);
	if (pthread_mutex_lock(&grcba->grcba_lock))
		abort();
	if (pthread_mutex_unlock(&pdcs->pdcs_pending_lock))
		abort();

	if (!pdcs->pdcs_write(pdcs->pdcs_wfd, cmd, len, errdesc, errlen)) {
		pthread_mutex_unlock(&grcba->grcba_lock);
		pthread_mutex_lock(&pdcs->pdcs_pending_lock);
		ptree_remove(&r, &pdcs->pdcs_pending, tagcmp, (void **)&or);
		pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
		pthread_mutex_destroy(&grcba->grcba_lock);
		pthread_cond_destroy(&grcba->grcba_cv);
		free(grcba);
		return 0;
	}
	
	while (grcba->grcba_code == -1)
	{
		int retval;
		//5 second timeout
		if((retval = wait_timeout(&grcba->grcba_lock, &grcba->grcba_cv, cmd_timeout)) != 0)
		{
			switch (retval)
			{
				case ETIMEDOUT:
				default:
					if (errdesc)
						(void) snprintf(errdesc, errlen,"cmd: \"%s\" Timed out after %dms.",cmd,cmd_timeout);
					pthread_mutex_lock(&pdcs->pdcs_pending_lock);
					ptree_remove(&r, &pdcs->pdcs_pending, tagcmp, (void **)&or);
					pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
					pthread_mutex_destroy(&grcba->grcba_lock);
					pthread_cond_destroy(&grcba->grcba_cv);
					if (grcba->grcba_line)
						free(grcba->grcba_line);
					free(grcba);
					return 0; //Timeout
			}
		}
	}
	
	pthread_mutex_destroy(&grcba->grcba_lock);
	pthread_cond_destroy(&grcba->grcba_cv);
	
	if (grcba->grcba_code != desired)
	{
		if (errdesc)
			(void) snprintf(errdesc, errlen, "protocol error: %d%s%s", grcba->grcba_code, grcba->grcba_line ? " " : "", grcba->grcba_line ? grcba->grcba_line : "");
		if (grcba->grcba_line)
			free(grcba->grcba_line);
		free(grcba);
		return 0;
	}
	
	if (grcba->grcba_line)
		free(grcba->grcba_line);
	free(grcba);
	return 1;
}

/*
 * jcmd is for a command that returns results, like getKey
 */
static int
jcmd(pdc_session_t *pdcs, 
	 const char *cmd, 
	 int desired, 
	 char **reply,
	 int replylen, 
	 char *errdesc, 
	 int errlen)
{
	int len = (int)strlen(cmd);
	jgetresult_cb_arg_t *grcba;
	result_t r;
	result_t *or;

	if (!(grcba = malloc(sizeof (*grcba)))) {
		if (errdesc)
			(void) snprintf(errdesc, errlen, "%s", strerror(errno));
		return 0;
	}
	r.r_tag = 0;
	r.r_notify = jgetresult_cb;
	r.r_arg = grcba;
	memset(grcba, 0, sizeof (*grcba));
	grcba->grcba_code = -1;
	grcba->grcba_reply = *reply;
	grcba->grcba_reply_ptr = *reply;
	grcba->grcba_reply_len = replylen;
		
	if (pthread_mutex_init(&grcba->grcba_lock, NULL))
		abort();
	if (pthread_cond_init(&grcba->grcba_cv, 0))
		abort();
	if (pthread_mutex_lock(&pdcs->pdcs_pending_lock))
		abort();
	if (!ptree_replace(&r, &pdcs->pdcs_pending, tagcmp, (void **)&or)) {
		pthread_mutex_destroy(&grcba->grcba_lock);
		pthread_cond_destroy(&grcba->grcba_cv);
		pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
		free(grcba); grcba = NULL;
		if (errdesc)
			(void) snprintf(errdesc, errlen,
			    "result replacement failure");
		return 0;
	}
	// if or != NULL then pdcs_pending already contained a node with tag == r.r_tag
	assert(!or);
	if (pthread_mutex_lock(&grcba->grcba_lock))
		abort();
	if (pthread_mutex_unlock(&pdcs->pdcs_pending_lock))
		abort();

#ifndef _MSC_EXTENSIONS
#ifdef DEBUG_PROTOCOL
	if (debug_protocol) {
		write(1, "L: ", 3);
		write(1, cmd, len);
	}
#endif
#endif
	if (!pdcs->pdcs_write(pdcs->pdcs_wfd, cmd, len, errdesc, errlen)) {
		pthread_mutex_lock(&pdcs->pdcs_pending_lock);
		ptree_remove(&r, &pdcs->pdcs_pending, tagcmp, (void **)&or);
		pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
		pthread_mutex_destroy(&grcba->grcba_lock);
		pthread_cond_destroy(&grcba->grcba_cv);
		free(grcba); grcba = NULL;
		return 0;
	}
	while (grcba->grcba_code == -1)
	{
		int retval;
		//5 second timeout
		if((retval = wait_timeout(&grcba->grcba_lock, &grcba->grcba_cv, cmd_timeout)) != 0)
		{
			switch (retval)
			{
				case ETIMEDOUT:
				default:
					if (errdesc)
						(void) snprintf(errdesc, errlen,"jcmd: \"%s\" Timed out after %dms.",cmd,cmd_timeout);
					pthread_mutex_lock(&pdcs->pdcs_pending_lock);
					ptree_remove(&r, &pdcs->pdcs_pending, tagcmp, (void **)&or);
					pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
					pthread_mutex_destroy(&grcba->grcba_lock);
					pthread_cond_destroy(&grcba->grcba_cv);
					free(grcba); grcba = NULL;
					return 0;
			}
		}
		//if (pthread_cond_wait(&grcba->grcba_cv, &grcba->grcba_lock))
		//	abort();
	}
	pthread_mutex_destroy(&grcba->grcba_lock);
	pthread_cond_destroy(&grcba->grcba_cv);
	if (grcba->grcba_code != desired) {
		if (errdesc)
			(void) snprintf(errdesc, errlen,
			    "protocol error: %d%s%s", grcba->grcba_code,
			    grcba->grcba_reply ? " " : "", grcba->grcba_reply ?
			    grcba->grcba_reply : "");
		*reply = grcba->grcba_reply;
		free(grcba); grcba = NULL;
		return 0;
	}
	*reply = grcba->grcba_reply;
	free(grcba); grcba = NULL;
	return 1;
}

//int CCONV
//pdc_set(pdc_session_t *pdcs, const char *key, const char *val,
//    int len, int remove_on_close, char *errdesc, int errlen)
//{
//	char *escval;
//	char *buf;
//	int res;
//
//	if(val[0] == '\0')
//	{
//		if (!escape("\001", len, &escval)) {
//			if (errdesc)
//				(void) snprintf(errdesc, errlen, "%s", strerror(errno));
//			return 0;
//		}
//	}
//	else
//	{
//		if (!escape(val, len, &escval)) {
//			if (errdesc)
//				(void) snprintf(errdesc, errlen, "%s", strerror(errno));
//			return 0;
//		}
//	}
//	if ((len = pasprintf(&buf, "set %s=\"%s\"%s\n", key, escval,
//	    remove_on_close ? " for session" : "")) < 0) {
//		free((void *)escval); escval = NULL;
//		if (errdesc)
//			(void) snprintf(errdesc, errlen, "%s", strerror(errno));
//		return 0;
//	}
//	res = cmd(pdcs, 200, buf, errdesc, errlen);
//	free((void *)escval); escval = NULL;
//	free(buf); buf = NULL;
//	return res;
//}

/*
 * Returns a nonzero listen ID on success, 0 otherwise.
 */
pdc_listen_id_t CCONV
pdc_listen(pdc_session_t *pdcs, const char *pattern,
    void (*cb)(const char *, const char *, unsigned int, pdict_reason_t,
    void *), void *arg, char *errdesc, int errlen)
{
	listener_t *l;
	static int lid = 1;
	char *buf;
	int len;

	if(!pdcs)
		return 0;

	if (!(l = malloc(sizeof (*l)))) {
		if (errdesc)
			(void) snprintf(errdesc, errlen, "%s", strerror(errno));
		return 0;
	}
	l->l_id = lid;
	l->l_cb = cb;
	l->l_arg = arg;
	if ((len = pasprintf(&buf, "listen \"%s\" lid%d\n", pattern, lid++)) < 0)
	{
		if (errdesc)
			(void) snprintf(errdesc, errlen, "%s", strerror(errno));
		return 0;
	}

	pthread_mutex_lock(&pdcs->pdcs_listeners_lock);
	if (!ptree_replace(l, (ptree_node_t **)&pdcs->pdcs_listeners, lcmp, NULL)) {
		free(buf); buf = NULL;
		free(l); l = NULL;
		if (errdesc)
			(void) snprintf(errdesc, errlen, "%s", strerror(errno));
		pthread_mutex_unlock(&pdcs->pdcs_listeners_lock);
		return 0;
	}
	pthread_mutex_unlock(&pdcs->pdcs_listeners_lock);

	if (!cmd(pdcs, 200, buf, errdesc, errlen)) {
		free(buf); buf = NULL;
		return 0;
	}

	free(buf); buf = NULL;
	return l->l_id;
}

int CCONV
pdc_enable_periodic_reports(pdc_session_t *pdcs, int periodms,
    char *errdesc, int errlen)
{
	char *buf;
	int res;
	if(!pdcs)
		return 0;

	if (periodms <= 0) {
		if (errdesc)
			(void) snprintf(errdesc, errlen, "invalid period");
		return 0;
	}
	if (pasprintf(&buf, "report %d report\n", periodms) < 0) {
		if (errdesc)
			(void) snprintf(errdesc, errlen, "%s", strerror(errno));
		return 0;
	}
	res = cmd(pdcs, 200, buf, errdesc, errlen);
	free(buf); buf = NULL;
	return res;
}

int CCONV
pdc_disable_periodic_reports(pdc_session_t *pdcs, char *errdesc,
    int errlen)
{
	if(!pdcs)
		return 0;
	return cmd(pdcs, 200, "report 0 report\n", errdesc, errlen);
}

int CCONV
pdc_flush(pdc_session_t *pdc, char *errdesc, int errlen)
{
	if(!pdc)
		return 0;
	return cmd(pdc, 200, "flush\n", errdesc, errlen);
}

int CCONV
pdc_ignore(pdc_session_t *pdcs, pdc_listen_id_t id, char *errdesc,
    int errlen)
{
	char *buf;
	int res;

	if(!pdcs)
		return 0;

	if (pasprintf(&buf, "ignore lid%d\n", id) < 0) {
		if (errdesc)
			(void) snprintf(errdesc, errlen, "%s", strerror(errno));
		return 0;
	}
	res = cmd(pdcs, 200, buf, errdesc, errlen);
	free(buf); buf = NULL;
	return res;
}

//Async stuff starts here
static void
async_cmd_callback(pdc_session_t *pdcs, const char *cmd, 
	void (*r_notify)(pdc_session_t *, struct result *, int code,int final, const char *line), void *r_arg,
    void (*error)(const char *errdesc, void *arg), void *arg);

typedef struct {
	int aca_desired;
	void (*aca_error)(const char *errdesc, void *arg);
	void *aca_error_arg;
} async_cmd_arg_t;

/*
 * Default callback for async commands.
 * If the code matches what we wanted, then we don't hear anything about it.
 * Otherwise, we get the error callback.
 */
static void
async_cmd_cb(pdc_session_t *pdcs, result_t *r, int code, int final,
    const char *line)
{
	async_cmd_arg_t *aca;
	char *buf;

	assert(final);
	aca = (async_cmd_arg_t *)r->r_arg;
	if (aca->aca_desired != code) {
		if (pasprintf(&buf, "protocol error: %s", line) > 0) {
			if(aca->aca_error)
				aca->aca_error(buf, aca->aca_error_arg);
			free(buf); buf = NULL;
		} else {
			/* XXX warn about low-memory condition */
			if(aca->aca_error)
				aca->aca_error("protocol error (insufficient memory"
			    " to describe)", aca->aca_error_arg);
		}
	}
	free(aca); aca = NULL;
	free(r); r = NULL;
}

typedef struct {
	void (*aaa_success)(void *arg, void (*error)(const char *errdesc, void *arg));
	void (*aaa_error)(const char *errdesc, void *arg);
	void *aaa_arg;
	char *aaa_pass;
} async_auth_arg_t;

static void
async_auth_cb(pdc_session_t *pdcs, result_t *r, int code, int final,
    const char *line)
{
	async_auth_arg_t *aaa;

	assert(final);
	aaa = (async_auth_arg_t *)r->r_arg;

	switch(code)
	{
		//Authentication needed
		case 999:
			{		
				int randomHash;
				char buf[100];
				md5_state_t state;
				md5_byte_t digest[16];

				int di;

				randomHash = strtol(line, NULL, 0);
				
				LOG(PHIDGET_LOG_DEBUG,"Got hash: %d",randomHash);
				
				if(aaa->aaa_pass)
				{
					snprintf(buf,100,"%d%s",randomHash,aaa->aaa_pass);
				}

				md5_init(&state);
				md5_append(&state, (const md5_byte_t *)buf, (int)strlen(buf));
				md5_finish(&state, digest);
				
				memset(buf, 0, sizeof(buf));
				
				snprintf(buf, 5, "997 ");
				
				for (di = 0; di < 16; ++di)
					snprintf((buf+4) + di * 2, 3, "%02x", digest[di]);
					
				buf[strlen(buf)] = '\n';

				//send out the next async command
				async_cmd_callback(pdcs, buf, async_auth_cb, aaa, aaa->aaa_error, aaa->aaa_arg);

				goto exit_pending;
			}
		// Authentication failed
		case 998:		
			LOG(PHIDGET_LOG_INFO,"Authentication Failed - bad password");
			//send out the error event
			if(aaa->aaa_error)
				aaa->aaa_error("Authentication Failed - bad password", aaa->aaa_arg);
			goto exit_done;
		// Authentication is good, or not needed
		case 996:
			//make sure there is a verison - otherwise this is a really old webservice
			{
				char *versionstring = strstr(line, "version=");
				if(!versionstring)
				{
					LOG(PHIDGET_LOG_ERROR,"Trying to connect to an old webservice. Webservice needs to be updated.");
					//send out an error event
					if(aaa->aaa_error)
						aaa->aaa_error("Trying to connect to an old webservice. Webservice needs to be updated.", aaa->aaa_arg);
					goto exit_done;
				}
				//It's good - this is the end of authentication
				else
				{
					//send out auth_finished event
					if(aaa->aaa_success)
						aaa->aaa_success(aaa->aaa_arg, aaa->aaa_error);
				}
			}
			goto exit_done;
		// Version mismatch
		case 994:
			{
				char buf[100];
				snprintf(buf, sizeof(buf), "Version Mismatch; webservice:%s, expecting:%s",strlen(line)>24?(line+24):line,ws_protocol_ver);
				LOG(PHIDGET_LOG_ERROR, buf);
				//send out error event
				if(aaa->aaa_error)
					aaa->aaa_error(buf, aaa->aaa_arg);
			}
			goto exit_done;
		case 500:
			{
				//callback an error
				char buf[100];
				snprintf(buf, sizeof(buf), "Network Error (probably a timeout occured): %s", line);
				LOG(PHIDGET_LOG_ERROR, buf);
				//send out error event
				if(aaa->aaa_error)
					aaa->aaa_error(buf, aaa->aaa_arg);
			}
			goto exit_done;
		default:
			//callback an error
			if(aaa->aaa_error)
				aaa->aaa_error("Unexpected response during authorization. Probably a version mismatch - upgrade the Webservice.", aaa->aaa_arg);
			goto exit_done;
	}

exit_done:
	free(aaa); aaa = NULL;
exit_pending:
	free(r); r = NULL;
}

/*
 * Sends out an async command. Response comes back on the r_notify callback.
 */
static void
async_cmd_callback(pdc_session_t *pdcs, const char *cmd, 
	void (*r_notify)(pdc_session_t *, struct result *, int code,int final, const char *line), void *r_arg,
    void (*error)(const char *errdesc, void *arg), void *arg)
{
	static int tag = 1;
	char errdesc[256];
	result_t *r;
	result_t *or;
	char *buf;
	int len;

	if ((len = pasprintf(&buf, "T%d %s", tag, cmd)) < 0) {
		if(error)
			error(strerror(errno), arg);
		return;
	}
	if (!(r = malloc(sizeof (*r)))) {
		if(error)
			error(strerror(errno), arg);
		return;
	}
	/* add pending */
	pthread_mutex_lock(&pdcs->pdcs_pending_lock);
	r->r_tag = tag++;
	r->r_notify = r_notify;
	r->r_arg = r_arg;
	ptree_replace(r, &pdcs->pdcs_pending, tagcmp, (void **)&or);
	// if or != NULL then pdcs_pending already contained a node with tag == r.r_tag
	assert(!or);

	if (!pdcs->pdcs_write(pdcs->pdcs_wfd, buf, (int)strlen(buf), errdesc,
		(unsigned int)sizeof (errdesc))) {
		if(error)
			error(errdesc, arg);
		ptree_remove(r, &pdcs->pdcs_pending, tagcmp, (void **)&or);
		if(r == or && r)
		{
			if(r->r_arg)
				free(r->r_arg); r->r_arg = NULL;
			free(r); r = NULL;
		}
		pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
		return;
	}

	pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
	free(buf); buf = NULL;
}

/*
 * Sends out an async command. Errors come back on the error callback if desired does not match the response.
 */
static void
async_cmd(pdc_session_t *pdcs, int desired, const char *cmd,
    void (*error)(const char *errdesc, void *arg), void *arg)
{
	async_cmd_arg_t *aca;

	if (!(aca = malloc(sizeof (*aca)))) {
		if(error)
			error(strerror(errno), arg);
		return;
	}

	aca->aca_desired = desired;
	aca->aca_error = error;
	aca->aca_error_arg = arg;

	async_cmd_callback(pdcs, cmd, async_cmd_cb, aca, error, arg);
}

typedef struct _pdc_session_and_arg_t{
	pdc_session_t *pdcs;
	void *r_arg;
} pdc_session_and_arg_t;

static ptree_walk_res_t
clean_pending_async(const void *node, int level, void *arg, void *pwra)
{
	result_t *rk = (result_t *)node;
	pdc_session_and_arg_t *a = (pdc_session_and_arg_t *)arg;
	result_t *r;
	char msg[1024];

	if(rk->r_notify == &async_cmd_cb && ((async_cmd_arg_t *)rk->r_arg)->aca_error_arg == a->r_arg)
	{
		ptree_inorder_walk_remove(&a->pdcs->pdcs_pending, (void **)&r, pwra, tagcmp);
		assert(rk == r);

		snprintf(msg, 1024, "Socket was closed before command T%d finished.",r->r_tag);

		//notify frees r
		if(r->r_notify)
			r->r_notify(a->pdcs, r, 500, PTRUE, msg);
	}

	return PTREE_WALK_CONTINUE;
}

static int
async_pending_cmp(const void *arg, const void *tv)
{
	result_t *rk = (result_t *)tv;

	if(rk->r_notify == &async_cmd_cb && ((async_cmd_arg_t *)rk->r_arg)->aca_error_arg == arg)
		return 0;
	return -1;
}

//If there are pending keys for this arg, then 1st wait up to 1 second for them to finish, then report the errors
void
cleanup_pending(pdc_session_t *pdcs, void *arg)
{
	int timeout = 500; //ms
	pdc_session_and_arg_t a = {pdcs, arg};

	pthread_mutex_lock(&pdcs->pdcs_pending_lock);
	//TODO: Do this differently - can't use a different compare then the one used to add to the list!
	while(ptree_contains(arg, pdcs->pdcs_pending, async_pending_cmp, NULL) && timeout>0)
	{
		pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
		SLEEP(10);
		timeout-=10;
		pthread_mutex_lock(&pdcs->pdcs_pending_lock);
	}
	if(timeout<=0)
	{
		ptree_walk(pdcs->pdcs_pending, PTREE_POSTORDER, clean_pending_async, tagcmp, &a);
	}
	pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
}

//loop so long as there are any pending
void
wait_pending(pdc_session_t *pdcs)
{
	int timeout = 500; //ms

	pthread_mutex_lock(&pdcs->pdcs_pending_lock);
	while(pdcs->pdcs_pending && timeout>0)
	{
		pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
		SLEEP(10);
		timeout-=10;
		pthread_mutex_lock(&pdcs->pdcs_pending_lock);
	}
	pthread_mutex_unlock(&pdcs->pdcs_pending_lock);
}

void CCONV
pdc_async_enable_periodic_reports(pdc_session_t *pdcs, int periodms,
								  void (*error)(const char *errdesc, void *arg),
								  void *arg)
{
	char *buf;
	
	if(!pdcs)
		return;

	if (periodms <= 0) {
		if (error)
			error("invalid period", arg);
		return;
	}
	if (pasprintf(&buf, "report %d report\n", periodms) < 0) {
		if (error)
			error(strerror(errno), arg);
		return;
	}
	async_cmd(pdcs, 200, buf, error, arg);
	free(buf); buf = NULL;
}

void CCONV
pdc_async_disable_periodic_reports(pdc_session_t *pdcs, void (*error)(const char *errdesc, void *arg),
								   void *arg)
{
	if(!pdcs)
		return;
	async_cmd(pdcs, 200, "report 0 report\n", error, arg);
}

void CCONV
pdc_async_set(pdc_session_t *pdcs, const char *key, const char *val,
    int len, int remove_on_close, void (*error)(const char *errdesc, void *arg),
    void *arg)
{
	char *escval;
	char *buf;
	if(!pdcs)
		return;
		
	if(val[0] == '\0')
	{
		if (!escape("\001", len, &escval)) {
			if (error)
				error(strerror(errno), arg);
			return;
		}
	}
	else
	{
		if (!escape(val, len, &escval)) {
			if (error)
				error(strerror(errno), arg);
			return;
		}
	}
	if (pasprintf(&buf, "set %s=\"%s\"%s\n", key, escval,
	    remove_on_close ? " for session" : "") < 0) {
		free((void *)escval); escval = NULL;
		if (error)
			error(strerror(errno), arg);
		return;
	}
	async_cmd(pdcs, 200, buf, error, arg);
	free(buf); buf = NULL;
	free((void *)escval); escval = NULL;
}

void CCONV
pdc_async_ignore(pdc_session_t *pdcs, pdc_listen_id_t id, 
	void (*error)(const char *errdesc, void *arg), void *arg)
{
	char *buf;
	if(!pdcs)
		return;
	
	if (pasprintf(&buf, "ignore lid%d\n", id) < 0) {
		if (error)
			error(strerror(errno), arg);
		return;
	}
	async_cmd(pdcs, 200, buf, error, arg);
	free(buf); buf = NULL;
}

void CCONV
pdc_async_authorize(pdc_session_t *pdcs, const char *version, char *password, 
	void (*success) (void *arg, void (*error)(const char *errdesc, void *arg)),
	void (*error)(const char *errdesc, void *arg), void *arg)
{	
	char *buf;
	async_auth_arg_t *aaa;

	if(!pdcs)
		return;

	if (!(aaa = malloc(sizeof (*aaa)))) {
		if(error)
			error(strerror(errno), arg);
		return;
	}

	aaa->aaa_success = success;
	aaa->aaa_error = error;
	aaa->aaa_arg = arg;
	aaa->aaa_pass = password;
	
	if (pasprintf(&buf, "995 authenticate, version=%s\n", version) < 0) {
		if (error)
			error(strerror(errno), arg);
		return;
	}

	//200 is bogus - I want to get the callback on any response
	async_cmd_callback(pdcs, buf, async_auth_cb, aaa, error, arg);
	free(buf); buf = NULL;
}

void CCONV
pdc_async_remove(pdc_session_t *pdcs, const char *pattern, void (*error)(const char *errdesc, void *arg), void *arg)
{
	char *buf;
	
	if(!pdcs)
		return;
	
	if (pasprintf(&buf, "remove %s\n", pattern) < 0) {
		if (error)
			error(strerror(errno), arg);
		return;
	}
	async_cmd(pdcs, 200, buf, error, arg);
	free(buf); buf = NULL;
}

int CCONV
pdc_quit(pdc_session_t *pdc, char *errdesc, int errlen)
{
	return cmd(pdc, 200, "quit\n", errdesc, errlen);
}

int CCONV
pdc_remove(pdc_session_t *pdcs, const char *pattern, char *errdesc, int errlen)
{
	char *buf;
	int res;
	
	if(!pdcs)
		return 0;

	if (pasprintf(&buf, "remove %s\n", pattern) < 0) {
		if (errdesc)
			(void) snprintf(errdesc, errlen, "%s", strerror(errno));
		return 0;
	}
	res = cmd(pdcs, 200, buf, errdesc, errlen);
	free(buf); buf = NULL;
	return res;
}

int CCONV
pdc_get(pdc_session_t *pdcs, const char *key, char *val, int vallen, char *errdesc, int errlen)
{
	char *buf, *results;
	int res, resultsize;

	if(!pdcs)
		return 0;
	
	resultsize = vallen + 30;

	results = (char *)malloc(resultsize);

	if (pasprintf(&buf, "get %s\n", key) < 0) {
		if (errdesc)
			(void) snprintf(errdesc, errlen, "%s", strerror(errno));
		free(results);
		return 0;
	}
	res = jcmd(pdcs, buf, 200, &results, resultsize, errdesc, errlen);
	if(res)
	{
		char *ueval, *eol, *resultval;
		unsigned int uevlen;

		if ((eol = strchr(results, '\n')))
		{
			eol[0] = '\0';
		}
		if(!(resultval = strstr(results, "value ")))
		{
			//key does not exist
			res = 2;
			val[0] = 0;
			goto end;
		}
		resultval += 6;

		if (!unescape(resultval, &ueval, &uevlen)) {
			/* XXX log */
			res = 0; //error
			val[0] = 0;
			goto end;
		}
		strncpy(val, ueval, vallen-1);
		val[vallen-1]='\0';
		free(ueval);
	}

end:
	free(buf);
	free(results);
	return res;
}

static pdict_reason_t
_pdict_reason_from_str(const char *s)
{
	if (strcmp(s, "changed") == 0)
		return PDR_VALUE_CHANGED;
	if (strcmp(s, "current") == 0)
		return PDR_CURRENT_VALUE;
	if (strcmp(s, "added") == 0)
		return PDR_ENTRY_ADDED;
	if (strcmp(s, "removing") == 0)
		return PDR_ENTRY_REMOVING;
	return 0;
}

int CCONV
pdc_get_server_session_id(pdc_session_t *pdc, int *id, char *errdesc,
    int errlen)
{
	char reply[80];
	char *buf;
	int res;

	if(!pdc)
		return 0;

	if (pasprintf(&buf, "get session id\n") < 0) {
		if (errdesc)
			(void) snprintf(errdesc, errlen, "%s", strerror(errno));
		return 0;
	}
	res = jcmd(pdc, buf, 200, (char **)&reply, sizeof (reply), errdesc, errlen);
	free(buf); buf = NULL;

	if (res && id)
		*id = atoi(reply);
	return res;
}

int CCONV
pdc_readthread_join(pdc_session_t *pdcs, void **status)
{
	int res=0;
	if(pdcs)
	{
		if(pdcs->pdcs_resultreader)
			res = pthread_join(pdcs->pdcs_resultreader, status);
		pdcs->pdcs_resultreader = 0;
	}
	return res;
}

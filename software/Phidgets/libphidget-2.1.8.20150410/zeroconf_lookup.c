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

#include "stdafx.h"
#include "csocket.h"
#include "csocketevents.h"
#include "cphidgetlist.h"
#include "cphidgetmanager.h"
#include "cphidgetdictionary.h"
#include "cphidgetsbc.h"
#include "zeroconf.h"
#include "dns_sd.h"

/*
	This is all taken from mDNSResponderPosix's nss_mdns code
	so we can lookup .local name manually under uClibc (PhidgetSBC)
*/

#ifdef ZEROCONF_LOOKUP

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif

/* runtime linking of dns_sd functions */
#ifdef ZEROCONF_RUNTIME_LINKING
	typedef void (DNSSD_API * DNSServiceRefDeallocateType) (DNSServiceRef);
	typedef DNSServiceErrorType (DNSSD_API * DNSServiceQueryRecordType)
		(DNSServiceRef *, DNSServiceFlags, uint32_t, const char *,
		uint16_t, uint16_t, DNSServiceQueryRecordReply, void *context);
	typedef DNSServiceErrorType (DNSSD_API * DNSServiceProcessResultType) (DNSServiceRef);
	typedef int (DNSSD_API * DNSServiceRefSockFDType) (DNSServiceRef sdRef);

	extern DNSServiceRefDeallocateType DNSServiceRefDeallocatePtr;
	extern DNSServiceQueryRecordType DNSServiceQueryRecordPtr;
	extern DNSServiceProcessResultType DNSServiceProcessResultPtr;
	extern DNSServiceRefSockFDType DNSServiceRefSockFDPtr;
#else
	#define DNSServiceRefDeallocatePtr DNSServiceRefDeallocate
	#define DNSServiceQueryRecordPtr DNSServiceQueryRecord
	#define DNSServiceProcessResultPtr DNSServiceProcessResult
	#define DNSServiceRefSockFDPtr DNSServiceRefSockFD
#endif


//----------
// Structs
//----------

typedef struct
{
	int value;
	const char * name;
	const char * comment;
} table_entry_t;

// Linked list of domains
typedef struct domain_entry
{
	char * domain;
	struct domain_entry * next;
} domain_entry_t;

// Config
typedef struct
{
	domain_entry_t * domains;
} config_t;


//----------
// Constants, Defines
//----------

#define k_hostname_maxlen 255
#define k_aliases_max 15
#define k_addrs_max 15
// Maximum length of a single DNS label
#define DNS_LABEL_MAXLEN 63
// Maximum length of a DNS name
#define DNS_NAME_MAXLEN 255
#define CONF_LINE_SIZE 1024

// Label entries longer than this are actually pointers.
static const int k_label_maxlen = DNS_LABEL_MAXLEN;
// 0 seconds, 500 milliseconds
static const struct timeval k_select_time = { 0, 500000 };
static const char * k_local_suffix = "local";
static const char k_dns_separator = '.';
static const table_entry_t k_table_af [] =
	{
		{ AF_UNSPEC, NULL, NULL },
		{ AF_LOCAL, "LOCAL", NULL },
		{ AF_UNIX, "UNIX", NULL },
		{ AF_INET, "INET", NULL },
		{ AF_INET6, "INET6", NULL }
	};
static const int k_table_af_size = sizeof (k_table_af) / sizeof (* k_table_af);
static const char * k_table_ns_class [] =
	{
		NULL,
		"IN"
	};
static const int k_table_ns_class_size = sizeof (k_table_ns_class) / sizeof (* k_table_ns_class);
static const char * k_table_ns_type [] =
	{
		NULL,
		"A",
		"NS",
		"MD",
		"MF",
		"CNAME",
		"SOA",
		"MB",
		"MG",
		"MR",
		"NULL",
		"WKS",
		"PTR",
		"HINFO",
		"MINFO",
		"MX",
		"TXT",
		"RP",
		"AFSDB",
		"X25",
		"ISDN",
		"RT",
		"NSAP",
		NULL,
		"SIG",
		"KEY",
		"PX",
		"GPOS",
		"AAAA",
		"LOC",
		"NXT",
		"EID",
		"NIMLOC",
		"SRV",
		"ATMA",
		"NAPTR",
		"KX",
		"CERT",
		"A6",
		"DNAME",
		"SINK",
		"OPT"
	};
static const int k_table_ns_type_size = sizeof (k_table_ns_type) / sizeof (* k_table_ns_type);

const char * k_conf_file = "/etc/nss_mdns.conf";
const char k_comment_char = '#';
const char * k_keyword_domain = "domain";
const char * k_default_domains [] =
	{
		"local",
		"254.169.in-addr.arpa",
		"8.e.f.ip6.int",
		"8.e.f.ip6.arpa",
		"9.e.f.ip6.int",
		"9.e.f.ip6.arpa",
		"a.e.f.ip6.int",
		"a.e.f.ip6.arpa",
		"b.e.f.ip6.int",
		"b.e.f.ip6.arpa",
		// Always null terminated
		NULL
	};
const config_t k_empty_config =
	{
		NULL
	};

//----------
// Enums, Structs, Typedefs
//----------

/* Possible results of lookup using a nss_* function.  */
enum nss_status
{
  NSS_STATUS_TRYAGAIN = -2,
  NSS_STATUS_UNAVAIL,
  NSS_STATUS_NOTFOUND,
  NSS_STATUS_SUCCESS,
  NSS_STATUS_RETURN
};

enum
{
	// Format is broken.  Usually because we ran out of data
	// (according to rdata) before the labels said we should.
	DNS_RDATA_TO_NAME_BAD_FORMAT = -1,
	// The converted rdata is longer than the name buffer.
	DNS_RDATA_TO_NAME_TOO_LONG = -2,
	// The rdata contains a pointer.
	DNS_RDATA_TO_NAME_PTR = -3,
};

enum
{
	CMP_DNS_SUFFIX_SUCCESS = 1,
	CMP_DNS_SUFFIX_FAILURE = 0,
	CMP_DNS_SUFFIX_BAD_NAME = 1,
	CMP_DNS_SUFFIX_BAD_DOMAIN = -2
};

typedef enum nss_status nss_status;
typedef struct hostent hostent;
typedef int ns_type_t;
typedef int ns_class_t;
typedef int errcode_t;

typedef void
mdns_lookup_callback_t
(
	DNSServiceRef		sdref,
	DNSServiceFlags		flags,
	uint32_t			interface_index,
	DNSServiceErrorType	error_code,
	const char			*fullname,	  
	uint16_t			rrtype,
	uint16_t			rrclass,
	uint16_t			rdlen,
	const void			*rdata,
	uint32_t			ttl,
	void				*context
);

typedef struct buf_header
{
	char hostname [k_hostname_maxlen + 1];
	char * aliases [k_aliases_max + 1];
	char * addrs [k_addrs_max + 1];
} buf_header_t;

typedef struct result_map
{
	int done;
	nss_status status;
	hostent * hostent;
	buf_header_t * header;
	int aliases_count;
	int addrs_count;
	char * buffer;
	// Index for addresses - grow from low end
	// Index points to first empty space
	int addr_idx;
	// Index for aliases - grow from high end
	// Index points to lowest entry
	int alias_idx;
	int r_errno;
	int r_h_errno;
} result_map_t;

// Context - tracks position in config file, used for error reporting
typedef struct
{
	const char * filename;
	int linenum;
} config_file_context_t;


//----------
// Globals
//----------

static config_t * g_config = NULL;

pthread_mutex_t g_config_mutex =
#ifdef PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
	PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
#else
	PTHREAD_MUTEX_INITIALIZER;
#endif

mdns_lookup_callback_t mdns_lookup_callback;


//----------
// Prototypes
//----------

static int
callback_body_ptr (
	const char * fullname,
	result_map_t * result,
	int rdlen,
	const void * rdata
);

static nss_status
handle_events (DNSServiceRef sdref, result_map_t * result, const char * str);

static nss_status
mdns_lookup_name (
	const char * fullname,
	int af,
	result_map_t * result
);

const char *
ns_class_to_str (ns_class_t in);
const char *
ns_type_to_str (ns_type_t in);
ns_type_t
af_to_rr (int af);

int
dns_rdata_to_name (const char * rdata, int rdlen, char * name, int name_len);
int
cmp_dns_suffix (const char * name, const char * domain);

static char *
add_hostname_or_alias (result_map_t * result, const char * data, int len);
static char *
add_hostname_len (result_map_t * result, const char * fullname, int len);
static char *
add_alias_to_buffer (result_map_t * result, const char * data, int len);
static char *
add_hostname_or_alias (result_map_t * result, const char * data, int len);
static void *
add_address_to_buffer (result_map_t * result, const void * data, int len);
static errcode_t
add_domain (config_t * conf, const char * domain);

static nss_status
set_err_success (result_map_t * result);
static nss_status
set_err_buf_too_small (result_map_t * result);
static nss_status
set_err_internal_resource_full (result_map_t * result);
static nss_status
set_err (result_map_t * result, nss_status status, int err, int herr);
static nss_status
set_err_bad_hostname (result_map_t * result);
static nss_status
set_err_notfound (result_map_t * result);
static nss_status
set_err_mdns_failed (result_map_t * result);
static nss_status
set_err_system (result_map_t * result);

static const char *
is_applicable_name (
	result_map_t * result,
	const char * name,
	char * lookup_name
);
int
islocal (const char * name);

static int
contains_domain_suffix (const config_t * conf, const char * addr);
static char *
contains_alias (result_map_t * result, const char * alias);
static void *
contains_address (result_map_t * result, const void * data, int len);
static int
contains_domain (const config_t * conf, const char * domain);

static int
init_result (
	result_map_t * result,
	hostent * result_buf,
	char * buf,
	size_t buflen
);
errcode_t
init_config ();
static errcode_t
load_config (config_t * conf);
static errcode_t
process_config_line (
	config_t * conf,
	char * line,
	config_file_context_t * context
);
static errcode_t
default_config (config_t * conf);
int
config_is_mdns_suffix (const char * name);

static char *
get_next_word (char * input, char **next);


//Functions
/*
	Get next word (whitespace separated) from input string.
	A null character is written into the first whitespace character following
	the word.
	
	Parameters
		input
			Input string.  This string is modified by get_next_word.
		next
			If non-NULL and the result is non-NULL, a pointer to the
			character following the end of the word (after the null)
			is written to 'next'.
			If no word is found, the original value is unchanged.
			If the word extended to the end of the string, 'next' points
			to the trailling NULL.
			It is safe to pass 'str' as 'input' and '&str' as 'next'.
	Returns
		Pointer to the first non-whitespace character (and thus word) found.
		if no word is found, returns NULL.
 */
static char *
get_next_word (char * input, char **next)
{
	char * curr = input;
	char * result;
	
	while (isspace (*curr))
	{
		curr ++;
	}
	
	if (*curr == 0)
	{
		return NULL;
	}
	
	result = curr;
	while (*curr && ! isspace (*curr))
	{
		curr++;
	}
	if (*curr)
	{
		*curr = 0;
		if (next)
		{
			*next = curr+1;
		}
	}
	else
	{
		if (next)
		{
			*next = curr;
		}
	}
	
	return result;
}
static errcode_t
add_domain (config_t * conf, const char * domain)
{
	if (! contains_domain (conf, domain))
	{
		domain_entry_t * d =
			(domain_entry_t *) malloc (sizeof (domain_entry_t));
		if (! d)
		{
			LOG(PHIDGET_LOG_ERROR,
				"mdns: Can't allocate memory in nss_mdns:init_config, %s:%d",
				__FILE__, __LINE__
			);
			return ENOMEM;
		}

		d->domain = strdup (domain);
		if (! d->domain)
		{
			LOG(PHIDGET_LOG_ERROR,
				"mdns: Can't allocate memory in nss_mdns:init_config, %s:%d",
				__FILE__, __LINE__
			);
			free (d);
			return ENOMEM;
		}
		d->next = conf->domains;
		conf->domains = d;
	}
	
	return 0;
}


static int
contains_domain (const config_t * conf, const char * domain)
{
	const domain_entry_t * curr = conf->domains;
	
	while (curr != NULL)
	{
		if (strcasecmp (curr->domain, domain) == 0)
		{
			return 1;
		}
		
		curr = curr->next;
	}
	
	return 0;
}
static errcode_t
default_config (config_t * conf)
{
	int i;
	for (i = 0; k_default_domains [i]; i++)
	{
		int errcode =
			add_domain (conf, k_default_domains [i]);
		if (errcode)
		{
			// Something has gone (badly) wrong - let's bail
			return errcode;
		}
	}
	
	return 0;
}
/*
	Parse a line of the configuration file.
	For each keyword recognised, perform appropriate handling.
	If the keyword is not recognised, print a message to syslog
	and continue.
	
	Returns
		0 success, or recoverable config file error
		non-zero serious system error, processing aborted
 */
static errcode_t
process_config_line (
	config_t * conf,
	char * line,
	config_file_context_t * context
)
{
	char * curr = line;
	char * word;
	
	word = get_next_word (curr, &curr);
	if (! word || word [0] == k_comment_char)
	{
		// Nothing interesting on this line
		return 0;
	}
	
	if (strcmp (word, k_keyword_domain) == 0)
	{
		word = get_next_word (curr, &curr);
		if (word)
		{
			int errcode = add_domain (conf, word);
			if (errcode)
			{
				// something badly wrong, bail
				return errcode;
			}
			
			if (get_next_word (curr, NULL))
			{
				LOG(PHIDGET_LOG_WARNING,
					"%s, line %d: ignored extra text found after domain",
					context->filename,
					context->linenum
				);
			}
		}
		else
		{
			LOG(PHIDGET_LOG_WARNING,
				"%s, line %d: no domain specified",
				context->filename,
				context->linenum
			);
		}
	}
	else
	{
		LOG(PHIDGET_LOG_WARNING,
			"%s, line %d: unknown keyword %s - skipping",
			context->filename,
			context->linenum,
			word
		);
	}
	
	return 0;
}

int
cmp_dns_suffix (const char * name, const char * domain)
{
	const char * nametail;
	const char * domaintail;

	// Idiot checks
	if (*name == 0 || *name == k_dns_separator)
	{
		// Name can't be empty or start with separator
		return CMP_DNS_SUFFIX_BAD_NAME;
	}
	
	if (*domain == 0)
	{
		return CMP_DNS_SUFFIX_SUCCESS;
			// trivially true
	}
	
	if (*domain == k_dns_separator)
	{
		// drop leading separator from domain
		domain++;
		if (*domain == k_dns_separator)
		{
			return CMP_DNS_SUFFIX_BAD_DOMAIN;
		}
	}

	// Find ends of strings
	for (nametail = name; *nametail; nametail++)
		;
	for (domaintail = domain; *domaintail; domaintail++)
		;
	
	// Shuffle back to last real character, and drop any trailing '.'
	// while we're at it.
	nametail --;
	if (*nametail == k_dns_separator)
	{
		nametail --;
		if (*nametail == k_dns_separator)
		{
			return CMP_DNS_SUFFIX_BAD_NAME;
		}
	}
	domaintail --;
	if (*domaintail == k_dns_separator)
	{
		domaintail --;
		if (*domaintail == k_dns_separator)
		{
			return CMP_DNS_SUFFIX_BAD_DOMAIN;
		}
	}
	
	// Compare.
	while (
		nametail >= name
		&& domaintail >= domain
		&& tolower(*nametail) == tolower(*domaintail))
	{
		nametail--;
		domaintail--;
	}
	
	/* A successful finish will be one of the following:
		(leading and trailing . ignored)
		
		name  :  domain2.domain1
		domain:  domain2.domain1
		        ^
		
		name  : domain3.domain2.domain1
		domain:         domain2.domain1
		               ^
	 */
	if (
		domaintail < domain
		&& (nametail < name || *nametail == k_dns_separator)
	)
	{
		return CMP_DNS_SUFFIX_SUCCESS;
	}
	else
	{
		return CMP_DNS_SUFFIX_FAILURE;
	}
}
static errcode_t
load_config (config_t * conf)
{
	FILE * cf;
	char line [CONF_LINE_SIZE];
	config_file_context_t context;

	context.filename = k_conf_file;
	context.linenum = 0;
	
	
	cf = fopen (context.filename, "r");
	if (! cf)
	{
		LOG(PHIDGET_LOG_INFO,
			"mdns: Couldn't open nss_mdns configuration file %s, using default.",
			context.filename
		);
		return default_config (conf);
	}
	
	while (fgets (line, CONF_LINE_SIZE, cf))
	{
		int errcode;
		context.linenum++;
		errcode = process_config_line (conf, line, &context);
		if (errcode)
		{
			// Critical error, give up
			return errcode;
		}
	}
	
	return 0;
}
/*
	Initialise the configuration from the config file.
	
	Returns
		0 success
		non-zero error code on failure
 */
errcode_t
init_config ()
{
	if (g_config)
	{
		/*
			Safe to test outside mutex.
			If non-zero, initialisation is complete and g_config can be
			safely used read-only.  If zero, then we do proper mutex
			testing before initialisation.
		 */
		return 0;
	}
	else
	{
		int errcode = -1;
		int presult;
		config_t * temp_config;
		
		// Acquire mutex
#ifdef _WINDOWS
		if(!g_config_mutex)
			pthread_mutex_init(&g_config_mutex, 0);
#endif
		presult = pthread_mutex_lock (&g_config_mutex);
		if (presult)
		{
			LOG(PHIDGET_LOG_ERROR,
				"mdns: Fatal mutex lock error in nss_mdns:init_config, %s:%d: %d: %s",
				__FILE__, __LINE__, presult, strerror (presult)
			);
			return presult;
		}
		
		// Test again now we have mutex, in case initialisation occurred while
		// we were waiting
		if (! g_config)
		{
			temp_config = (config_t *) malloc (sizeof (config_t));
			if (temp_config)
			{
				// NOTE: This code will leak memory if initialisation fails
				// repeatedly.  This should only happen in the case of a memory
				// error, so I'm not sure if it's a meaningful problem. - AW
				*temp_config = k_empty_config;
				errcode = load_config (temp_config);
	
				if (! errcode)
				{
					g_config = temp_config;
				}
			}
			else
			{
				LOG(PHIDGET_LOG_ERROR,
					"mdns: Can't allocate memory in nss_mdns:init_config, %s:%d",
					__FILE__, __LINE__
				);
				errcode = errno;
			}
		}
		
		presult = pthread_mutex_unlock (&g_config_mutex);
		if (presult)
		{
			LOG(PHIDGET_LOG_ERROR,
				"mdns: Fatal mutex unlock error in nss_mdns:init_config, %s:%d: %d: %s",
				__FILE__, __LINE__, presult, strerror (presult)
			);
			errcode = presult;
		}

		return errcode;
	}
}
static int
contains_domain_suffix (const config_t * conf, const char * addr)
{
	const domain_entry_t * curr = conf->domains;
	
	while (curr != NULL)
	{
		if (cmp_dns_suffix (addr, curr->domain) > 0)
		{
			return 1;
		}
		
		curr = curr->next;
	}
	
	return 0;
}
int
config_is_mdns_suffix (const char * name)
{
	int errcode = init_config ();
	if (! errcode)
	{
		return contains_domain_suffix (g_config, name);
	}
	else
	{
		errno = errcode;
		return -1;
	}
}
static nss_status
set_err_system (result_map_t * result)
{
	return set_err (result, NSS_STATUS_UNAVAIL, errno, NETDB_INTERNAL);
}
static int
init_result (
	result_map_t * result,
	hostent * result_buf,
	char * buf,
	size_t buflen
)
{
	if (buflen < sizeof (buf_header_t))
	{
		return ERANGE;
	}

	result->hostent = result_buf;
	result->header = (buf_header_t *) buf;
	result->header->hostname[0] = 0;
	result->aliases_count = 0;
	result->header->aliases[0] = NULL;
	result->addrs_count = 0;
	result->header->addrs[0] = NULL;
	result->buffer = buf + sizeof (buf_header_t);
	result->addr_idx = 0;
	result->alias_idx = buflen - sizeof (buf_header_t);
	result->done = 0;
	set_err_notfound (result);

	// Point hostent to the right buffers
	result->hostent->h_name = result->header->hostname;
	result->hostent->h_aliases = result->header->aliases;
	result->hostent->h_addr_list = result->header->addrs;
	
	return 0;
}
/*
	Test whether name is applicable for mdns to process, and if so copy into
	lookup_name buffer (if non-NULL).
	
	Returns
		Pointer to name to lookup up, if applicable, or NULL otherwise.
 */
static const char *
is_applicable_name (
	result_map_t * result,
	const char * name,
	char * lookup_name
)
{
	int match = config_is_mdns_suffix (name);
	if (match > 0)
	{
		if (lookup_name)
		{
			strncpy (lookup_name, name, k_hostname_maxlen + 1);
			return lookup_name;
		}
		else
		{
			return name;
		}
	}
	else
	{
		if (match < 0)
		{
			set_err_system (result);
		}
		return NULL;
	}
}

int
islocal (const char * name)
{
	return cmp_dns_suffix (name, k_local_suffix) > 0;
}

static nss_status
set_err_mdns_failed (result_map_t * result)
{
	return set_err (result, NSS_STATUS_TRYAGAIN, EAGAIN, TRY_AGAIN);
}
static nss_status
set_err_notfound (result_map_t * result)
{
	return set_err (result, NSS_STATUS_NOTFOUND, ENOENT, HOST_NOT_FOUND);
}
static char *
contains_alias (result_map_t * result, const char * alias)
{
	int i;
	
	for (i = 0; result->header->aliases [i]; i++)
	{
		if (strcmp (result->header->aliases [i], alias) == 0)
		{
			return result->header->aliases [i];
		}
	}
	
	return NULL;
}
static nss_status
set_err_bad_hostname (result_map_t * result)
{
	return set_err (result, NSS_STATUS_TRYAGAIN, ENOENT, NO_RECOVERY);
}
/*
	Set the status in the result.
	
	Parameters
		result
			Result structure to update
		status
			New nss_status value
		err
			New errno value
		herr
			New h_errno value
	
	Returns
		New status value
 */
static nss_status
set_err (result_map_t * result, nss_status status, int err, int herr)
{
	result->status = status;
	result->r_errno = err;
	result->r_h_errno = herr;
	
	return status;
}
static void *
contains_address (result_map_t * result, const void * data, int len)
{
	int i;
	
	// Idiot check
	if (len != result->hostent->h_length)
	{
		LOG(PHIDGET_LOG_WARNING,
			"mdns: Unexpected rdata length for address.  Expected %d, got %d",
			result->hostent->h_length,
			len
		);
		// XXX And continue for now.
	}

	for (i = 0; result->header->addrs [i]; i++)
	{
		if (memcmp (result->header->addrs [i], data, len) == 0)
		{
			return result->header->addrs [i];
		}
	}
	
	return NULL;
}
static nss_status
set_err_internal_resource_full (result_map_t * result)
{
	return set_err (result, NSS_STATUS_RETURN, ERANGE, NO_RECOVERY);
}
static nss_status
set_err_buf_too_small (result_map_t * result)
{
	return set_err (result, NSS_STATUS_TRYAGAIN, ERANGE, NETDB_INTERNAL);
}
static nss_status
set_err_success (result_map_t * result)
{
	result->status = NSS_STATUS_SUCCESS;
	return result->status;
}
static int
callback_body_ptr (
	const char * fullname,
	result_map_t * result,
	int rdlen,
	const void * rdata
)
{
	char result_name [k_hostname_maxlen + 1];
	int rv;
	
	// Fullname should be .in-addr.arpa or equivalent, which we're
	// not interested in.  Ignore it.
	
	rv = dns_rdata_to_name (rdata, rdlen, result_name, k_hostname_maxlen);
	if (rv < 0)
	{
		const char * errmsg;
		
		switch (rv)
		{
		  case DNS_RDATA_TO_NAME_BAD_FORMAT:
			errmsg = "mdns: PTR '%s' result badly formatted ('%s...')";
			break;
		
		  case DNS_RDATA_TO_NAME_TOO_LONG:
			errmsg = "mdns: PTR '%s' result too long ('%s...')";
			break;
		
		  case DNS_RDATA_TO_NAME_PTR:
			errmsg = "mdns: PTR '%s' result contained pointer ('%s...')";
			break;
		
		  default:
			errmsg = "mdns: PTR '%s' result conversion failed ('%s...')";
		}

		LOG(PHIDGET_LOG_WARNING,
			errmsg,
			fullname,
			result_name
		);
		
		return -1;
	}
	
	LOG(PHIDGET_LOG_DEBUG,
		"mdns: PTR '%s' resolved to '%s'",
		fullname,
		result_name
	);
	
	// Data should be a hostname
	if (!
		add_hostname_or_alias (
			result,
			result_name,
			rv
		)
	)
	{
		result->done = 1;
		return -1;
	}
	
	return 0;
}



const char *
ns_class_to_str (ns_class_t in)
{
	if (in < k_table_ns_class_size)
		return k_table_ns_class [in];
	else
		return NULL;
}


const char *
ns_type_to_str (ns_type_t in)
{
	if (in < k_table_ns_type_size)
		return k_table_ns_type [in];
	else
		return NULL;
}


ns_type_t
af_to_rr (int af)
{
	switch (af)
	{
	  case AF_INET:
		return kDNSServiceType_A;
	
	  case AF_INET6:
		return kDNSServiceType_AAAA;
	
	  default:
		//return ns_t_invalid;
		return 0;
	}
}
/*
	Add fully qualified name as hostname or alias.
	
	If hostname is not fully qualified this is not an error, but the data
	returned may be not what the application wanted.

	Parameter
		result
			Result structure to write to
		data
			Incoming alias (null terminated)
		len
			Length of data buffer (in bytes), including trailing null
	
	Result
		Pointer to start of newly written data,
		or NULL on error
		If alias or hostname already exists, returns pointer to that instead.
 */
static char *
add_hostname_or_alias (result_map_t * result, const char * data, int len)
{
	char * hostname = result->hostent->h_name;

	if (*hostname)
	{
		if (strcmp (hostname, data) == 0)
		{
			return hostname;
		}
		else
		{
			return add_alias_to_buffer (result, data, len);
		}
	}
	else
	{
		return add_hostname_len (result, data, len);
	}
}

/*
	Add an address to the buffer.
	
	Parameter
		result
			Result structure to write to
		data
			Incoming address data buffer
			Must be 'int' aligned
		len
			Length of data buffer (in bytes)
			Must match data alignment
	
	Result
		Pointer to start of newly written data,
		or NULL on error.
		If address already exists in buffer, returns pointer to that instead.
 */
static void *
add_address_to_buffer (result_map_t * result, const void * data, int len)
{
	int new_addr;
	void * start;
	void * temp;
	
	if ((temp = contains_address (result, data, len)))
	{
		return temp;
	}
	
	if (result->addrs_count >= k_addrs_max)
	{
		// Not enough addr slots
		set_err_internal_resource_full (result);
		LOG(PHIDGET_LOG_ERROR,
			"mdns: Internal address buffer full; increase size"
		);
		return NULL;
	}
	
	// Idiot check
	if (len != result->hostent->h_length)
	{
		LOG(PHIDGET_LOG_WARNING,
			"mdns: Unexpected rdata length for address.  Expected %d, got %d",
			result->hostent->h_length,
			len
		);
		// XXX And continue for now.
	}

	new_addr = result->addr_idx + len;
	
	if (new_addr > result->alias_idx)
	{
		// Not enough room
		set_err_buf_too_small (result);
		LOG(PHIDGET_LOG_DEBUG,
			"mdns: Ran out of buffer when adding address %d",
			result->addrs_count + 1
		);
		return NULL;
	}

	start = result->buffer + result->addr_idx;
	memcpy (start, data, len);
	result->addr_idx = new_addr;
	result->header->addrs [result->addrs_count] = start;
	result->addrs_count ++;
	result->header->addrs [result->addrs_count] = NULL;

	return start;
}

/*
	Wait on result of callback, and process it when it arrives.
	
	Parameters
		sdref
			dns-sd reference
		result
			Initialised 'result' data structure.
		str
			lookup string, used for status/error reporting.
 */
static nss_status
handle_events (DNSServiceRef sdref, result_map_t * result, const char * str)
{
	int dns_sd_fd = DNSServiceRefSockFDPtr(sdref);
	int nfds = dns_sd_fd + 1;
	fd_set readfds;
	struct timeval tv;
	int select_result;

	while (! result->done)
	{
		FD_ZERO(&readfds);
		FD_SET(dns_sd_fd, &readfds);

		tv = k_select_time;
		
		select_result =
			select (nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
		if (select_result > 0)
		{
			if (FD_ISSET(dns_sd_fd, &readfds))
			{
				LOG(PHIDGET_LOG_DEBUG,
					"mdns: Reply received for %s",
					str
				);
				DNSServiceProcessResultPtr(sdref);
			}
			else
			{
				LOG(PHIDGET_LOG_WARNING,
					"mdns: Unexpected return from select on lookup of %s",
					str
				);
			}
		}
		else
		{
			// Terminate loop due to timer expiry
			LOG(PHIDGET_LOG_DEBUG,
				"mdns: %s not found - timer expired",
				str
			);
			set_err_notfound (result);
			break;
		}
	}
	
	return result->status;
}

/*
	Lookup a fully qualified hostname using the default record type
	for the specified address family.
	
	Parameters
		fullname
			Fully qualified hostname.  If not fully qualified the code will
			still 'work', but the lookup is unlikely to succeed.
		af
			Either AF_INET or AF_INET6.  Other families are not supported.
		result
			Initialised 'result' data structure.
 */
static nss_status
mdns_lookup_name (
	const char * fullname,
	int af,
	result_map_t * result
)
{
	// Lookup using mDNS.
	DNSServiceErrorType errcode;
	DNSServiceRef sdref;
	ns_type_t rrtype;
	nss_status status;
	
	LOG(PHIDGET_LOG_DEBUG,
		"mdns: Attempting lookup of %s",
		fullname
	);
	
	switch (af)
	{
	  case AF_INET:
		rrtype = kDNSServiceType_A;
		result->hostent->h_length = 4;
			// Length of an A record
		break;
	
	  case AF_INET6:
		rrtype = kDNSServiceType_AAAA;
		result->hostent->h_length = 16;
			// Length of an AAAA record
		break;
	
	  default:
		LOG(PHIDGET_LOG_WARNING,
			"mdns: Unsupported address family %d",
			af
		);
		return set_err_bad_hostname (result);
	}
	result->hostent->h_addrtype = af;
	
	errcode =
		DNSServiceQueryRecordPtr (
			&sdref,
			kDNSServiceFlagsForceMulticast,		// force multicast query
			kDNSServiceInterfaceIndexAny,	// all interfaces
			fullname,	// full name to query for
			rrtype,		// resource record type
			kDNSServiceClass_IN,	// internet class records
			mdns_lookup_callback,	// callback
			result		// Context - result buffer
		);
	
	if (errcode)
	{
		LOG(PHIDGET_LOG_WARNING,
			"mdns: Failed to initialise lookup, error %d",
			errcode
		);
		return set_err_mdns_failed (result);
	}

	status = handle_events (sdref, result, fullname);
	DNSServiceRefDeallocatePtr (sdref);
	return status;
}

/*
	Examine incoming data and add to relevant fields in result structure.
	This routine is called from DNSServiceProcessResult where appropriate.
 */
void
mdns_lookup_callback
(
	DNSServiceRef		sdref,
	DNSServiceFlags		flags,
	uint32_t			interface_index,
	DNSServiceErrorType	error_code,
	const char			*fullname,	  
	uint16_t			rrtype,
	uint16_t			rrclass,
	uint16_t			rdlen,
	const void			*rdata,
	uint32_t			ttl,
	void				*context
)
{
	// A single record is received

	result_map_t * result = (result_map_t *) context;

	(void)sdref; // Unused
	(void)interface_index; // Unused
	(void)ttl; // Unused
	
	if (! (flags & kDNSServiceFlagsMoreComing) )
	{
		result->done = 1;
	}

	if (error_code == kDNSServiceErr_NoError)
	{
		ns_type_t expected_rr_type =
			af_to_rr (result->hostent->h_addrtype);

		// Idiot check class
		if (rrclass != C_IN)
		{
			LOG(PHIDGET_LOG_WARNING,
				"mdns: Received bad RR class: expected %d (%s),"
				" got %d (%s), RR type %d (%s)",
				C_IN,
				ns_class_to_str (C_IN),
				rrclass,
				ns_class_to_str (rrclass),
				rrtype,
				ns_type_to_str (rrtype)
			);
			return;
		}
		
		// If a PTR
		if (rrtype == kDNSServiceType_PTR)
		{
			if (callback_body_ptr (fullname, result, rdlen, rdata) < 0)
				return;
		}
		else if (rrtype == expected_rr_type)
		{
			if (!
				add_hostname_or_alias (
					result,
					fullname,
					strlen (fullname)
				)
			)
			{
				result->done = 1;
				return;
					// Abort on error
			}

			if (! add_address_to_buffer (result, rdata, rdlen) )
			{
				result->done = 1;
				return;
					// Abort on error
			}
		}
		else
		{
			LOG(PHIDGET_LOG_WARNING,
				"mdns: Received bad RR type: expected %d (%s),"
				" got %d (%s)",
				expected_rr_type,
				ns_type_to_str (expected_rr_type),
				rrtype,
				ns_type_to_str (rrtype)
			);
			return;
		}
		
		if (result->status != NSS_STATUS_SUCCESS)
			set_err_success (result);
	}
	else
	{
		// For now, dump message to syslog and continue
		LOG(PHIDGET_LOG_WARNING,
			"mdns: callback returned error %d",
			error_code
		);
	}
}

int
dns_rdata_to_name (const char * rdata, int rdlen, char * name, int name_len)
{
	int i = 0;
		// Index into 'name'
	const char * rdata_curr = rdata;
	
	// drop any leading whitespace rubbish
	while (isspace (*rdata_curr))
	{
		rdata_curr ++;
		if (rdata_curr > rdata + rdlen)
		{
			return DNS_RDATA_TO_NAME_BAD_FORMAT;
		}
	}
	
	/*
		In RDATA, a DNS name is stored as a series of labels.
		Each label consists of a length octet (max value 63)
		followed by the data for that label.
		The series is terminated with a length 0 octet.
		A length octet beginning with bits 11 is a pointer to
		somewhere else in the payload, but we don't support these
		since we don't have access to the entire payload.
	
		See RFC1034 section 3.1 and RFC1035 section 3.1.
	 */
	while (1)
	{
		int term_len = *rdata_curr;
		rdata_curr++;

		if (term_len == 0)
		{
			break;
				// 0 length record terminates label
		}
		else if (term_len > k_label_maxlen)
		{
			name [i] = 0;
			return DNS_RDATA_TO_NAME_PTR;
		}
		else if (rdata_curr + term_len > rdata + rdlen)
		{
			name [i] = 0;
			return DNS_RDATA_TO_NAME_BAD_FORMAT;
		}
		
		if (name_len < i + term_len + 1)
			// +1 is separator
		{
			name [i] = 0;
			return DNS_RDATA_TO_NAME_TOO_LONG;
		}
		
		memcpy (name + i, rdata_curr, term_len);
		
		i += term_len;
		rdata_curr += term_len;
		
		name [i] = k_dns_separator;
		i++;
	}
	
	name [i] = 0;
	return i;
}

/*
	Add fully qualified hostname to result.
	
	Parameter
		result
			Result structure to write to
		fullname
			Fully qualified hostname
	
	Result
		Pointer to start of hostname buffer,
		or NULL on error (usually hostname too long)
 */

static char *
add_hostname_len (result_map_t * result, const char * fullname, int len)
{
	if (len >= k_hostname_maxlen)
	{
		set_err_bad_hostname (result);
		LOG(PHIDGET_LOG_WARNING,
			"mdns: Hostname too long '%.*s': len %d, max %d",
			len,
			fullname,
			len,
			k_hostname_maxlen
		);
		return NULL;
	}
	
	result->hostent->h_name =
		strcpy (result->header->hostname, fullname);
	
	return result->header->hostname;
}
/*
	Add an alias to the buffer.
	
	Parameter
		result
			Result structure to write to
		data
			Incoming alias (null terminated)
		len
			Length of data buffer (in bytes), including trailing null
	
	Result
		Pointer to start of newly written data,
		or NULL on error
		If alias already exists in buffer, returns pointer to that instead.
 */
static char *
add_alias_to_buffer (result_map_t * result, const char * data, int len)
{
	int new_alias;
	char * start;
	char * temp;
	
	if ((temp = contains_alias (result, data)))
	{
		return temp;
	}
	
	if (result->aliases_count >= k_aliases_max)
	{
		// Not enough alias slots
		set_err_internal_resource_full (result);
		LOG(PHIDGET_LOG_ERROR,
			"mdns: Internal alias buffer full; increase size"
		);
		return NULL;
	}

	new_alias = result->alias_idx - len;
	
	if (new_alias < result->addr_idx)
	{
		// Not enough room
		set_err_buf_too_small (result);
		LOG(PHIDGET_LOG_DEBUG,
			"mdns: Ran out of buffer when adding alias %d",
			result->aliases_count + 1
		);
		return NULL;
	}

	start = result->buffer + new_alias;
	memcpy (start, data, len);
	result->alias_idx = new_alias;
	result->header->aliases [result->aliases_count] = start;
	result->aliases_count ++;
	result->header->aliases [result->aliases_count] = NULL;

	return start;
}



/*
	These are the exported gethostbyname functions
	These will use regular gethostbyname for non .local hostnames,
	and connect to the mdns daemon for looking up .local hostnames.
*/
struct hostent *
mdns_gethostbyname (
	const char *name
)
{
	return mdns_gethostbyname2(name, AF_INET);
}

struct hostent *
mdns_gethostbyname2 (
	const char *name,
	int af
)
{
	hostent *result_buf;
	char buf[1024];
	size_t buflen=1024;
	int errnop;
	int h_errnop;

	char lookup_name [k_hostname_maxlen + 1];
	result_map_t result;
	int err_status;

	if(!islocal(name))
#ifdef _WINDOWS
		return (gethostbyname(name));
#else
		return (gethostbyname2(name, af));
#endif

	result_buf=malloc(sizeof(hostent));

	// Initialize Zeroconf - does run time linking of dns-sd functions
	// if it fails, return null
	if(InitializeZeroconf())
		return NULL;

	// Initialise result
	err_status = init_result (&result, result_buf, buf, buflen);
	if (err_status)
	{
		errnop = err_status;
		h_errnop = NETDB_INTERNAL;
		return NULL;
	}
		
	if (is_applicable_name (&result, name, lookup_name))
	{
		// Try using mdns
		nss_status rv;

		LOG(PHIDGET_LOG_DEBUG,
			"mdns: Local name: %s",
			name
		);

		rv = mdns_lookup_name (name, af, &result);
		if (rv == NSS_STATUS_SUCCESS)
		{
			return result_buf;
		}
	}

	// Return current error status (defaults to NOT_FOUND)
	
	errnop = result.r_errno;
	h_errnop = result.r_h_errno;

	return NULL;
}

#endif

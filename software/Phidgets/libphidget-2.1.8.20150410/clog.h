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

#ifndef _CLOG
#define _CLOG

/** \defgroup phidlog Phidget Logging 
 * Logging is provided mainly for debugging purposes. Enabling logging will output internal library
 * information that can be used to find bugs with the help of Phidgetd Inc. Alternatively, the user
 * can enable and write to the log for their own uses.
 * @{
 */

typedef enum {
	PHIDGET_LOG_CRITICAL = 1,	/**< Really important errors that can't be recovered. Usually followed by an abort() */
	PHIDGET_LOG_ERROR,			/**< Errors that are recovered from. */
	PHIDGET_LOG_WARNING,		/**< Warning's about weird things that aren't neccesarily wrong. */
	PHIDGET_LOG_DEBUG,			/**< Should only be used during development - only shows up in the debug library. */
	PHIDGET_LOG_INFO,			/**< Info about the going on's in the library. */
	PHIDGET_LOG_VERBOSE		/**< Everything, including very common messages. */
} CPhidgetLog_level;

/**
 * Enables logging.
 * @param level The highest level of logging to output. All lower levels will also be output.
 * @param outputFile File to output log to. This should be a full pathname, not a relative pathname. Specify NULL to output to stdout.
 */
PHIDGET21_API int CCONV CPhidget_enableLogging(CPhidgetLog_level level, const char *outputFile);
/**
 * Disables logging.
 */
PHIDGET21_API int CCONV CPhidget_disableLogging();
/**
 * Appends a message to the log.
 * @param level The level at which to log the message.
 * @param id An arbitrary identifier.
 * @param message The message (printf style).
 */
PHIDGET21_API int CCONV CPhidget_log(CPhidgetLog_level level, const char *id, const char *message, ...);

#ifndef EXTERNALPROTO

#define LOG_TO_STDERR 0x8000
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define LOG(level, ...) CPhidget_log(level, __FILE__ "(" TOSTRING(__LINE__) ")", __VA_ARGS__)
#define LOG_STDERR(level, ...) CPhidget_log(LOG_TO_STDERR|level, __FILE__ "(" TOSTRING(__LINE__) ")", __VA_ARGS__)

extern CPhidgetLog_level logging_level;

#endif

/** @} */

#endif

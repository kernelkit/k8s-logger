/* SPDX-License-Identifier: MIT */
#include "config.h"

#ifdef HAVE_SYSLOGP

#include <stddef.h>
#include <syslog/syslog.h>	/* libsyslog from sysklogd project */

void log_open     (const char *ident, int option, int facility);
void log_close    (void);
void logit        (int severity, const char *fmt, ...);
int  log_facility (const char *arg);

#else

#define SYSLOG_NAMES
#include <err.h>
#include <string.h>
#include <syslog.h>

#define log_open  openlog
#define log_close closelog
#define logit     syslog

static inline int log_facility(const char *arg)
{
	for (size_t i = 0; facilitynames[i].c_name; i++) {
		if (strcmp(facilitynames[i].c_name, arg))
			continue;

		return facilitynames[i].c_val;
	}

	warnx("unknown facility '%s', falling back to 'user'", arg);
	return LOG_USER;
}

#endif

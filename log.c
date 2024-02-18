/* SPDX-License-Identifier: MIT */
#include "config.h"

#define SYSLOG_NAMES
#include <syslog/syslog.h>	/* libsyslog from sysklogd project */
#include <libite/lite.h>

#include <err.h>
#include <stdarg.h>
#include <stdlib.h>
#include "log.h"

struct syslog_data log = SYSLOG_DATA_INIT;

/* Dumpster dive in container for process ID */
static void dumpster_dive(const char *ident, int *log_pid)
{
	char buf[512];
	char *docker;
	FILE *fp;

	docker = which("podman");
	if (!docker)
		docker = which("docker");
	if (!docker)
		goto err;

	snprintf(buf, sizeof(buf), "%s inspect %s", docker, ident);
	fp = popen(buf, "r");
	if (fp) {
		int state = 0;

		while (fgets(buf, sizeof(buf), fp)) {
			char *ptr = chomp(buf);

			if (!state) {
				if (strstr(buf, "\"State\":"))
					state = 1;
				continue;
			}

			ptr = strstr(buf, "\"Pid\":");
			if (!ptr)
				continue;
			ptr += 6;
			*log_pid = atoi(ptr);
			break;
		}

		pclose(fp);
		if (*log_pid != -1)
			return;
	}
err:
	*log_pid = getpid();
}

void log_open(const char *ident, int option, int facility)
{
	openlog_r(ident, LOG_PID, facility, &log);
}

void log_close(void)
{
	closelog_r(&log);
}

void logit(int severity, const char *fmt, ...)
{
	va_list ap;

	if (log.log_pid == -1)
		dumpster_dive(ident, &log.log_pid);

	va_start(ap, fmt);
	vsyslogp_r(severity, &log, msgid, NULL, fmt, ap);
	va_end(ap);
}

int log_facility(const char *arg)
{
	for (size_t i = 0; facilitynames[i].c_name; i++) {
		if (strcmp(facilitynames[i].c_name, arg))
			continue;

		return facilitynames[i].c_val;
	}

	warnx("unknown facility '%s', falling back to 'user'", arg);
	return LOG_USER;
}

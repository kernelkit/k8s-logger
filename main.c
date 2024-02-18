/* SPDX-License-Identifier: MIT */
#include "config.h"

#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libite/lite.h>
#include "log.h"

#define TOKEN(ptr) if ((ptr = token(ptr)) == NULL) continue

char *ident = NULL;


static char *token(char *ptr)
{
	if (!ptr)
		return NULL;

	ptr = strpbrk(ptr, " \t");
	if (!ptr)
		return NULL;

	return ++ptr;
}

static int usage(const char *arg0, int code)
{
	printf("Usage:\n"
	       "  %s [OPTIONS] FILE\n"
	       "\n"
	       "Options:\n"
	       "  -f facilty   Log facility name, see syslog.h\n"
	       "  -h           Display this help text and exit\n"
	       "  -i ident     Log identity, e.g., container name\n"
	       "  -n           Run in foreground, do not daemonize\n"
	       "  -v           Dispaly program version and exit\n"
	       "\n"
	       "Arguments:\n"
	       "  FILE         k8s-file log pipe to read from\n"
	       "\n", arg0);

	return code;
}

static int version(int rc)
{
	puts(PACKAGE_STRING);
	printf("Bug report address: %-40s\n", PACKAGE_BUGREPORT);

	return rc;
}

/*
 * Parses k8s-file log format.
 * Syntax:
 *    TIMEFMT [stdout|stderr|NONE|...] [F|P] MESSAGE
 *
 *    F - Full line
 *    P - Partial line, wait for continuation
 *
 * Example:
 *    2024-02-16T15:08:45.594121067+00:00 stdout F Starting syslogd: OK
 *    2024-02-16T15:08:45.594121067+00:00 stdout P Starting ntpd:
 *    2024-02-16T15:08:45.788566992+00:00 stdout F OK
 */
int main(int argc, char *argv[])
{
	char msg[1024] = { 0 }, buf[512];
	int partial, facility = LOG_USER;
	int daemonize = 1;
	FILE *fp;
	int c;

	while ((c = getopt(argc, argv, "f:hi:nv")) != EOF) {
		switch (c) {
		case 'f':
			facility = log_facility(optarg);
			break;
		case 'h':
			return usage(argv[0], 0);
		case 'n':
			daemonize = 0;
			break;
		case 'i':
			ident = optarg;
			break;
		case 'v':
			return version(0);
		default:
			return usage(argv[0], 1);
		}
	}

	if (optind >= argc)
		errx(1, "Missing argument, fifo to read from.");

	fp = fopen(argv[optind], "r");
	if (!fp)
		err(1, "failed opening %s", argv[optind]);

	if (daemonize) {
		if (daemon(0, 0))
			err(1, "failed daemonizing");
	} else {
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		if (chdir("/"))
			warn("failed changing to root directoy");
	}

	log_open(ident, 0, facility);
	while (fgets(buf, sizeof(buf), fp)) {
		int priority = LOG_NOTICE;
		char *ptr = chomp(buf);

		/* skip time, we're forwarding to syslog anyway */
		TOKEN(ptr);

		/* not stdout, either stderr or unknown */
		if (strncmp(ptr, "stdout", 6))
			priority = LOG_ERR;

		/* is it a partial fragment, check also for bad log writers */
		TOKEN(ptr);
		partial = *ptr == 'P' || *ptr == 'p';

		/* finally, the log message */
		TOKEN(ptr);
		if (partial) {
			strlcat(msg, ptr, sizeof(msg));
			continue;
		}

		strlcat(msg, ptr, sizeof(msg));
		logit(priority, "%s", msg);

		/* done prepare for next segment */
		*msg = 0;
	}
	log_close();
	fclose(fp);

	return 0;
}

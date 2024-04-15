/* SPDX-License-Identifier: MIT */
#include "config.h"

#include <err.h>
#include <getopt.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libite/lite.h>
#include "log.h"

#define TOKEN(ptr) if ((ptr = token(ptr)) == NULL) continue

char *ident = NULL;
int running = 1;

static char *token(char *ptr)
{
	if (!ptr)
		return NULL;

	ptr = strpbrk(ptr, " \t");
	if (!ptr)
		return NULL;

	return ++ptr;
}

static void sig(int signo)
{
	syslog(LOG_NOTICE, "got signal %d", signo);
	running = 0;
}

static void sig_init(void)
{
	signal(SIGTERM, sig);
	signal(SIGHUP, sig);
}

static int usage(const char *arg0, int code)
{
	printf("Usage:\n"
	       "  %s [OPTIONS] FILE\n"
	       "\n"
	       "Options:\n"
	       "  -c           Create (and remove) FIFO at startup\n"
	       "  -d           Debug mode\n"
	       "  -f facilty   Log facility name, see syslog.h\n"
	       "  -h           Display this help text and exit\n"
	       "  -i ident     Log identity, e.g., container name\n"
	       "  -n           Run in foreground, do not daemonize\n"
	       "  -p pidfile   Alternative PID file, default /run/$ident.pid"
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
	char msg[1024] = { 0 }, buf[512], *fn, *pidfn = NULL;
	int create = 0, daemonize = 1, facility = LOG_USER;
	int partial, fd, c, opts = LOG_PID;
	struct pollfd pfd;
	FILE *fp;

	while ((c = getopt(argc, argv, "cdf:hi:np:v")) != EOF) {
		switch (c) {
		case 'c':
			create = 1;
			break;
		case 'd':
			opts |= LOG_PERROR;
			break;
		case 'f':
			facility = log_facility(optarg);
			break;
		case 'h':
			return usage(argv[0], 0);
		case 'i':
			ident = optarg;
			break;
		case 'n':
			daemonize = 0;
			break;
		case 'p':
			pidfn = optarg;
			break;
		case 'v':
			return version(0);
		default:
			return usage(argv[0], 1);
		}
	}

	if (optind >= argc)
		errx(1, "Missing argument, fifo to read from.");
	fn = argv[optind];

	if (daemonize) {
		if (daemon(0, 0))
			err(1, "failed daemonizing");
	} else {
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		if (chdir("/"))
			warn("failed changing to root directoy");
	}

	sig_init();
	log_open(ident, opts, facility);

	if (create) {
		/* our responsibilty to create, so remove if exists already */
		if (fexist(fn))
			remove(fn);

		if (mkfifo(fn, 0600) && errno != EEXIST) {
			syslog(LOG_ERR, "failed creating FIFO %s", fn);
			exit(1);
		}
	}

	if (ident && !pidfn) {
		snprintf(buf, sizeof(buf), "/run/%s-%s.pid", PACKAGE_NAME, ident);
		pidfn = buf;
	}

	logit(LOG_INFO, "creating pidfile %s", pidfn ?: "<k8s-logger>");
	if (pidfile(pidfn))
		logit(LOG_ERR, "failed creating pidfile: %s", strerror(errno));

reopen:
	logit(LOG_INFO, "opening fifo %s", fn);
	fd = open(fn, O_RDONLY | O_NONBLOCK);
	if (fd == -1) {
		logit(LOG_ERR, "failed opening %sd: %s", fn, strerror(errno));
		err(1, "failed opening %s", fn);
	}

	fp = fdopen(fd, "r");
	if (!fp) {
		logit(LOG_ERR, "failed opening fifo stream: %s", strerror(errno));
		close(fd);
		log_close();
		exit(1);
	}

	pfd.fd = fd;
	pfd.events = POLLIN;
	pfd.revents = 0;

	logit(LOG_INFO, "entering poll loop ...");
	while (running && poll(&pfd, 1, -1) > 0) {
		int priority = LOG_NOTICE;
		char *ptr;

		if (pfd.revents & POLLHUP) {
			fclose(fp);
			goto reopen;
		}

		if (!fgets(buf, sizeof(buf), fp))
			continue;
		ptr = chomp(buf);

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

	logit(LOG_NOTICE, "shutting down.");
	log_close();
	fclose(fp);
	if (create)
		remove(fn);

	return 0;
}

AM_CFLAGS	    = -W -Wall -Wextra -Wno-unused
DISTCLEANFILES	    = *~ DEADJOE semantic.cache *.gdb *.elf core core.* *.d
dist_doc_DATA	    = README.md LICENSE

bin_PROGRAMS	    = k8s-logger
k8s_logger_SOURCES  = main.c log.h
k8s_logger_CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE
k8s_logger_CFLAGS   = $(syslog_CFLAGS) $(lite_CFLAGS)
k8s_logger_LDADD    = $(syslog_LIBS)   $(lite_LIBS)

if SYSLOGP
k8s_logger_SOURCES += log.c
endif

release: distcheck
	@for file in $(DIST_ARCHIVES); do		\
		md5sum $$file    > ../$$file.md5;	\
		sha256sum $$file > ../$$file.sha256;	\
	done
	@mv $(DIST_ARCHIVES) ../
	@echo
	@echo "Resulting release files in ../"
	@echo "================================================================="
	@for file in $(DIST_ARCHIVES); do					\
		printf "$$file    \tDistribution tarball\n";			\
		printf "$$file.md5\t";    cat ../$$file.md5    | cut -f1 -d' ';	\
		printf "$$file.sha256\t"; cat ../$$file.sha256 | cut -f1 -d' ';	\
	done

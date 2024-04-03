Change Log
==========

All notable changes to the project are documented in this file.


[v1.3][] - 2024-04-03
---------------------

### Changes
 - If started with `-c` and FIFO already exists, it is first removed
 - If started with `-c` the FIFO is removed at program exit

### Fixes
 - Change from blocking `fgets()` to `poll()` to respond to signals.
   This means releases prior to this did not exit on SIGTERM


[v1.2][] - 2024-02-19
---------------------

### Changes
 - New command line option `-c` to create FIFO at startup
 - Add support for signals


[v1.1][] - 2024-02-19
---------------------

### Changes
 - Create `/run/k8s-logger-$ident.pid` for syncing container start

### Fixes
 - Call `fopen()` of fifo *after* daemonzing to prevent blocking


[v1.0][] - 2024-02-18
---------------------

Initial release.

Supports both the plain `syslog()` API [RFC3164][] and extended
[sysklogd][] `syslogp()` [RFC5424][] APIs.  The latter allows
logging container main PID.

[RFC3164]:  https://datatracker.ietf.org/doc/html/rfc3164
[RFC5424]:  https://datatracker.ietf.org/doc/html/rfc5424
[sysklogd]: https://github.com/troglobit/sysklogd

[v1.3]: https://github.com/kernelkit/k8s-logger/compare/v1.2...v1.3
[v1.2]: https://github.com/kernelkit/k8s-logger/compare/v1.1...v1.2
[v1.1]: https://github.com/kernelkit/k8s-logger/compare/v1.0...v1.1

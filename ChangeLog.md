Change Log
==========

All notable changes to the project are documented in this file.


[v1.1][] - 2024-02-18
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

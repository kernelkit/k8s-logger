# k8s-logger

Converts k8s-file logs from podman to `syslog()`.  Useful for systems
that don't have systemd-journald.

Runs best in the background with a FIFO as argument.

## Example k8s-file format

```
2024-02-16T15:08:45.594121067+00:00 stdout F Starting syslogd: OK
2024-02-16T15:08:45.594121067+00:00 stdout P Starting ntpd:
2024-02-16T15:08:45.788566992+00:00 stdout F OK
2024-02-16T15:08:45.791816404+00:00 stdout P Starting dropbear sshd:
2024-02-16T15:08:45.794263331+00:00 stdout F OK
2024-02-16T15:08:45.797041058+00:00 stdout P Starting mini-snmpd:
2024-02-16T15:08:45.799285816+00:00 stdout F OK
```

## Implementation

 1. Opens syslog connection with the given identity and facility
 2. Skips timestamp
 3. Logs everything not `stderr` as `LOG_NOTICE`
 4. Scoops up all partials (`P`) into the same log line
 5. Sends full (`F`) logs to `syslog()`

If the system runs sysklogd, and has libsyslog installed, the logger
will use `syslogp()` instead and go dumpster diving for the PID of the
docker/podman process with a name matching the identity.


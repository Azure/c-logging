# `log_internal_error_report` requirements

`log_internal_error_report` implements reporting an error when the logging framework fails.

The purpose of this module is to offers to the user some signalling that the logging framework failed (for example the number of properties placed in a stack context was too high and there was not other good way to signal back the user).

## Exposed API

```
void log_internal_error_report(void);
```

## log_internal_error_report

```c
void log_internal_error_report(void);
```

`log_internal_error_report` indicates to the user that a critical logging framework error has happened.

**SRS_LOG_INTERNAL_ERROR_01_003: [** On `Windows`, if `LOG_ABORT_ON_ERROR` is defined, `log_internal_error_report` shall call `_set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT)` to disable the abort message. **]**

**SRS_LOG_INTERNAL_ERROR_01_001: [** If `LOG_ABORT_ON_ERROR` is defined, `log_internal_error_report` shall call `abort` the execution. **]**

**SRS_LOG_INTERNAL_ERROR_01_002: [** Otherwise `log_internal_error_report` shall return. **]**

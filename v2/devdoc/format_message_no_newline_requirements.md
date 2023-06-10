# `format_message_no_newline` requirements

`format_message_no_newline` is a wrapper of `FormatMessage` that removes the newline characters placed at the end of the formatted messages.

## Exposed API

```c
DWORD FormatMessageA_no_newline(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments);
```

## FormatMessageA_no_newline

```c
DWORD FormatMessageA_no_newline(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments);
```

`FormatMessageA_no_newline` calls `FormatMessageA` and removes the newline characters from the resulting output.

**SRS_FORMATMESSAGE_NO_NEWLINE_01_001: [** `FormatMessageA_no_newline` shall call `FormatMessageA` with the same arguments. **]**

**SRS_FORMATMESSAGE_NO_NEWLINE_01_002: [** If `FormatMessageA` returns 0, `FormatMessageA_no_newline` shall return 0. **]**

**SRS_FORMATMESSAGE_NO_NEWLINE_01_003: [** Otherwise, `FormatMessageA_no_newline` shall remove any `\r` or `\n` characters that have been placed at the end of the formatted output by `FormatMessageA` and return the number of `CHAR`s left in `lpBuffer`. **]**

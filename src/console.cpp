#include "precompiled.h"

void console_init() {
	FILE *log = nullptr;
	fopen_s(&log, "data/console.log", "w");
	if (log) {
		fclose(log);
	}
}

void console_print(const char *text) {
	puts(text);

	FILE *log = nullptr;
	fopen_s(&log, "data/console.log", "a");
	if (log) {
		fputs(text, log);
		fclose(log);
	}
}
void console_printf(const char *text, ...) {
	va_list argptr;
	char message[2048];

	va_start(argptr, text);
	vsnprintf_s(message, 2048, _TRUNCATE, text, argptr);
	va_end(argptr);

	console_print(message);
}
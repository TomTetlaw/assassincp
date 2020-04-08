#include "precompiled.h"

void console_init() {
	FILE *log = nullptr;
	fopen_s(&log, "data/console.log", "w");
	if (log) {
		fclose(log);
	}
}

void console_print(const char *text) {
	fwrite(text, 1, strlen(text), stdout);

	FILE *log = nullptr;
	fopen_s(&log, "data/console.log", "a");
	if (log) {
		fputs(text, log);
		fclose(log);
	}
}
void console_printf(const char *text, ...) {
	va_list argptr;
	char message[2048]; //@todo: make this better

	va_start(argptr, text);
	vsnprintf_s(message, 2048, _TRUNCATE, text, argptr);
	va_end(argptr);

	console_print(message);
}
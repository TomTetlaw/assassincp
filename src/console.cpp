#include "precompiled.h"

Console console;

void Console::init() {
	FILE *log = nullptr;
	fopen_s(&log, "data/console.log", "w");
	if (log) {
		fclose(log);
	}
}

void Console::printf(const char *text, ...) {
	va_list argptr;
	char message[2048];

	va_start(argptr, text);
	vsnprintf_s(message, 2048, _TRUNCATE, text, argptr);
	va_end(argptr);

	::printf("%s", message);

	FILE *log = nullptr;
	fopen_s(&log, "data/console.log", "a");
	if (log) {
		fwrite(message, 1, strlen(message), log);
		fclose(log);
	}
}
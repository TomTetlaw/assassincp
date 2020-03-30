#include "precompiled.h"

Console console;

void Console::printf(const char *text, ...) {
	va_list argptr;
	char message[2048];

	va_start(argptr, text);
	vsnprintf_s(message, 2048, _TRUNCATE, text, argptr);
	va_end(argptr);

	::printf("%s", message);
}
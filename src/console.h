#ifndef __CONSOLE_H__
#define __CONSOLE_H__

struct Console {
	void printf(const char *text, ...);
};

extern Console console;

#endif
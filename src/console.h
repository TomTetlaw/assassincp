#ifndef __CONSOLE_H__
#define __CONSOLE_H__

void console_init();

// print out to the console.
// these will not put a newline at the end.
void console_print(const char *text);
void console_printf(const char *text, ...);

#endif
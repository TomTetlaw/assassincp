#ifndef __HOTLOAD_H__
#define __HOTLOAD_H__

typedef void(*Hotload_Callback)(const char *filename, void *data);

struct Hotloaded_File {
	const char *filename = nullptr;
	void *data = nullptr;
	int last_write_low = 0;
	int last_write_high = 0;
	Hotload_Callback callback = nullptr;
};

void hotload_init();
void hotload_shutdown();
void hotload_add_file(const char *filename, void *data, Hotload_Callback callback);
void hotload_check_files_non_blocking();

#endif
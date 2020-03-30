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

struct Hotload {
	Array<Hotloaded_File *> files;

	void init();
	void shutdown(); 
	void add_file(const char *filename, void *data, Hotload_Callback callback);
	void check_files_non_blocking();
};

extern Hotload hotload;

#endif
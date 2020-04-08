#ifndef __HOTLOAD_H__
#define __HOTLOAD_H__

// if you're loading a file that should be hotloaded, which is anything that could be edited
// and then reloaded (textures, config files, etc), then you do it like this:
//
// void callback(const char *filename, void *data) {
//     // the file has been changed and must be loaded again
// }
// hotload_add_file("my_file_name", my_data, callback);
//
// textures, config files, and fonts are currently all hotloaded automatically.

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
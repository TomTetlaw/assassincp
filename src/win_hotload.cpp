#include "precompiled.h"
#include <Windows.h>

internal Array<Hotloaded_File *> files;
internal HANDLE dir_change_notification;

void hotload_init() {
	dir_change_notification = FindFirstChangeNotificationA("w:/build/data", TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE); //@todo: check if this works on other machines
}

void hotload_shutdown() {
	FindCloseChangeNotification(dir_change_notification);

	For(files) {
		auto it = files[it_index];
		delete it;
	}
}

void hotload_add_file(const char *filename, void *data, Hotload_Callback callback) {
	Hotloaded_File *file = new Hotloaded_File;
	file->filename = filename;
	file->callback = callback;
	file->data = data;
	files.append(file);

	FILETIME file_time;
	HANDLE file_handle = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	GetFileTime(file_handle, NULL, NULL, &file_time);
	CloseHandle(file_handle);
	file->last_write_low = file_time.dwLowDateTime;
	file->last_write_high = file_time.dwHighDateTime;
}

void hotload_check_files_non_blocking() {
	if (WaitForSingleObject(dir_change_notification, 0) == WAIT_OBJECT_0) {
		For(files) {
			auto it = files[it_index];
			HANDLE file_handle = CreateFileA(it->filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (file_handle == INVALID_HANDLE_VALUE) { // could fail if an editor program is still holding onto the file
				continue;
			}

			FILETIME file_time;
			BOOL success = GetFileTime(file_handle, NULL, NULL, &file_time); // could fail if an editor program is still holding onto the file
			CloseHandle(file_handle);

			if (success) {
				FILETIME last_write_time;
				last_write_time.dwLowDateTime = it->last_write_low;
				last_write_time.dwHighDateTime = it->last_write_high;
				
				int res = CompareFileTime(&file_time, &last_write_time);
				if(res) {
					console_printf("Hotloading %s\n", it->filename);
					it->last_write_low = file_time.dwLowDateTime;
					it->last_write_high = file_time.dwHighDateTime;
					it->callback(it->filename, it->data);
					break;
				}

				FindNextChangeNotification(dir_change_notification);
			}
		}
	}
}
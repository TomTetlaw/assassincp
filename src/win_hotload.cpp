#include "precompiled.h"
#include <Windows.h>

Hotload hotload;
HANDLE dir_change_notification;

void Hotload::init() {
	dir_change_notification = FindFirstChangeNotification("E:/stuff/code/game/build/data/", TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE); //@IncompletePath
}

void Hotload::shutdown() {
	FindCloseChangeNotification(dir_change_notification);

	For(files) {
		delete *it;
	}
}

void Hotload::add_file(const char *filename, void *data, Hotload_Callback callback) {
	Hotloaded_File *file = new Hotloaded_File;
	file->filename = filename;
	file->callback = callback;
	file->data = data;
	files.append(file);

	FILETIME file_time;
	HANDLE file_handle = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	GetFileTime(file_handle, NULL, NULL, &file_time);
	CloseHandle(file_handle);
	file->last_write_low = file_time.dwLowDateTime;
	file->last_write_high = file_time.dwHighDateTime;
}

void Hotload::check_files_non_blocking() {
	if (WaitForSingleObject(dir_change_notification, 0) == WAIT_OBJECT_0) {
		For(files) {
			HANDLE file_handle = CreateFile((*it)->filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (file_handle == INVALID_HANDLE_VALUE) { // could fail if an editor program is still holding onto the file
				continue;
			} 

			FILETIME file_time;
			BOOL success = GetFileTime(file_handle, NULL, NULL, &file_time); // could fail if an editor program is still holding onto the file
			CloseHandle(file_handle);

			if (success) {
				FILETIME last_write_time;
				last_write_time.dwLowDateTime = (*it)->last_write_low;
				last_write_time.dwHighDateTime = (*it)->last_write_high;
				
				int res = CompareFileTime(&file_time, &last_write_time);
				if(res) {
					console.printf("Hotloading %s\n", (*it)->filename);
					(*it)->last_write_low = file_time.dwLowDateTime;
					(*it)->last_write_high = file_time.dwHighDateTime;
					(*it)->callback((*it)->filename, (*it)->data);
					break;
				} 

				FindNextChangeNotification(dir_change_notification);
			}
		}
	}
}
#include "precompiled.h"
#include <Windows.h>
#include <commdlg.h>

void System::open_file_dialogue(const char *dir, const char *filters, dstr *out) {
	OPENFILENAMEA file_name;
	memset(&file_name, 0, sizeof(OPENFILENAMEA));
	file_name.lStructSize = sizeof(OPENFILENAMEA);
	file_name.lpstrInitialDir = dir;
	file_name.lpstrFilter = filters;
	file_name.nFilterIndex = 1;
	file_name.lpstrTitle = "Find file...";
	file_name.nMaxFile = 2048;
	file_name.Flags = OFN_FILEMUSTEXIST;
	file_name.lpstrFile = out->data;
	if (!GetOpenFileNameA(&file_name)) {
		DWORD err2 = GetLastError();
		DWORD err = CommDlgExtendedError();
		LPSTR messageBuffer = nullptr;
		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
		printf("Open file dialogue error: %s ", messageBuffer);
		LocalFree(messageBuffer);
		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, err2, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
		printf("%s\n", messageBuffer);
		LocalFree(messageBuffer);
	}
}
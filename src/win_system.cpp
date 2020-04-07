#include "precompiled.h"
#include <Windows.h>
#include <commdlg.h>

void system_open_file_dialogue(const char *dir, const char *filters, char *out) {
	char buffer[MAX_PATH] = { 0 };

	OPENFILENAMEA file_name;
	memset(&file_name, 0, sizeof(OPENFILENAMEA));
	file_name.lStructSize = sizeof(OPENFILENAMEA);
	file_name.lpstrInitialDir = dir;
	file_name.lpstrFilter = filters;
	file_name.nFilterIndex = 1;
	file_name.lpstrTitle = "Find file...";
	file_name.nMaxFile = MAX_PATH;
	file_name.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	file_name.lpstrFile = buffer;
	if (!GetOpenFileNameA(&file_name)) {
		DWORD err2 = GetLastError();
		DWORD err = CommDlgExtendedError();
		LPSTR messageBuffer = nullptr;
		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
		console_printf("Open file dialogue error: %s ", messageBuffer);
		LocalFree(messageBuffer);
		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, err2, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
		console_printf("%s\n", messageBuffer);
		LocalFree(messageBuffer);
	}

	strcpy(out, buffer);
}
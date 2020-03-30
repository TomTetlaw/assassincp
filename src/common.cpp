#include "precompiled.h"

const char *parse_token(const char *text, char token[MAX_TOKEN_LENGTH])
{
	const char *position = text;
	int n = 0;

	token[0] = 0;

	if (!text) {
		return nullptr;
	}

	if (*position == '\0') {
		token[n] = 0;
		return nullptr;
	}

	while (*position <= 32) {
		if (*position == '\0') {
			return nullptr;
		}
		position++;
	}

	if (*position == '/' && *(position + 1) == '/') {
		while (*position != '\n') {
			if (*position == 0) {
				token[n] = 0;
				return nullptr;
			}
			position++;
		}
	}

	if (*position == '"') {
		position++;
		while (*position != '"') {
			if (*position == 0) {
				token[n] = 0;
				return nullptr;
			}
			token[n] = *position;
			n++;
			position++;
		}
		token[n] = 0;
		position++;
		return position;
	}

	if (*position == '(') {
		position++;
		while (*position != ')') {
			if (*position == 0) {
				token[n] = 0;
				return nullptr;
			}
			token[n] = *position;
			n++;
			position++;
		}
		token[n] = 0;
		position++;
		return position;
	}

	while (*position > 32) {
		token[n] = *position;
		n++;
		position++;

		if (*position == '"' || *position == '(') {
			token[n] = 0;
			return position;
		}
	}

	token[n] = 0;
	return position;
}

Load_File_Result load_file(const char *filename) {
	FILE *f = nullptr;
	Load_File_Result result;

	fopen_s(&f, filename, "rb");

	if (!f) {
		return result;
	}

	int len = 0;
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = new char[len + 1];
	fread_s((void*)buffer, len + 1, len, 1, f);
	buffer[len] = 0;

	fclose(f);

	result.data = buffer;
	result.length = len;
	return result;
}
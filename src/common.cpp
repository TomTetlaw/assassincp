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

	assert(len > 0);

	char *buffer = new char[len + 1];
	fread_s((void*)buffer, len + 1, len, 1, f);
	buffer[len] = 0;

	fclose(f);

	result.data = buffer;
	result.length = len;
	return result;
}

Save_File::~Save_File() {
	if (handle != nullptr) {
		console_printf("Warning: didn't close save file at end of scope!\n");
	}
}

bool save_open_write(const char *file_name, Save_File *file) {
	fopen_s(&file->handle, file_name, "w");
	return file->handle != nullptr;
}

bool save_open_read(const char *file_name, Save_File *file) {
	fopen_s(&file->handle, file_name, "r");
	return file->handle != nullptr;
}

void save_close(Save_File *file) {
	if (!file->handle) {
		return;
	}

	fclose(file->handle);
	file->handle = nullptr;
}

void save_write(Save_File *file, const void *data, int size) {
	if (!file->handle) {
		return;
	}

	fwrite(data, 1, size, file->handle);
}

void save_write_int(Save_File *file, int value) {
	save_write(file, (const void *)&value, sizeof(int));
}

void save_write_uint(Save_File *file, uint value) {
	save_write(file, (const void *)&value, sizeof(uint));
}

void save_write_float(Save_File *file, float value) {
	save_write(file, (const void *)&value, sizeof(float));
}

void save_write_vec2(Save_File *file, Vec2 value) {
	save_write_float(file, value.x);
	save_write_float(file, value.y);
}

void save_write_vec4(Save_File *file, Vec4 value) {
	save_write_float(file, value.x);
	save_write_float(file, value.y);
	save_write_float(file, value.z);
	save_write_float(file, value.w);
}

void save_write_string(Save_File *file, const char *value) {
	int len = strlen(value);
	save_write_int(file, len);
	save_write(file, (const void *)value, len);
}

void save_write_bool(Save_File *file, bool value) {
	save_write_int(file, value ? 1 : 0);
}

void save_read(Save_File *file, void *data, int size) {
	if (!file->handle) {
		return;
	}

	fread(data, 1, size, file->handle);
}

void save_read_int(Save_File *file, int *value) {
	save_read(file, (void *)value, sizeof(int));
}

void save_read_uint(Save_File *file, uint *value) {
	save_read(file, (void *)value, sizeof(uint));
}

void save_read_float(Save_File *file, float *value) {
	save_read(file, (void *)value, sizeof(float));
}

void save_read_vec2(Save_File *file, Vec2 *value) {
	save_read_float(file, &value->x);
	save_read_float(file, &value->y);
}

void save_read_vec4(Save_File *file, Vec4 *value) {
	save_read_float(file, &value->x);
	save_read_float(file, &value->y);
	save_read_float(file, &value->z);
	save_read_float(file, &value->w);
}

void save_read_string(Save_File *file, char *value) {
	int len = 0;
	save_read_int(file, &len);
	save_read(file, (void *)value, len);
}

void save_read_bool(Save_File *file, bool *value) {
	int v = 0;
	save_read_int(file, &v);
	*value = v == 1 ? true : false;
}
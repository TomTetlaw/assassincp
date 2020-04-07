#ifndef __COMMON_H__
#define __COMMON_H__

static const int MAX_TOKEN_LENGTH = 2048;
const char *parse_token(const char *text, char token[MAX_TOKEN_LENGTH]);

struct Load_File_Result {
	char *data = nullptr;
	int length = 0;
};

Load_File_Result load_file(const char *filename);

struct Save_File {
	FILE *handle = nullptr;

	~Save_File();
};

bool save_open_write(const char *file_name, Save_File *file);
bool save_open_read(const char *file_name, Save_File *file);
void save_close(Save_File *file);

void save_write(Save_File *file, const void *data, int size);
void save_write_int(Save_File *file, int value);
void save_write_float(Save_File *file, float value);
void save_write_vec2(Save_File *file, Vec2 value);
void save_write_vec4(Save_File *file, Vec4 value);
void save_write_string(Save_File *file, const char *value);
void save_write_bool(Save_File *file, bool value);

void save_read(Save_File *file, void *data, int size);
void save_read_int(Save_File *file, int *value);
void save_read_float(Save_File *file, float *value);
void save_read_vec2(Save_File *file, Vec2 *value);
void save_read_vec4(Save_File *file, Vec4 *value);
void save_read_string(Save_File *file, char *value);
void save_read_bool(Save_File *file, bool *value);

#endif
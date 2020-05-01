#ifndef __COMMON_H__
#define __COMMON_H__

struct Load_File_Result {
	char *data = nullptr;
	int length = 0;
};

// loads an entire file into a buffer, you must delete[] result.data after using it.
Load_File_Result load_file(const char *filename);

// parses a token into the token buffer and return pointer to the string after the token
// and returns null if the file has ended.
//
// a typical use (using load_file) looks like this:
//
// const char *text = load_file(config.filename).data;
// char token[MAX_TOKEN_LENGTH] = { 0 };
//
// while (true) {
//     text = parse_token(text, token);
//     if (!text) break;
//     do_something_with_token(token);
// }
//
// supports "", (), token will be whatever is inside those characters
// supports // comments

static const int MAX_TOKEN_LENGTH = 2048; //@todo: allow different lengths
const char *parse_token(const char *text, char token[MAX_TOKEN_LENGTH]);

struct Save_File {
	FILE *handle = nullptr;
	uint version = 0;
	// doesn't close the handle, just checks if it is closed and warns if not
	~Save_File();
};

// these functions write data to a file, a typical use looks like this:
//
// float my_int = 10.0f;
// char my_string[1024] = "Hello, world!\n";
//
// Save_File file;
// save_open_write("save_file.sav", &file);
// save_write_int(&file, my_int);
// save_write_int(&file, my_string);
// save_close(&file);
//
// char what_the_string_was[1024] = {0};
// save_open_read("save_file.sav", &file);
// save_read_int(&file, &my_int);
// save_read_int(&file, what_the_string_was);
// save_close(&file);

bool save_open_write(const char *file_name, Save_File *file);
bool save_open_read(const char *file_name, Save_File *file);
void save_close(Save_File *file);

//@todo: figure out how to make these handle little/big endian

void save_write(Save_File *file, const void *data, int size);
void save_write_int(Save_File *file, int value);
void save_write_uint(Save_File *file, uint value);
void save_write_u8(Save_File *file, u8 value);
void save_write_float(Save_File *file, float value);
void save_write_vec2(Save_File *file, Vec2 value);
void save_write_vec4(Save_File *file, Vec4 value);
void save_write_string(Save_File *file, const char *value);
void save_write_bool(Save_File *file, bool value);

void save_read(Save_File *file, void *data, int size);
void save_read_int(Save_File *file, int *value);
void save_read_u8(Save_File *file, u8 *value);
void save_read_uint(Save_File *file, uint *value);
void save_read_float(Save_File *file, float *value);
void save_read_vec2(Save_File *file, Vec2 *value);
void save_read_vec4(Save_File *file, Vec4 *value);
void save_read_string(Save_File *file, char *value);
void save_read_bool(Save_File *file, bool *value);

#endif
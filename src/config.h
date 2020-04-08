#ifndef __CONFIG_H__
#define __CONFIG_H__

// if you want to put a var into the config file (data/config.txt) so that you can easily change it at runtime.
// you can also make a callback that will be called whenever the value changes.
// 
// config.txt will be written every time the game closes which will add the new var to the 
// file if it is not in the file already.
//
// void callback(Config_Var *var) {
//     console_printf("var %s was changed!\n", var->name);
// }
//
// float my_value = 0.0f; // put its default value here which it will take if not found in the file
// register_var("my_value", &my_value, callback);

enum Config_Var_Type {
	VAR_INT,
	VAR_FLOAT,
	VAR_DOUBLE,
	VAR_STRING,
	VAR_BOOL,
	VAR_VEC2,
	VAR_VEC3,
	VAR_VEC4,
};

struct Config_Var;
typedef void (*Config_Var_Callback)(Config_Var *var);

constexpr int var_string_length = 256;

struct Config_Var {
	const char *name = nullptr;
	Config_Var_Type type = VAR_FLOAT;

	union {
		int *int_dest;
		float *float_dest;
		double *double_dest;
		bool *bool_dest;
		Vec2 *vec2_dest;
		Vec4 *vec4_dest;
	};

	char string_data[var_string_length];
	char print_string[var_string_length];

	Config_Var_Callback callback = nullptr;
};

struct Config {
	Array<Config_Var *> vars;
	const char *filename = nullptr;
};

void config_load(const char *filename); // all vars must be registered before calling this
void config_shutdown();
void register_var(const char *name, float *var, Config_Var_Callback callback = nullptr);
void register_var(const char *name, double *var, Config_Var_Callback callback = nullptr);
void register_var(const char *name, char **var, Config_Var_Callback callback = nullptr);
void register_var(const char *name, bool *var, Config_Var_Callback callback = nullptr);
void register_var(const char *name, int *var, Config_Var_Callback callback = nullptr);
void register_var(const char *name, Vec2 *var, Config_Var_Callback callback = nullptr);
void register_var(const char *name, Vec4 *var, Config_Var_Callback callback = nullptr);
void config_write_file(const char *filename);

extern Config config;

#endif
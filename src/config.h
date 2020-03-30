#ifndef __CONFIG_H__
#define __CONFIG_H__

enum Config_Var_Type {
	VAR_INT,
	VAR_FLOAT,
	VAR_STRING,
	VAR_BOOL,
	VAR_VEC2,
	VAR_VEC3,
	VAR_VEC4,
};

struct Config_Var;
typedef void (*Config_Var_Callback)(Config_Var *var);

struct Config_Var {
	const char *name = nullptr;
	Config_Var_Type type = VAR_FLOAT;

	union {
		int *int_dest;
		float *float_dest;
		char **string_dest;
		bool *bool_dest;
		Vec2 *vec2_dest;
		Vec3 *vec3_dest;
		Vec4 *vec4_dest;
	};

	char *string_data = nullptr;
	int string_length = 0;

	char *print_string = nullptr;

	Config_Var_Callback callback = nullptr;
};

struct Config {
	Array<Config_Var *> vars;
	const char *filename = nullptr;

	void init();
	void shutdown();
	void register_var(const char *name, float *var, Config_Var_Callback callback = nullptr);
	void register_var(const char *name, char **var, Config_Var_Callback callback = nullptr);
	void register_var(const char *name, bool *var, Config_Var_Callback callback = nullptr);
	void register_var(const char *name, int *var, Config_Var_Callback callback = nullptr);
	void register_var(const char *name, Vec2 *var, Config_Var_Callback callback = nullptr);
	void register_var(const char *name, Vec3 *var, Config_Var_Callback callback = nullptr);
	void register_var(const char *name, Vec4 *var, Config_Var_Callback callback = nullptr);
	void write_file(const char *filename);
	void set_var_from_string(Config_Var *var, const char *string);
};

void config_load_file(const char *filename, void *data); // all vars must be registered before calling this

extern Config config;

#endif
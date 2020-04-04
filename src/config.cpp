#include "precompiled.h"

Config config;

#define PRINT_STRING_LENGTH 1024

void set_var_string(Config_Var *var) {
	if (!var->print_string) {
		var->print_string = new char[PRINT_STRING_LENGTH];
	}

	switch (var->type) {
	case VAR_FLOAT:
		sprintf_s(var->print_string, PRINT_STRING_LENGTH, "%f", *var->float_dest);
		break;
	case VAR_INT:
		sprintf_s(var->print_string, PRINT_STRING_LENGTH, "%d", *var->int_dest);
		break;
	case VAR_BOOL:
		if (*var->bool_dest) {
			strcpy_s(var->print_string, PRINT_STRING_LENGTH, "true");
		}
		else {
			strcpy_s(var->print_string, PRINT_STRING_LENGTH, "false");
		}
		break;
	case VAR_STRING:
		strncpy_s(var->print_string, PRINT_STRING_LENGTH, var->string_data, PRINT_STRING_LENGTH);
		break;
	case VAR_VEC2:
		sprintf_s(var->print_string, PRINT_STRING_LENGTH, "(%f %f)", var->vec2_dest->x, var->vec2_dest->y);
		break;
	case VAR_VEC3:
		sprintf_s(var->print_string, PRINT_STRING_LENGTH, "(%f %f %f)", var->vec3_dest->x, var->vec3_dest->y, var->vec3_dest->z);
		break;
	case VAR_VEC4:
		sprintf_s(var->print_string, PRINT_STRING_LENGTH, "(%f %f %f %f)", var->vec4_dest->x, var->vec4_dest->y, var->vec4_dest->z, var->vec4_dest->w);
		break;
	}
}

void Config::init() {
}

void add_var(Config_Var *v) {
	set_var_string(v);
	config.vars.append(v);
	console.printf("added var: %s (=%s)\n", v->name, v->print_string);
}

void Config::register_var(const char *name, float *var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_FLOAT;
	v->name = name;
	v->float_dest = var;
	v->callback = callback;
	add_var(v);
}

void Config::register_var(const char *name, char **var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_STRING;
	v->name = name;
	v->string_dest = var;
	v->callback = callback;
	add_var(v);
}

void Config::register_var(const char *name, bool *var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_BOOL;
	v->name = name;
	v->bool_dest = var;
	v->callback = callback;
	add_var(v);
}

void Config::register_var(const char *name, int *var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_INT;
	v->name = name;
	v->int_dest = var;
	v->callback = callback;
	add_var(v);
}

void Config::register_var(const char *name, Vec2 *var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_VEC2;
	v->name = name;
	v->vec2_dest = var;
	v->callback = callback;
	add_var(v);
}

void Config::register_var(const char *name, Vec3 *var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_VEC3;
	v->name = name;
	v->vec3_dest = var;
	v->callback = callback;
	add_var(v);
}

void Config::register_var(const char *name, Vec4 *var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_VEC4;
	v->name = name;
	v->vec4_dest = var;
	v->callback = callback;
	add_var(v);
}

void Config::shutdown() {
	For(vars, {
		if (it->string_data) {
			delete[] it->string_data;
			delete[] it->print_string;
		}
		delete it;
	});
}

void Config::set_var_from_string(Config_Var *var, const char *string) {
	console.printf("setting var %s to %s\n", var->name, string);
	
	switch (var->type) {
	case VAR_FLOAT:
		*var->float_dest = (float)atof(string);
		break;
	case VAR_INT:
		*var->int_dest = atoi(string);
		break;
	case VAR_BOOL:
		if (!strcmp(string, "true")) {
			*var->bool_dest = true;
		}
		else {
			*var->bool_dest = false;
		}
		break;
	case VAR_STRING: {
			int length = (int)strlen(string);
			if (var->string_data) {
				delete[] var->string_data;
			}
			var->string_data = new char[length + 1];
			strcpy_s(var->string_data, length + 1, string);
			var->string_data[length] = 0;
			*var->string_dest = var->string_data;
			var->string_length = length;
		} break;
	case VAR_VEC2:
		sscanf_s(string, "%f %f", &var->vec2_dest->x, &var->vec2_dest->y);
		break;
	case VAR_VEC3:
		sscanf_s(string, "%f %f %f", &var->vec3_dest->x, &var->vec3_dest->y, &var->vec3_dest->z);
		break;
	case VAR_VEC4:
		sscanf_s(string, "%f %f %f %f", &var->vec4_dest->x, &var->vec4_dest->y, &var->vec4_dest->z, &var->vec4_dest->w);
		break;
	}

	set_var_string(var);

	if (var->callback) {
		var->callback(var);
	}
}

void config_load_file(const char *filename, void *data) {
	config.filename = filename;

	const char *text = load_file(config.filename).data;
	char token[MAX_TOKEN_LENGTH] = { 0 };

	while (true) {
		text = parse_token(text, token);
		if (!text) {
			break;
		}

		bool found = false;

		for(int i = 0; i < config.vars.num; i++) {
			Config_Var *var = config.vars[i];
			if (!strcmp(var->name, token)) {
				text = parse_token(text, token);
				config.set_var_from_string(var, token);
				found = true;
				break;
			}
		}
	}

	delete[] text;
}

void Config::write_file(const char *filename) {
	FILE *file = nullptr;
	fopen_s(&file, filename, "w");
	if (!file) {
		return;
	}

	For(vars, {
		switch (it->type) {
		case VAR_FLOAT:
			fprintf_s(file, "%s %f\n", it->name, *it->float_dest);
			break;
		case VAR_INT:
			fprintf_s(file, "%s %d\n", it->name, *it->int_dest);
			break;
		case VAR_BOOL:
			fprintf_s(file, "%s %s\n", it->name, *it->bool_dest ? "true" : "false");
			break;
		case VAR_STRING:
			fprintf_s(file, "%s %s\n", it->name, it->string_data);
			break;
		case VAR_VEC2:
			fprintf_s(file, "%s (%f %f)\n", it->name, it->vec2_dest->x, it->vec2_dest->y);
			break;
		case VAR_VEC3:
			fprintf_s(file, "%s (%f %f %f)\n", it->name, it->vec3_dest->x, it->vec3_dest->y, it->vec3_dest->z);
			break;
		case VAR_VEC4:
			fprintf_s(file, "%s (%f %f %f %f)\n", it->name, it->vec4_dest->x, it->vec4_dest->y, it->vec4_dest->z, it->vec4_dest->w);
			break;
		}
	});

	fclose(file);
}
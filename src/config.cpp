#include "precompiled.h"

internal Array<Config_Var *> vars;
internal const char *file_name = nullptr;

internal void set_var_string(Config_Var *var) {
	switch (var->type) {
	case VAR_FLOAT:
		sprintf_s(var->print_string, var_string_length, "%f", *var->float_dest);
		break;
	case VAR_DOUBLE:
		sprintf_s(var->print_string, var_string_length, "%f", *var->double_dest); // %f is for double... for some reason
		break;
	case VAR_INT:
		sprintf_s(var->print_string, var_string_length, "%d", *var->int_dest);
		break;
	case VAR_BOOL:
		if (*var->bool_dest) {
			strcpy_s(var->print_string, var_string_length, "true");
		}
		else {
			strcpy_s(var->print_string, var_string_length, "false");
		}
		break;
	case VAR_STRING:
		strncpy_s(var->print_string, var_string_length, var->string_data, var_string_length);
		break;
	case VAR_VEC2:
		sprintf_s(var->print_string, var_string_length, "(%f %f)", var->vec2_dest->x, var->vec2_dest->y);
		break;
	case VAR_VEC4:
		sprintf_s(var->print_string, var_string_length, "(%f %f %f %f)", var->vec4_dest->x, var->vec4_dest->y, var->vec4_dest->z, var->vec4_dest->w);
		break;
	default:
		assert(0); // unrecognised type
	}
}

internal void add_var(Config_Var *v) {
	set_var_string(v);
	vars.append(v);
}

void register_var(const char *name, float *var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_FLOAT;
	v->name = name;
	v->float_dest = var;
	v->callback = callback;
	add_var(v);
}

void register_var(const char *name, double *var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_DOUBLE;
	v->name = name;
	v->double_dest = var;
	v->callback = callback;
	add_var(v);	
}

void register_var(const char *name, char **var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_STRING;
	v->name = name;
	if(*var) {
		strcpy_s(v->string_data, var_string_length, *var);
	}
	*var = v->string_data;
	v->callback = callback;
	add_var(v);
}

void register_var(const char *name, bool *var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_BOOL;
	v->name = name;
	v->bool_dest = var;
	v->callback = callback;
	add_var(v);
}

void register_var(const char *name, int *var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_INT;
	v->name = name;
	v->int_dest = var;
	v->callback = callback;
	add_var(v);
}

void register_var(const char *name, Vec2 *var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_VEC2;
	v->name = name;
	v->vec2_dest = var;
	v->callback = callback;
	add_var(v);
}

void register_var(const char *name, Vec4 *var, Config_Var_Callback callback) {
	Config_Var *v = new Config_Var;
	v->type = VAR_VEC4;
	v->name = name;
	v->vec4_dest = var;
	v->callback = callback;
	add_var(v);
}

void config_shutdown() {
	For(vars) {
		auto it = vars[it_index];
		delete it;
	}
}

internal void set_var_from_string(Config_Var *var, const char *string) {
	switch (var->type) {
	case VAR_FLOAT:
		*var->float_dest = (float)atof(string); // atof returns double by default... for some reason
		break;
	case VAR_DOUBLE:
		*var->double_dest = atof(string); // atof returns double by default... for some reason
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
			strcpy_s(var->string_data, var_string_length, string);
		} break;
	case VAR_VEC2:
		sscanf_s(string, "%f %f", &var->vec2_dest->x, &var->vec2_dest->y);
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

void config_load(const char *filename) {
	file_name = filename;

	const char *text = load_file(file_name).data;
	char token[MAX_TOKEN_LENGTH] = { 0 };

	while (true) {
		text = parse_token(text, token);
		if (!text) {
			break;
		}

		bool found = false;

		for(int i = 0; i < vars.num; i++) {
			Config_Var *var = vars[i];
			if (!strcmp(var->name, token)) {
				text = parse_token(text, token);
				set_var_from_string(var, token);
				found = true;
				break;
			}
		}
	}

	delete[] text;
}

void config_write_file(const char *filename) {
	FILE *file = nullptr;
	fopen_s(&file, filename, "w");
	if (!file) {
		return;
	}

	For(vars) {
		auto it = vars[it_index];
		switch (it->type) {
		case VAR_FLOAT:
			fprintf_s(file, "%s %f\n", it->name, *it->float_dest);
			break;
		case VAR_DOUBLE:
			fprintf_s(file, "%s %f\n", it->name, *it->double_dest); // %f is for double... for some reason
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
		case VAR_VEC4:
			fprintf_s(file, "%s (%f %f %f %f)\n", it->name, it->vec4_dest->x, it->vec4_dest->y, it->vec4_dest->z, it->vec4_dest->w);
			break;
		default:
			assert(0); // unrecognised type
		}
	}

	fclose(file);
}
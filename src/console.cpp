#include "precompiled.h"

enum Console_State {
	STATE_OPEN,
	STATE_OPEN_MORE,
	STATE_CLOSED,
};

constexpr int console_line_size = 1024;

struct Console_Line {
	char text[console_line_size] = {0};
	bool from_user = false;
};

internal Array<Console_Line> lines;
internal Array<Console_Line> history;
internal int history_index = 0;

internal Vec4 input_colour = Vec4(1, 1, 1, 1);
internal char input_text[console_line_size] = {0};
internal int cursor = 0;

internal Console_State state = STATE_CLOSED;
internal float box_left = 0.0f;
internal float box_right = 0.0f;
internal float box_top = 0.0f;
internal float box_bottom = 0.0f;

internal float input_left = 0.0f;
internal float input_right = 0.0f;
internal float input_top = 0.0f;
internal float input_bottom = 0.0f;

internal float console_bottom = 27.0f * 9.0f;
internal float console_bottom_more = 88.0f * 9.0f;
internal float console_open_speed = 100.0f;

internal float scroll_y = 0.0f;
internal float scroll_size = 0.0f;
internal float scroll_left = 0.0f;
internal float scroll_right = 0.0f;
internal float scroll_top = 0.0f;
internal float scroll_bottom = 0.0f;
internal bool dragging_scroll = false;
internal bool show_scroll = false;
internal float percent_visible = 0.0f;

internal float line_height = 0.0f;
internal float margin_x = 10.0f;

internal Font *font = nullptr;
internal float char_width = 16.0f;

internal Array<Console_Command> commands;

void cmd_list_commands(Array<Command_Argument> &args) {
	for(int i = 0; i < commands.num; i++) {
		console_printf("%s\n", commands[i].name);
	}
}

void console_init() {
	FILE *log = nullptr;
	fopen_s(&log, "data/console.log", "w");
	if (log) {
		fclose(log);
	}

	register_var("console_bottom", &console_bottom);
	register_var("console_bottom_more", &console_bottom_more);
	register_var("console_open_speed", &console_open_speed);
	register_command("list_commands", cmd_list_commands);

	font = load_font("data/fonts/cascadia.ttf", 16);
	line_height = font->line_skip;
	char_width = font->advance;

	memset(input_text, 0, console_line_size);
}

void console_print(const char *text) {
	FILE *log = nullptr;
	fopen_s(&log, "data/console.log", "a");
	if (log) {
		fputs(text, log);
		fclose(log);
	}

	bool appended = false;

	if(lines.num > 0) {
		int length = strlen(lines[lines.num - 1].text);
		if(lines[lines.num - 1].text[length - 1] != '\n') {
			strcat_s(lines[lines.num -1].text, console_line_size, text);
			appended = true;
		}
	}

	if(!appended) {
		Console_Line line;
		strcpy_s(line.text, console_line_size, text);
		line.from_user = false;
		lines.append(line);
	}

	scroll_y = box_top;
}

void console_printf(const char *text, ...) {
	va_list argptr;
	char message[console_line_size];

	va_start(argptr, text);
	vsnprintf_s(message, console_line_size, _TRUNCATE, text, argptr);
	va_end(argptr);

	console_print(message);
}

void console_update() {
	box_top = 0.0f;
	box_left = 0.0f;
	box_right = screen_width;

	switch(state) {
	case STATE_OPEN:
		box_bottom = approach(box_bottom, console_bottom - line_height, sys.delta_time * console_open_speed);
		break;
	case STATE_OPEN_MORE:
		box_bottom = approach(box_bottom, console_bottom_more - line_height, sys.delta_time * console_open_speed);
		break;
	case STATE_CLOSED:
		box_bottom = approach(box_bottom, 0.0f - line_height, sys.delta_time * console_open_speed);
		break;
	}

	input_top = box_bottom;
	input_left = 0.0f;
	input_right = screen_width;
	input_bottom = box_bottom + line_height;
	
	scroll_size = 32.0f;
	
	if(state == STATE_CLOSED) {
		show_scroll = false;
	} else {
		if(lines.num > 0) {
			percent_visible = 0.0f;
			if(state == STATE_OPEN) {
				percent_visible = console_bottom / (lines.num * line_height);
			} else {
				percent_visible = console_bottom_more / (lines.num * line_height);
			}

			if(percent_visible < 1.0f) {
				show_scroll = true;
			} else {
				show_scroll = false;
			}
		}
		if(show_scroll) {
			scroll_size = box_bottom * percent_visible;
			if(scroll_y > box_bottom - scroll_size) scroll_y = box_bottom - scroll_size;
			if(scroll_y < box_top) scroll_y = box_top;
			scroll_top = box_bottom - scroll_y - scroll_size;
			scroll_left = screen_width - 16.0f;
			scroll_right = screen_width;
			scroll_bottom = box_bottom - scroll_y;
		}
	}
}

void console_render() {
	render_setup_for_ui();
	render_box2(box_top, box_left, box_bottom, box_right, true, Vec4(63.0f / 255.0f, 133.0f / 255.0f, 191.0f / 255.0f, 0.4f));
	render_box2(input_top, input_left, input_bottom, input_right, true, Vec4(3.0f / 255.0f, 148.0f / 255.0f, 252.0f / 255.0f, 0.65f));

	if(show_scroll) {
		render_box2(0.0f, screen_width - 16.0f, box_bottom, screen_width, true, Vec4(52.0f / 255.0f, 232.0f / 255.0f, 235.0f / 255.0f, 0.65f));
		render_box2(scroll_top, scroll_left, scroll_bottom, scroll_right, true, Vec4(52.0f / 255.0f, 232.0f / 255.0f, 235.0f / 255.0f, 1.0f));
	}

	float current_y = box_bottom - line_height;
	int start = 0;
	if(show_scroll) 
		start = (int)((lines.num - (box_bottom / line_height)) * fabs(map_range(scroll_y, box_bottom - scroll_size, box_top, 0.0f, 1.0f)));
	for(int i = lines.num - start - 1; i >= 0; i--) {
		auto it = lines[i];
		Vec4 colour = Vec4(1, 1, 1, 1);
		if(it.from_user) colour = Vec4(0, 1, 0, 1);
		render_string(Vec2(margin_x, current_y), it.text, colour, font);
		current_y -= line_height;
	}

	render_string(Vec2(margin_x, input_top), input_text, input_colour, font);
	render_line(Vec2(margin_x + (cursor * char_width), input_top), Vec2(margin_x + (cursor * char_width), input_bottom));
}

void console_toggle_open() {
	switch(state) {
	case STATE_OPEN:
		state = STATE_OPEN_MORE;
		break;
	case STATE_OPEN_MORE:
		state = STATE_CLOSED;
		break;
	case STATE_CLOSED:
		state = STATE_OPEN;
		break;
	}
}

void process_input() {
	Array<Command_Argument> arguments;

	const char *text = input_text;
	char token[MAX_TOKEN_LENGTH] = { 0 };
	while (true) {
	    text = parse_token(text, token);
	    if (!text) break;

		Command_Argument arg;
		strcpy_s(arg.text, MAX_TOKEN_LENGTH, token);
		arguments.append(arg);
	}

	if(arguments.num > 0) {
		Console_Line line;
		sprintf_s(line.text, console_line_size, "%s\n", input_text);
		line.from_user = true;
		lines.append(line);

		Config_Var *var = config_find_var(arguments[0].text);
		if(var) {
			console_printf("%s = %s\n", var->name, var->print_string);
		}

		Console_Command *command = console_find_command(arguments[0].text);
		if(command) {
			arguments.remove(0);
			command->callback(arguments);
		}
	}
}

internal bool point_intersects_box(Vec2 point, float top, float left, float bottom, float right) {
	if(point.x < left || point.x > right) return false;
	if(point.y < top || point.y > bottom) return false;
	return true;
}

bool console_handle_mouse_press(int mouse_button, bool down, Vec2 position, bool is_double_click) {
	if(state == STATE_CLOSED) return false;
	
	if(mouse_button == SDL_BUTTON_LEFT) {
		if(down) {
			if(point_intersects_box(position, scroll_top, scroll_left, scroll_bottom, scroll_right)) {
				dragging_scroll = true;
				return true;
			}
		} else {
			dragging_scroll = false;
			return true;
		}
	}

	return false;
}

void console_handle_mouse_move(int relx, int rely) {
	if(dragging_scroll) {
		scroll_y -= rely;
	}
}

void console_handle_mouse_wheel(int amount) {
	scroll_y += amount * line_height;
}

bool console_handle_key_press(SDL_Scancode scancode, bool down, uint mods) {
	if(state == STATE_CLOSED) return false;

	if(down) {
		bool handled = false;

		char ch = '\0';
		if(input_translate_scancode(scancode, mods & KEY_MOD_SHIFT, &ch)) {
			char input_copy[console_line_size] = {0};
			memcpy(input_copy, input_text, console_line_size);
			for(int i = 0; i < console_line_size; i++) {
				if(i < cursor) input_text[i] = input_copy[i];
				else if(i > cursor) input_text[i] = input_copy[i-1];
				else if(i == cursor) input_text[i] = ch;
			}

			cursor += 1;
			handled = true;
		} else {
			if(scancode == SDL_SCANCODE_LEFT) {
				cursor -= 1;
				handled = true;
			}
			else if(scancode == SDL_SCANCODE_RIGHT) {
				cursor += 1;
				handled = true;
			}
			else if(scancode == SDL_SCANCODE_ESCAPE) {
				state = STATE_CLOSED;
				handled = true;
			} else if(scancode == SDL_SCANCODE_RETURN) {
				process_input();

				Console_Line line;
				strcpy_s(line.text, console_line_size, input_text);
				line.from_user = true;
				history.append(line);

				memset(input_text, 0, console_line_size);
				history_index = 0;

				handled = true;
			} else if(scancode == SDL_SCANCODE_DELETE) {
				char input_copy[console_line_size] = {0};
				memcpy(input_copy, input_text, console_line_size);
				for(int i = 0; i < console_line_size - 1; i++) {
					if(i < cursor) input_text[i] = input_copy[i];
					else if(i >= cursor) input_text[i] = input_copy[i + 1];
				}
				handled = true;
			} else if(scancode == SDL_SCANCODE_BACKSPACE) {
				if(cursor > 0) {
					char input_copy[console_line_size] = {0};
					memcpy(input_copy, input_text, console_line_size);
					cursor -= 1;
					for(int i = 0; i < console_line_size - 1; i++) {
						if(i < cursor) input_text[i] = input_copy[i];
						else if(i >= cursor) input_text[i] = input_copy[i + 1];
					}
				}
				handled = true;
			} else if(scancode == SDL_SCANCODE_UP) {
				if(history.num > 0) {
					history_index--;
					if(history_index < 0) history_index = history.num - 1;
					strcpy_s(input_text, console_line_size, history[history_index].text);
					cursor = strlen(input_text);
				}
				handled = true;
			} else if(scancode == SDL_SCANCODE_DOWN) {
				if(history.num > 0) {
					strcpy_s(input_text, console_line_size, history[history_index].text);
					cursor = strlen(input_text);
					history_index++;
					if(history_index >= history.num) history_index = 0;
				}
				handled = true;
			}
		}

		if(cursor < 0) cursor = 0;
		if(cursor > strlen(input_text)) cursor = strlen(input_text);
		if(handled) return true;
	}

	return false;
}

void register_command(const char *name, Console_Command_Callback callback) {
	Config_Var *var = config_find_var(name);
	if(var) {
		console_printf("%s already exists as a config var!\n", name);
	}

	for(int i = 0; i < commands.num; i++) {
		if(!strcmp(commands[i].name, name)) {
			console_printf("Command %s already exists!\n", name);
			return;
		}
	}

	Console_Command command = { name, callback };
	commands.append(command);
}

Console_Command *console_find_command(const char *name) {
	for(int i = 0; i < commands.num; i++) {
		if(!strcmp(commands[i].name, name)) {
			return &commands[i];
		}
	}	

	return nullptr;
}
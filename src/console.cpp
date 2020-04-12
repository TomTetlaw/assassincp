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
int history_index = 0;

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

internal float console_bottom = 250.0f;
internal float console_bottom_more = 800.0f;
internal float console_open_speed = 100.0f;

internal float line_height = 0.0f;
internal float margin_x = 10.0f;

internal Font *font = nullptr;
internal float char_width = 16.0f;

struct Console_Command {
	const char *name = nullptr;
	Console_Command_Callback callback = nullptr;
};

internal Array<Console_Command> commands;

void console_init() {
	FILE *log = nullptr;
	fopen_s(&log, "data/console.log", "w");
	if (log) {
		fclose(log);
	}

	register_var("console_bottom", &console_bottom);
	register_var("console_bottom_more", &console_bottom_more);
	register_var("console_open_speed", &console_open_speed);

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

	Console_Line line;
	strcpy(line.text, text);
	line.from_user = false;
	lines.append(line);
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
}

void console_render() {
	render_setup_for_ui();
	render_box2(box_top, box_left, box_bottom, box_right, true, Vec4(63.0f / 255.0f, 133.0f / 255.0f, 191.0f / 255.0f, 0.4f));
	render_box2(input_top, input_left, input_bottom, input_right, true, Vec4(3.0f / 255.0f, 148.0f / 255.0f, 252.0f / 255.0f, 0.65f));

	float current_y = box_bottom - line_height;
	for(int i = lines.num - 1; i >= 0; i--) {
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

bool console_handle_key_press(SDL_Scancode scancode, bool down, bool ctrl_pressed, bool alt_pressed, bool shift_pressed) {
	if(state == STATE_CLOSED) return false;

	if(down) {
		bool handled = false;

		char ch = '\0';
		if(input_translate_scancode(scancode, shift_pressed, &ch)) {
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
				Console_Line line;
				strcpy(line.text, input_text);
				line.from_user = true;
				lines.append(line);
				history.append(line);
				memset(input_text, 0, console_line_size);
				handled = true;
				history_index = 0;
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
					strcpy(input_text, history[history_index].text);
				}
			} else if(scancode == SDL_SCANCODE_DOWN) {
				if(history.num > 0) {
					strcpy(input_text, history[history_index].text);
					history_index++;
					if(history_index >= history.num) history_index = 0;
				}
			}
		}

		if(cursor < 0) cursor = 0;
		if(cursor > strlen(input_text)) cursor = strlen(input_text);
		if(handled) return true;
	}

	return false;
}

void register_command(const char *name, Console_Command_Callback callback) {
	for(int i = 0; i < commands.num; i++) {
		if(!strcmp(commands[i].name, name)) {
			console_printf("Command %s already exists!\n", name);
			return;
		}
	}
	
	Console_Command command = { name, callback };
	commands.append(command);
}
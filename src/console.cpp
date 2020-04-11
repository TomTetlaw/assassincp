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

internal Vec4 input_colour = Vec4(1, 1, 1, 1);
internal char input_text[console_line_size] = "Hello, world!";
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
	char message[2048]; //@todo: make this better

	va_start(argptr, text);
	vsnprintf_s(message, 2048, _TRUNCATE, text, argptr);
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
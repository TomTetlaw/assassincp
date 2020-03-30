#ifndef __UI_H__
#define __UI_H__

struct UI_Panel {
	Vec2 position;
	Vec2 size;
	float border_thickness = 1;
	Vec4 colour;
	Vec4 border_colour;

	UI_Panel *parent = nullptr;
	Array<UI_Panel *> children;

	UI_Panel() {
		size = Vec2(1, 1);
		colour = Vec4(1, 1, 1, 1);
		border_colour = Vec4(1, 1, 1, 1);
	}

	void add_child(UI_Panel *panel);

	virtual void render();
};

struct UI_Text : public UI_Panel {
	const char *string = nullptr;
	Font *font = nullptr;

	UI_Text() {
		colour = Vec4(1, 1, 1, 1);
		font = renderer.default_font;
	}

	void render();
};

struct UI_Image : public UI_Panel {
	const char *file_name = nullptr;
	Texture *texture = nullptr;

	void render();
};

struct UI_Button : public UI_Panel {
	const char *text = nullptr;
	Texture *on_texture = nullptr;
	Texture *off_texture = nullptr;

	bool is_toggle = false;
	bool state = false;

	virtual void on_click() {}
	void render();
};

Vec2 ui_get_absolute_position(UI_Panel *panel);

#endif
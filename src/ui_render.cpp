#include "precompiled.h"

Vec2 ui_get_absolute_position(UI_Panel *panel) {
	UI_Panel *parent = panel->parent;
	Vec2 position = panel->position;

	while (parent) {
		position = position + parent->position;
		parent = parent->parent;
	}

	return position;
}

void UI_Panel::add_child(UI_Panel *panel) {
	panel->parent = this;
	children.append(panel);
}

void UI_Panel::render() {
	Vec2 position = ui_get_absolute_position(this);

	renderer.box(true, position.x + border_thickness, position.y + border_thickness,
		size.x - border_thickness, size.y - border_thickness, colour);

	// left border
	renderer.box(false, position.x, position.y, border_thickness, size.y, border_colour);
	// top border
	renderer.box(false, position.x, position.y, size.x, border_thickness, border_colour);
	// right border
	renderer.box(false, position.x + size.x, position.y, border_thickness, size.y, border_colour);
	// bottom border
	renderer.box(false, position.x, position.y + size.y, size.x, border_thickness, border_colour);

	for (int i = 0; i < children.num; i++) {
		if (children[i]->parent != this) {
			console.printf("UI Error: panel child not referencing parent\n");
		}
		children[i]->render();
	}
}

void UI_Text::render() {
	float wrap = -1.0f;

	if (parent) {
		wrap = parent->size.x - parent->border_thickness;
	}

	// @Cleanup: this is needed because otherwise the UI_Panel::render will render a box over our string in the 
	// same colour, making it invisible. Need seperate colours for these things.
	Vec4 c = colour;
	colour = Vec4(0, 0, 0, 0);
	UI_Panel::render();
	colour = c;
	renderer.string(font, ui_get_absolute_position(this), colour, wrap, string);
}

void UI_Image::render() {
	Vec2 abs_position = ui_get_absolute_position(this);

	if (!texture) {
		texture = tex.load(file_name);
		size = Vec2(texture->width, texture->height);
	}

	Render_Texture rt;
	rt.position = abs_position;
	rt.size = size;
	rt.texture = texture;
	UI_Panel::render();
	renderer.texture(&rt);
}

void UI_Button::render() {
	Vec2 abs_position = ui_get_absolute_position(this);

	Texture *texture = nullptr;
	if (state) {
		texture = on_texture;
	}
	else {
		texture = off_texture;
	}

	if (!texture) {
		return;
	}

	UI_Panel::render();

	Render_Texture rt;
	rt.position = abs_position;
	rt.size = Vec2(texture->width, texture->height);
	rt.texture = texture;
	renderer.texture(&rt);
	
	if (text) {
		renderer.string(nullptr, abs_position, colour, -1, text);
	}
}
#pragma once
#include "raylib.h"
#include <stdio.h>

Font font;

void loadFonts() {
	font = LoadFontEx("resources/timesbd.ttf", 64, 0, 250);
	//SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);
	//printf(GetWorkingDirectory());
	//LoadFileData("arial.ttf", 16);
}

bool uiButton(const char* text, Rectangle bounds, Color primaryColor, Color secondaryColor, Color hoverColor) {
	Color drawColor = primaryColor;
	Color textColor = secondaryColor;
	bool hovered = (GetMouseX() > bounds.x && GetMouseY() > bounds.y && GetMouseX() < bounds.x + bounds.width
		&& GetMouseY() < bounds.y + bounds.height);
	bool pressed = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
	bool clicked = hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

	if (pressed) {
		drawColor = secondaryColor;
		textColor = primaryColor;
	}
	else if (hovered) {
		drawColor = hoverColor;
	}

	DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, drawColor);
	DrawTextEx(font, text, (Vector2) { bounds.x + 2, bounds.y + 2 }, 24, 4, RAYWHITE);

	return clicked;
}

bool uiCounter(const char* title, float* value, float inc, Vector2 pos) {
	char label[16];
	sprintf(label, "%f", *value);

	bool minus = uiButton("-", (Rectangle) { pos.x, pos.y, 32, 32 }, GRAY, BLACK, DARKGRAY);
	DrawText(title, pos.x + 48, pos.y - 8, 16, BLUE);
	DrawText(label, pos.x + 48, pos.y + 8, 16, BLACK);
	bool plus = uiButton("+", (Rectangle) { pos.x + 128, pos.y, 32, 32 }, GRAY, BLACK, DARKGRAY);

	if (minus) {
		(*value) -= inc;
	}
	if (plus) {
		(*value) += inc;
	}

	return minus || plus;
}
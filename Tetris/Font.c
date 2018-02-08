#include "Font.h"
#include <stdlib.h>

inline TextNode* NewTextNode() {
	TextNode *node = (TextNode*)malloc(sizeof(TextNode));
	node->text = AllocText();
	node->next = NULL;

	return node;
}

Text* Font_NewText(Font *font) {
	TextNode *node = font->node;
	if (!node) {
		font->node = NewTextNode();
		return font->node->text;
	}

	while (node->next)
		node = node->next;

	node->next = NewTextNode();
	return node->next->text;
}

void Font_RemoveText(Font *font, const Text *text) {
	TextNode *next, *node = font->node;
	
	if (node) {
		if (node->text == text)
			font->node = NULL;

		while (next = node->next) {
			if (next->text == text) {
				node->next = next->next;
				free(next);
				return;
			}

			node = next;
		}
	}
}

void Font_FreeText(Font *font, Text *text) {
	Font_RemoveText(font, text);
	FreeText(text);
}

void Font_RegenerateText(Font *font) {
	float uv_size = Font_UVSize(font);
	
	for (TextNode *node = font->node; node; node = node->next)
		GenerateTextData(node->text, uv_size);
}

void Font_Render(const Font *font) {
	for (TextNode *node = font->node; node; node = node->next)
		RenderText(node->text);
}

void Font_Free(Font *font) {
	TextNode *node;
	TextNode *next = font->node;
	while (node = next) {
		FreeText(node->text);
		next = node->next;
		free(node);
	}
}

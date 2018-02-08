#pragma once
#include "Text.h"

/*
	Font.h

	Assosiates an openGL texture with certain text in a linked list
*/

typedef struct TextNode_t {
	Text				*text;
	struct TextNode_t	*next;
} TextNode;

typedef struct {
	GLuint			texture;
	unsigned int	texture_size;
	unsigned int	char_size;

	TextNode		*node;
} Font;

//Creates and generates a new Text object and inserts into linked list
Text* Font_NewText(Font*);

//Remove text from linked list
void Font_RemoveText(Font*, const Text*);

//Generates all text in linked list again, call this when char_size or texture_size changes
void Font_RegenerateText(Font*);

//Frees the entire linked list
void Font_Free(Font*);

//Returns the current UV size of the font
inline float Font_UVSize(const Font *font) {
	return (float)font->char_size / (float)font->texture_size;
}

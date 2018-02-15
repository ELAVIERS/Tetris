#pragma once
#include <Windows.h>

WORD VKCodeFromString(const char *key);

void Bind(const char **tokens, unsigned int count);
void BindAxis(const char **tokens, unsigned int count);
void ClearBinds();

void KeyDown(WORD vk);
void KeyUp(WORD vk);

unsigned int BindsGetConfigString(char **string_out);

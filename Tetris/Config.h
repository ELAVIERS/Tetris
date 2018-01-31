#pragma once
#include "Dvar.h"
#include <stdbool.h>

bool RunConfig(const char *filepath);

void AddCvar(HDvar);

void FreeCvars();
void SaveCvars();

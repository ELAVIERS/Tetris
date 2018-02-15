#pragma once
#include "Dvar.h"
#include <stdbool.h>

bool RunConfig(const char *filepath);

void AddCvar(const Dvar*);

void FreeCvars();
void SaveCvars();

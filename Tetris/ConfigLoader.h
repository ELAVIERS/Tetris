#pragma once
#include <GL/glew.h>

typedef struct {
	char **tokens;
	unsigned int count;
} Statement;

void FreeStatements(Statement*, unsigned int count);

typedef struct {
	char *name;
	char *path;
	Statement *statements;
	unsigned int count;
} Configuration;

void FreeConfigurations(Configuration*, unsigned int count);

unsigned int LoadConfigFile(const char *filepath, Statement **out_statements);
unsigned int LoadConfigMulti(const char *directory, const char *filter, Configuration **out_statements);

unsigned int LoadConfigsFromDir(const char *directory, const char *filter, Configuration **out_configs);

void RunConfig(Configuration config);

#include "Dvar.h"

#include "Console.h"
#include "String.h"
#include <stdlib.h>
#include <string.h>

typedef struct Dvar_s {
	char *name;
	DvarType type;
	DvarValue data;
	DvarCallbackPtr callback;
} Dvar;

typedef struct DvarNode_s {
	char key;
	
	struct DvarNode_s *child;
	struct DvarNode_s *next;

	Dvar *dvar;
} DvarNode;

DvarNode* NewDvarNode(Dvar *dvar, unsigned int depth) {
	if (dvar->name[depth] == '\0')
		return NULL;

	DvarNode *node = (DvarNode*)malloc(sizeof(DvarNode));
	node->key = dvar->name[depth];
	node->next = NULL;
	node->child = NewDvarNode(dvar, depth + 1);

	if (!node->child)
		node->dvar = dvar;
	else
		node->dvar = NULL;

	return node;
}

void AddDvarNode(DvarNode* node, Dvar *dvar, unsigned int depth) {
	if (dvar->name[depth] == '\0')
		return;

	while (1) {
		if (dvar->name[depth] == node->key) {
			if (node->child) {
				AddDvarNode(node->child, dvar, depth + 1);
				return;
			}
			
			node->child = NewDvarNode(dvar, depth + 1);
			return;
		}

		if (!node->next) {
			node->next = NewDvarNode(dvar, depth);
			return;
		}

		if (dvar->name[depth] < node->next->key) {
			DvarNode* next_old = node->next;
			node->next = NewDvarNode(dvar, depth);
			node->next->next = next_old;
			return;
		}

		node = node->next;
	}
}

DvarNode* FindDvarNode(DvarNode *node, const char *name, unsigned int depth) {
	for (; node; node = node->next)
		if (node->key == name[depth]) {
			++depth;

			if (name[depth] != '\0')
				return FindDvarNode(node->child, name, depth);

			return node;
		}

	return NULL;
};

void FreeNode(DvarNode *node) {
	if (!node)
		return;

	DvarNode *next;
	do {
		next = node->next;

		FreeNode(node->child);

		if (node->dvar) {
			free(node->dvar->name);

			if (node->dvar->type == DVT_STRING)
				free(node->dvar->data.dstring);

			free(node->dvar);
		}

		free(node);

		node = next;
	} while (next);
}

void ListNodes(DvarNode *node) {
	for (; node; node = node->next) {
		if (node->dvar) {
			switch (node->dvar->type) {
			case DVT_FLOAT:
				ConsolePrint("FLOAT\t");
				break;
			case DVT_FUNCTION:
				ConsolePrint("FUNC\t");
				break;
			case DVT_STRING:
				ConsolePrint("STRING\t");
				break;
			}

			ConsolePrint(node->dvar->name);
			ConsolePrint("\n");
		}

		if (node->child)
			ListNodes(node->child);
	}
}

////////////////

void PrintValue(Dvar* dvar) {
	ConsolePrint(dvar->name);
	ConsolePrint(" = ");

	switch (dvar->type) {
	case DVT_FLOAT:
	{
		char *str = FloatToString(dvar->data.dfloat);
		ConsolePrint(str);
		free(str);
	}
	break;
	case DVT_STRING:
		ConsolePrint("\"");
		ConsolePrint(dvar->data.dstring);
		ConsolePrint("\"");
		break;
	}

	ConsolePrint("\n");
}

////////////////

DvarNode *dvar_root = NULL;

inline Dvar* FindDvar(const char *name) {
	DvarNode *node = FindDvarNode(dvar_root, name, 0);
	if (!node)
		return NULL;

	return node->dvar;
}

void AddDvarC(const char *name, DvarType type, DvarValue value, DvarCallbackPtr callback) {
	if (dvar_root && FindDvar(name))
		return;

	Dvar *dvar = (Dvar*)malloc(sizeof(Dvar));
	dvar->name = DupString(name);
	dvar->type = type;
	dvar->callback = callback;
	
	if (type == DVT_STRING)
		dvar->data.dstring = DupString(value.dstring);
	else
		dvar->data = value;


	if (dvar_root)
		AddDvarNode(dvar_root, dvar, 0);
	else
		dvar_root = NewDvarNode(dvar, 0);
}

void SetDvar(HDvar hdvar, DvarValue value) {
	Dvar *dvar = (Dvar*)hdvar;

	if (dvar->type == DVT_STRING) {
		free(dvar->data.dstring);
		dvar->data.dstring = DupString(value.dstring);
	}
	else
		dvar->data = value;

	if (dvar->callback)
		dvar->callback(dvar->data);

	PrintValue(dvar);
}

HDvar GetDvar(const char *name) {
	return FindDvar(name);
}

const char* HDvarName(HDvar hdvar) {
	return ((Dvar*)hdvar)->name;
}

char* HDvarValueAsString(HDvar hdvar) {
	Dvar *dvar = (Dvar*)hdvar;

	switch (dvar->type) {
	case DVT_STRING:
		return DupString(dvar->data.dstring);
	case DVT_FLOAT:
		return FloatToString(dvar->data.dfloat);
	}

	return NULL;
}

void ListDvars() {
	if (dvar_root)
		ListNodes(dvar_root);
}

void FreeDvars() {
	FreeNode(dvar_root);
}

////////////////

void HandleCommand(const char **tokens, unsigned int count) {
	if (count == 0)
		return;

	Dvar *dvar = FindDvar(tokens[0]);
	if (!dvar) {
		ConsolePrint(tokens[0]);
		ConsolePrint(" is not a variable\n");
		return;
	}

	if (dvar->type == DVT_FUNCTION) {
		ConsolePrint(dvar->name);
		ConsolePrint("(");
		for (unsigned int i = 0; i < count - 1; ++i) {
			ConsolePrint(tokens[i + 1]);
			if (i < count - 2)
				ConsolePrint(", ");
		}
		ConsolePrint(")\n");

		dvar->data.dfunc(tokens + 1, count - 1);
		return;
	}

	if (count > 1) {
		switch (dvar->type) {
		case DVT_FLOAT:
			dvar->data.dfloat = (DFloat)atof(tokens[1]);
			break;
		case DVT_STRING:
			free(dvar->data.dstring);
			dvar->data.dstring = DupString(tokens[1]);
			break;
		}
	}

	PrintValue(dvar);

	if (count > 1 && dvar->callback)
		dvar->callback(dvar->data);
}

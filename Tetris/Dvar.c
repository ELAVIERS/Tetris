#include "Dvar.h"
#include "Console.h"
#include "String.h"
#include <stdlib.h>
#include <string.h>

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

	if (dvar->name[depth] < node->key) {
		DvarNode *copy = (DvarNode*)malloc(sizeof(DvarNode));
		memcpy_s(copy, sizeof(DvarNode), node, sizeof(DvarNode));

		node->key = dvar->name[depth];
		node->next = copy;
		node->dvar = NULL;
		node->child = NewDvarNode(dvar, depth + 1);
		return;
	}

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
			DvarNode *next_old = node->next;
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
				free(node->dvar->value.string);

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
			case DVT_CALL:
				ConsolePrint("CALL\t");
				break;
			case DVT_FUNCTION:
				ConsolePrint("FUNC\t");
				break;
			case DVT_FLOAT:
				ConsolePrint("FLOAT\t");
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
		char *str = AllocStringFromFloat(dvar->value.number);
		ConsolePrint(str);
		free(str);
	}
	break;
	case DVT_STRING:
		ConsolePrint("\"");
		ConsolePrint(dvar->value.string);
		ConsolePrint("\"");
		break;
	}

	ConsolePrint("\n");
}

////////////////

DvarNode *dvar_root = NULL;

Dvar* GetDvar(const char *name) {
	DvarNode *node = FindDvarNode(dvar_root, name, 0);
	if (!node)
		return NULL;

	return node->dvar;
}

Dvar* AddDvarC(const char *name, DvarType type, DvarValue value, DvarCallback *callback) {
	if (dvar_root && GetDvar(name))
		return NULL;

	Dvar *dvar = (Dvar*)malloc(sizeof(Dvar));
	dvar->name = DupString(name);
	dvar->type = type;
	dvar->callback = callback;
	
	if (type == DVT_STRING)
		dvar->value.string = DupString(value.string);
	else
		dvar->value = value;


	if (dvar_root)
		AddDvarNode(dvar_root, dvar, 0);
	else
		dvar_root = NewDvarNode(dvar, 0);

	return dvar;
}

void SetDvar(Dvar *dvar, DvarValue value) {
	if (dvar->type == DVT_STRING) {
		free(dvar->value.string);
		dvar->value.string = DupString(value.string);
	}
	else
		dvar->value = value;

	if (dvar->callback)
		dvar->callback(dvar->value);

	PrintValue(dvar);
}

char* DvarAllocValueString(const Dvar  *dvar) {
	switch (dvar->type) {
	case DVT_STRING:
		return DupString(dvar->value.string);
	case DVT_FLOAT:
		return AllocStringFromFloat(dvar->value.number);
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

void DvarCommand(Dvar *dvar, const char **tokens, unsigned int count) {
	if (dvar->type == DVT_CALL) {
		ConsolePrint(dvar->name);
		ConsolePrint("()\n");
		dvar->value.call();
		return;
	}

	if (count > 0) {
		switch (dvar->type) {
		case DVT_FUNCTION:
			ConsolePrint(dvar->name);
			ConsolePrint("(");
			for (unsigned int i = 0; i < count; ++i) {
				ConsolePrint(tokens[i]);
				if (i < count - 1)
					ConsolePrint(", ");
			}
			ConsolePrint(")\n");

			dvar->value.function(tokens, count);
			return;
		case DVT_FLOAT:
			dvar->value.number = (float)atof(tokens[0]);
			break;
		case DVT_STRING:
			free(dvar->value.string);
			dvar->value.string = DupString(tokens[0]);
			break;
		}
	}

	PrintValue(dvar);

	if (count > 0 && dvar->callback)
		dvar->callback(dvar->value);
}

void HandleCommand(const char **tokens, unsigned int count) {
	if (count == 0) return;

	Dvar *dvar = GetDvar(tokens[0]);
	if (!dvar) {
		ConsolePrint(tokens[0]);
		ConsolePrint(" is not a variable\n");
	}
	else DvarCommand(dvar, tokens + 1, count - 1);
}

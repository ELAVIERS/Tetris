#include "Dvar.h"

#include "String.h"
#include <stdlib.h>
#include <string.h>

typedef struct DvarNode_s {
	char key;
	
	struct DvarNode_s *child;
	struct DvarNode_s *next;

	Dvar *dvar;
} DvarNode;

DvarNode* NewDvarNode(const Dvar *dvar, unsigned int depth) {
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

bool AddDvarNode(DvarNode* node, const Dvar *dvar, unsigned int depth) {
	if (dvar->name[depth] == '\0')
		return false;

	while (1) {
		if (node->key == dvar->name[depth])
			if (node->child)
				return AddDvarNode(node->child, dvar, depth + 1);
			else {
				node->child = NewDvarNode(dvar, depth + 1);
				return true;
			}

		if (!node->next) {
			node->next = NewDvarNode(dvar, depth);
			return true;
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
				free(node->dvar->data.string);

			free(node->dvar);
		}
		
		free(node);

		node = next;
	} while (next);
}

////////////////

DvarNode *dvar_root = NULL;

void _AddDvar(const char *name, DvarType type, DvarValue data) {
	Dvar *new_dvar = (Dvar*)malloc(sizeof(Dvar));
	new_dvar->type = type;

	new_dvar->name = DupString(name);

	if (type == DVT_STRING)
		new_dvar->data.string = DupString(data.string);
	else
		new_dvar->data = data;

	if (!dvar_root)
		dvar_root = NewDvarNode(new_dvar, 0);
	else
		AddDvarNode(dvar_root, new_dvar, 0);
}

/*
	NOTE: name should always point to constant memory
*/
void* _GetDvar(const char *name, DvarType type) {
	DvarNode *node = FindDvarNode(dvar_root, name, 0);

	if (!node) {
		DvarValue data;
		switch (type) {
		case DVT_FUNCTION: data.function = NULL; break;
		case DVT_FLOAT: data.number = 0.f; break;
		case DVT_STRING:
			data.string = (DString)malloc(1);
			data.string[0] = '\0';
			break;
		}

		_AddDvar(name, type, data);

		return &data;
	}
	else if (node->dvar->type != type)
		return NULL;

	return &node->dvar->data;
}

void FreeDvars() {
	FreeNode(dvar_root);
}

////////////////

void HandleCommand(const char **tokens, unsigned int count) {
	DvarNode *node = FindDvarNode(dvar_root, tokens[0], 0);
	if (!node) return;

	if (count == 1 && node->dvar->type != DVT_FUNCTION) {

		return;
	}

	switch (node->dvar->type) {
	case DVT_FUNCTION:
		node->dvar->data.function(tokens + 1, count - 1);
		break;
	case DVT_FLOAT:
		node->dvar->data.number = atof(tokens[1]);
		break;
	case DVT_STRING:
		free(node->dvar->data.string);
		node->dvar->data.string = DupString(tokens[1]);
		break;
	} 
}

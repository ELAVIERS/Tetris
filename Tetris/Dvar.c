#include "Dvar.h"
#include "Console.h"
#include "Messaging.h"
#include "Server.h"
#include "String.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

void SendServerNodes(DvarNode *node, int playerid) {
	static byte message[MSG_LEN] = { SVMSG_COMMAND };

	for (; node; node = node->next) {
		if (node->dvar && node->dvar->server) {
			unsigned int string_length = DvarGetCommandString(node->dvar, message + 1, MSG_LEN - 1);
			if (string_length)
				ServerSend(playerid, message, string_length + 2);
		}

		if (node->child)
			SendServerNodes(node->child, playerid);
	}
}

////////////////

void PrintValue(Dvar* dvar) {
	if (dvar->type == DVT_FUNCTION || dvar->type == DVT_CALL) return;

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

Dvar* AddDvarC(const char *name, DvarType type, DvarValue value, DvarCallback *callback, bool server) {
	if (dvar_root && GetDvar(name))
		return NULL;

	Dvar *dvar = (Dvar*)malloc(sizeof(Dvar));
	dvar->name = DupString(name);
	dvar->type = type;
	dvar->callback = callback;
	dvar->server = server;

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

unsigned int DvarGetCommandString(const Dvar *dvar, char dest[], unsigned int dest_size) {
	char *value = DvarAllocValueString(dvar);

	if (value) {
		snprintf(dest, dest_size, "%s %s", dvar->name, value);

		free(value);

		//Return length of string
		return strlen(dvar->name) + 1 + strlen(value);
	}

	return 0;
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

	if (count > 0 && dvar->callback)
		dvar->callback(dvar->value);
}

void HandleCommand(const char **tokens, unsigned int count, const char *string, bool message_server) {
	char **tokens_created = NULL;

	if (string) {
		count = SplitTokens(string, &tokens_created);
		tokens = tokens_created;
	}

	if (count == 0) return;

	Dvar *dvar = GetDvar(tokens[0]);
	if (!dvar) {
		ConsolePrint(tokens[0]);
		ConsolePrint(" is not a variable\n");
	}
	else {
		if (message_server && dvar->server) {
			if (string)
				MessageServerString(SVMSG_COMMAND, string);
			else {
				char *commandstr = CombineTokens(tokens, count);
				MessageServerString(SVMSG_COMMAND, commandstr);
				free(commandstr);
			}
		}
		else {
			DvarCommand(dvar, tokens + 1, count - 1);
			PrintValue(dvar);
		}
	}

	if (tokens_created)
		FreeTokens(tokens_created, count);
}

void HandleCommandTokens(const char **tokens, unsigned int count) {
	HandleCommand(*&tokens, count, NULL, true);
}

void HandleCommandString(const char *command, bool message_server) {
	HandleCommand(NULL, 0, command, message_server);
}

void SendServerDvars(int id) {
	SendServerNodes(dvar_root, id);
}

#include "Resource.h"

char *LoadStringResource(HINSTANCE inst, int id) {
	HRSRC resource = FindResource(inst, MAKEINTRESOURCE(id), RT_RCDATA);
	if (!resource)
		return NULL;

	HGLOBAL res_h = LoadResource(inst, resource);
	if (!res_h)
		return NULL;

	return LockResource(res_h);
}

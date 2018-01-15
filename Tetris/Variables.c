#include "Variables.h"

#include "Dvar.h"

void CLSetTextureIndexOrder(const char **tokens, unsigned int count) {

}

void CLSetTextureLevel(const char **tokens, unsigned int count) {

}

void InitDvars() {
	AddDString(		"cl_texture",				"");
	AddDFloat(		"cl_texture_index_size",	0);
	AddDFunction(	"cl_texture_index_order",	CLSetTextureIndexOrder);
	AddDFunction(	"cl_texture_level",			CLSetTextureLevel);
}

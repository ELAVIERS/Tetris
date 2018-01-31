#include "Callbacks.h"
#include "Config.h"
#include "Console.h"
#include "Bitmap.h"

void C_RunAsConfig(DvarValue strvalue) {
	RunConfig(strvalue.dstring);
}

Bitmap bmp_font;
Bitmap bmp_blocks;

void C_FontTexture(DvarValue strvalue) {
	FreeBMPBuffer(&bmp_font);

	if (LoadBMP(strvalue.dstring, &bmp_font)) {

	}
}

void C_FontIDSize(DvarValue fvalue) {

}

void C_BlockTexture(DvarValue strvalue) {
	FreeBMPBuffer(&bmp_blocks);

	if (LoadBMP(strvalue.dstring, &bmp_blocks)) {

	}
}

void C_BlockIDSize(DvarValue fvalue) {

}

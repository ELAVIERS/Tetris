#include "Rendering.h"
#include "Block.h"
#include "BlockManager.h"
#include "Globals.h"
#include "Shader.h"
#include <GL/GL.h>

void RenderTileBuffer(const byte *buffer, byte rows, byte columns, Mat3 in_transform, const Quad* quad, unsigned int level) {
	Mat3 transform;

	for (unsigned int r = 0; r < rows; ++r)
		for (unsigned int c = 0; c < columns; ++c) {
			if (buffer[RC1D(columns, r, c)]) {
				Mat3Identity(transform);
				Mat3Translate(transform, (float)c, (float)r);
				Mat3Multiply(transform, in_transform);
				ShaderSetUniformMat3(g_active_shader, "u_transform", transform);

				short index = TextureLevelIDIndex(level, buffer[RC1D(columns, r, c)]);

				unsigned short divsx = (short)(1.f / quad->uv_w + 0.5f);
				unsigned short divsy = (short)(1.f / quad->uv_h + 0.5f);

				ShaderSetUniformFloat2(g_active_shader, "u_uvoffset", (float)(index % divsx) * quad->uv_w + .00005f, (float)(index / divsx) * quad->uv_h);

				QuadRender(quad);
			};
		}
}

inline void RenderEdgePart(Mat3 transform, int id) {
	ShaderSetUniformMat3(g_active_shader, "u_transform", transform);
	glBindTexture(GL_TEXTURE_2D, g_textures[id].glid);
	QuadRender(g_quads + QUAD_SINGLE);
}

void RenderBorder(float x, float y, float w, float h, float bw, float bh) {
	Mat3 transform;
	w -= bw;
	h -= bh;

	Mat3Identity(transform);
	Mat3Scale(transform, bw, bh);
	Mat3Translate(transform, x, y);
	RenderEdgePart(transform, TEX_BL);

	Mat3Translate(transform, 0, h);
	RenderEdgePart(transform, TEX_UL);

	Mat3Translate(transform, w, 0);
	RenderEdgePart(transform, TEX_UR);

	Mat3Translate(transform, 0, -h);
	RenderEdgePart(transform, TEX_BR);

	//Horizontal edges
	Mat3Identity(transform);
	Mat3Scale(transform, w - bw, bh);
	Mat3Translate(transform, x + bw, y);
	RenderEdgePart(transform, TEX_B);

	Mat3Translate(transform, 0.f, h);
	RenderEdgePart(transform, TEX_U);

	//Vertical Edges
	Mat3Identity(transform);
	Mat3Scale(transform, bw, h - bh);
	Mat3Translate(transform, x, y + bh);
	RenderEdgePart(transform, TEX_L);

	Mat3Translate(transform, w, 0.f);
	RenderEdgePart(transform, TEX_R);
}

#include "Text.h"

void RenderString(const char *string, Mat3 transform) {
	glBindTexture(GL_TEXTURE_2D, g_textures[TEX_FONT].glid);

	unsigned short divsx = (short)(1.f / g_quads[QUAD_FONT].uv_w);
	unsigned short divsy = (short)(1.f / g_quads[QUAD_FONT].uv_h);

	Mat3 copy;
	Mat3Copy(copy, transform);

	size_t len = strlen(string);
	for (size_t i = 0; i < len; ++i) {
		ShaderSetUniformMat3(g_active_shader, "u_transform", copy);

		int id = GetCharID(string[i]);
		ShaderSetUniformFloat2(g_active_shader, "u_uvoffset", (id % divsx) * g_quads[QUAD_FONT].uv_w, 1.f - ((id / divsx + 1.f) * g_quads[QUAD_FONT].uv_h));

		QuadRender(g_quads + QUAD_FONT);

		Mat3Translate(copy, transform[0][0], 0);
	}
}

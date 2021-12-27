/*
 * Partially based on skshader_editor/imgui_impl_skg.cpp
 * Wish I had found it before implementing most of this myself.
 */

#include <stdlib.h>
#include <skig.h>
#include <string>
#include <stereokit.h>
#include <stereokit_ui.h>
#include <sk_gpu.h>
#include <asset_types/material.h>
#include <asset_types/texture.h>
#include <imgui.h>
#include "imgui_shader.hlsl.h"

using namespace sk;
using namespace ImGui;

namespace skig {

typedef struct SKIGBuffer {
	skg_buffer_type_ type;
	size_t size = 0;
	void* marshal;
	size_t marshalCount;
	skg_buffer_t buffer;
} SKIGBuffer;

skg_pipeline_t pipeline;
skg_shader_t shader;

skg_buffer_t paramBuffer;

SKIGBuffer vertexBuffer { skg_buffer_type_vertex };
SKIGBuffer indexBuffer { skg_buffer_type_index };

skg_mesh_t renderMesh;

skg_tex_t fontTex;

void skig_ensureMarshalSize(SKIGBuffer* buffer, const size_t count, const size_t stride) {
	const size_t size = count * stride;

	if (buffer->marshalCount < count) {
		if (buffer->marshal != nullptr) {
			free(buffer->marshal);
		}

		buffer->marshal = malloc(size);
		buffer->marshalCount = count;
	}
}

void skig_setBufferContents(skg_mesh_t* mesh, SKIGBuffer* buffer, const size_t count, const size_t stride) {
	const size_t size = count * stride;

	if (buffer->size < size) {
		if (skg_buffer_is_valid(&buffer->buffer)) {
			skg_buffer_destroy(&buffer->buffer);
		}

		buffer->buffer = skg_buffer_create(buffer->marshal, count, stride, buffer->type, skg_use_dynamic);
		buffer->size = size;

		switch (buffer->type) {
		case skg_buffer_type_vertex:
			skg_mesh_set_verts(mesh, &buffer->buffer);
			break;
		case skg_buffer_type_index:
			skg_mesh_set_inds(mesh, &buffer->buffer);
			break;
		}
	} else {
		skg_buffer_set_contents(&buffer->buffer, buffer->marshal, size);
	}
}

void skig_destroyBuffer(SKIGBuffer* buffer) {
	if (skg_buffer_is_valid(&buffer->buffer)) skg_buffer_destroy(&buffer->buffer);

	if (buffer->marshal != nullptr) free(buffer->marshal);

	buffer->size = 0;
	buffer->marshalCount = 0;
}

bool skig_init() {
	shader = skg_shader_create_memory(sks_imgui_shader_hlsl, sizeof(sks_imgui_shader_hlsl));
	if (!skg_shader_is_valid(&shader)) {
		skg_log(skg_log_critical, "Failed to initialize StereoKitImGui shader!");
		return false;
	}

	pipeline = skg_pipeline_create(&shader);
	skg_shader_meta_release(shader.meta);

	skg_pipeline_set_cull(&pipeline, skg_cull_none);
	skg_pipeline_set_depth_test(&pipeline, skg_depth_test_always);
	skg_pipeline_set_depth_write(&pipeline, false);
	skg_pipeline_set_scissor(&pipeline, true);
	skg_pipeline_set_transparency(&pipeline, skg_transparency_blend);

	paramBuffer = skg_buffer_create(nullptr, 1, sizeof(float[4][4]), skg_buffer_type_constant, skg_use_dynamic);

	renderMesh = skg_mesh_create(&vertexBuffer.buffer, &indexBuffer.buffer);

	ImGuiIO& io = ImGui::GetIO();
	void* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32((unsigned char **)&pixels, &width, &height);

	fontTex = skg_tex_create(skg_tex_type_image, skg_use_static, skg_tex_fmt_rgba32_linear, skg_mip_none);
	skg_tex_set_contents(&fontTex, pixels, width, height);

	io.Fonts->TexID = (ImTextureID) &fontTex;

	return true;
}

void skig_destroy() {
	skg_pipeline_destroy(&pipeline);
	skg_shader_destroy(&shader);

	skg_buffer_destroy(&paramBuffer);

	skig_destroyBuffer(&vertexBuffer);
	skig_destroyBuffer(&indexBuffer);

	skg_mesh_destroy(&renderMesh);

	skg_tex_destroy(&fontTex);
}

typedef struct SKIG_Save {
	skg_tex_t* oldTarget;
	int32_t oldViewport[4];
} SKIG_Save;

SKIG_Save savedState;

void skig_begin() {
	savedState.oldTarget = skg_tex_target_get();
	skg_viewport_get((int32_t*) &savedState.oldViewport);
}

void skig_end() {
	skg_tex_target_bind(savedState.oldTarget);
	skg_viewport((int32_t*) &savedState.oldViewport);

	// Reset scissor or the following clear will use it
	skg_scissor((int32_t*) &savedState.oldViewport);
}

void skig_render(tex_t renderTarget) {
	if (renderTarget == nullptr) return;

	ImDrawData* drawData = ImGui::GetDrawData();

	ImVec2 displaySize = drawData->DisplaySize;
	if (displaySize.x <= 0.0f || displaySize.y <= 0.0f) return;

	skg_tex_t* tex = &renderTarget->tex;
	skg_tex_target_bind(tex);

	{
		// Reset scissor or the following clear will use it
		int32_t clearScissor[4] = { 0, 0, tex->width, tex->height };
		skg_scissor(clearScissor);

		float clearColor[4] = { 0, 0, 0, 0 };
		skg_target_clear(false, clearColor);
	}

	ImVec2 displayPos = drawData->DisplayPos;
	// Setup orthographic projection matrix into our constant buffer
	// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
	{
		float L = displayPos.x;
		float R = displayPos.x + drawData->DisplaySize.x;
		float T = displayPos.y;
		float B = displayPos.y + drawData->DisplaySize.y;
		float mvp[4][4] = {
			{ 2.0f/(R-L),  0.0f,        0.0f, 0.0f },
			{ 0.0f,        2.0f/(T-B),  0.0f, 0.0f },
			{ 0.0f,        0.0f,        0.5f, 0.0f },
			{ (R+L)/(L-R), (T+B)/(B-T), 0.5f, 1.0f }, };
		skg_buffer_set_contents(&paramBuffer, mvp, sizeof(mvp));
	}

	skg_pipeline_bind(&pipeline);

	skg_buffer_bind(&paramBuffer, { 0, skg_stage_vertex }, 0);

	for (size_t drawListIndex = 0; drawListIndex < drawData->CmdListsCount; drawListIndex++) {
		const ImDrawList* drawList = drawData->CmdLists[drawListIndex];

		// Update vertex buffer
		skig_ensureMarshalSize(&vertexBuffer, drawList->VtxBuffer.Size, sizeof(skg_vert_t));

		for (size_t i = 0; i < drawList->VtxBuffer.Size; i++) {
			skg_vert_t* vertex = &((skg_vert_t*) vertexBuffer.marshal)[i];
			ImDrawVert* sourceVertex = &drawList->VtxBuffer.Data[i];

			vertex->pos[0] = sourceVertex->pos[0];
			vertex->pos[1] = sourceVertex->pos[1];
			vertex->uv[0] = sourceVertex->uv[0];
			vertex->uv[1] = sourceVertex->uv[1];
			memcpy(&vertex->col, &sourceVertex->col, sizeof(ImU32));
		}

		skig_setBufferContents(&renderMesh, &vertexBuffer, drawList->VtxBuffer.Size, sizeof(skg_vert_t));

		// Update index buffer
		skig_ensureMarshalSize(&indexBuffer, drawList->IdxBuffer.Size, sizeof(int32_t));

		// Dear ImGui uses int16 for indices, skg uses int32
		for (size_t i = 0; i < drawList->IdxBuffer.Size; i++) {
			((int32_t*) indexBuffer.marshal)[i] = drawList->IdxBuffer[i];
		}

		skig_setBufferContents(&renderMesh, &indexBuffer, drawList->IdxBuffer.Size, sizeof(int32_t));

		// Rebind buffer in case it changed
		// FIXME: this is slow
		skg_mesh_bind(&renderMesh);

		for (size_t drawIndex = 0; drawIndex < drawList->CmdBuffer.Size; drawIndex++) {
			const ImDrawCmd* drawCmd = &drawList->CmdBuffer[drawIndex];

			if (drawCmd->UserCallback != nullptr) {
				if (drawCmd->UserCallback != ImDrawCallback_ResetRenderState)
					drawCmd->UserCallback(drawList, drawCmd);

				continue;
			}

			ImVec4 clipRect = drawCmd->ClipRect;
			int32_t scissor[4] = { (int) (clipRect.x - displayPos.x), (int) (clipRect.y - displayPos.y), (int) (clipRect.z - clipRect.x), (int) (clipRect.w - clipRect.y) };
			skg_scissor(scissor);

			// TODO: allow arbitrary textures
			skg_tex_bind((skg_tex_t*) drawCmd->TextureId, { 0, skg_stage_pixel });

			skg_draw(drawCmd->IdxOffset, drawCmd->VtxOffset, drawCmd->ElemCount, 1);
		}
	}
}

}

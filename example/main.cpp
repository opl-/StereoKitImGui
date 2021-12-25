#include <stdlib.h>
#include <string>
#include <stereokit.h>
#include <stereokit_ui.h>
#include <imgui.h>
#include <skig.h>

using namespace sk;
using namespace ImGui;
using namespace skig;

mesh_t sphere_mesh;
material_t sphere_mat;
pose_t sphere_pose = pose_identity;

tex_t renderTarget;

int main(void) {
	// Setup StereoKit
	sk_settings_t settings = {};
	settings.app_name = "SKNativeTemplate";
	settings.assets_folder = "Assets";
	settings.display_preference = display_mode_flatscreen;
	if (!sk_init(settings))
		return 1;

	// Setup ImGui
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(600, 600);
	io.DeltaTime = 1.0f / 90.0f;

	io.Fonts->AddFontDefault();
	unsigned char* atlasPixels = nullptr;
	int atlasWidth, atlasHeight;
	io.Fonts->GetTexDataAsRGBA32(&atlasPixels, &atlasWidth, &atlasHeight);

	// Setup StereoKitImGui
	skig_init();

	// Create a render target texture with an appropriate size
	renderTarget = tex_create(tex_type_rendertarget);
	tex_set_colors(renderTarget, 600, 600, nullptr);

	// Setup application
	sphere_mesh = mesh_find(default_id_mesh_sphere);
	sphere_mat = material_find(default_id_material);
	sphere_pose.position.z = -2.0f;

	// Use texture
	material_set_texture(sphere_mat, "diffuse", renderTarget);
	tex_release(renderTarget);

	while (sk_step( []() {
		// Save current render state
		skig_begin();

		// Render any number of UIs
		ImGui::NewFrame();

		ImGui::Text("test");
		ImGui::Button("button");

		ImGui::Render();

		skig_render(renderTarget);

		// Restore saved render state
		skig_end();

		// Render the rest of the application
		ui_handle_begin("Sphere", sphere_pose, mesh_get_bounds(sphere_mesh), false);
		render_add_mesh(sphere_mesh, sphere_mat, matrix_identity);
		ui_handle_end();
	}));

	// Clean up
	skig_destroy();
	ImGui::DestroyContext();
	sk_shutdown();

	return 0;
}

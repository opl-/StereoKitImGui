# StereoKit + ImGui

The code in this repository integrates ImGui into StereoKit (using sk_gpu for rendering).

ImGui is rendered into a StereoKit render target texture (`tex_t`).


## Usage

See `example/` for an example written in C++.

SKIG tries to do as little as possible. The user is expected to:

During application initialization:

- set up an ImGui context and font,
- call `skig_init` to initialize the font atlas texture,
- set up a StereoKit render target.

During application step:

- call `skig_begin` to save the default render pipeline state,
- render as many ImGui contexts as wanted:
	- update ImGui IO state,
	- draw widgets as usual,
	- call `ImGui::Render()`,
	- call `skig_render(renderTarget)`, which will call `ImGui::GetDrawData()` and render it to the passed in texture,
- call `skig_end` to restore the render pipeline state for main viewport.

During application exit, or when you want to free memory used by ImGui:

- call `skig_destroy`.


## Directories

- `example/` contains a simple C++ application based on the StereoKit native template.
- `include/` contains the public headers.
- `src/` contains the source code.


## Build

Changing the StereoKit or ImGui version can be done using `CMakeLists.txt`. This might become configurable in the future.

This repository contains a prebuilt [`imgui_shader.hlsl.h`](https://github.com/maluoi/sk_gpu/blob/master/skshader_editor/imgui_shader.hlsl.h) from [sk_gpu](https://github.com/maluoi/sk_gpu). If using a different version of StereoKit, you might need to copy a compiled version or build it manually using the shader compiler or editor available, both of which are available from that repository.

Use CMake to generate build files:

```sh
cmake --build . --parallel 4 --config Release
```


## Using in your project

[CPM](https://github.com/cpm-cmake/CPM.cmake) should make this process easy (see their documentation for how to include it):

```cmake
# ...

CPMAddPackage("gh:opl-/StereoKitImGui@1.0.0")

target_link_libraries(yourProject StereoKitImGui)
```

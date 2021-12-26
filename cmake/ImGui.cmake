CPMAddPackage(
  NAME ImGui
  GITHUB_REPOSITORY ocornut/imgui
  GIT_TAG v1.85
)

# ImGui #

set(IMGUI_SOURCES
  ${ImGui_SOURCE_DIR}/imgui.cpp
  ${ImGui_SOURCE_DIR}/imgui_draw.cpp
  ${ImGui_SOURCE_DIR}/imgui_tables.cpp
  ${ImGui_SOURCE_DIR}/imgui_widgets.cpp
  ${ImGui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

set(IMGUI_PUBLIC_HEADERS
  ${ImGui_SOURCE_DIR}/imconfig.h
  ${ImGui_SOURCE_DIR}/imgui.h
  ${ImGui_SOURCE_DIR}/imgui_internal.h # not actually public, but users might need it
  ${ImGui_SOURCE_DIR}/imstb_rectpack.h
  ${ImGui_SOURCE_DIR}/imstb_textedit.h
  ${ImGui_SOURCE_DIR}/imstb_truetype.h
  ${ImGui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h
)

add_library(ImGui SHARED
  ${IMGUI_PUBLIC_HEADERS}
  ${IMGUI_SOURCES}
)

target_include_directories(ImGui
PUBLIC
  ${ImGui_SOURCE_DIR}
)

set_target_properties(ImGui PROPERTIES
  PUBLIC_HEADER "${IMGUI_PUBLIC_HEADERS}"
)

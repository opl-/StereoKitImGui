#include <stereokit.h>

namespace skig {

bool skig_init();

// Save current render state to allow restoring it lter with skig_end
void skig_begin();

// Render draw data returned by ImGui for the current context
void skig_render(sk::tex_t renderTarget);

// Restore previously saved render state
void skig_end();

void skig_destroy();

}

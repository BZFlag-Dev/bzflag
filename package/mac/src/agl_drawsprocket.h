
#include <DrawSprocket.h>
#include <agl.h>

/* static function prototypes */

CGrafPtr dsSetupScreen( int width, int height );
void dsShutdownScreen(CGrafPtr theFrontBuffer);

AGLContext dsSetupAGL(AGLDrawable win);
void dsCleanupAGL(AGLContext ctx);

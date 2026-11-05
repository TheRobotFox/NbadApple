#include "Video.h"
#include <nspireio/nspireio.h>
#include <os.h>

// FIXME my newlib requires this, probably a version conflict
void _fini(void) {}

nio_console console;

int main(void) {
  // CX tested!
  if (!has_colors)
    return 0;
  Video vid = {0};
  nio_init(&console, NIO_MAX_COLS, NIO_MAX_ROWS, 0, 0, NIO_COLOR_BLACK,
           NIO_COLOR_WHITE, TRUE);
  nio_fprintf(&console, "Loading Data!\n");
  int error = init_Video("frames.tns", &vid);
  if (error) {
    nio_fprintf(&console, "Error %d\n", error);
    nio_free(&console);
    goto cleanup;
  }
	
  nio_free(&console);
  Player(&vid);
cleanup:
  closevid(&vid);

  return 0;
}

#include "Video.h"
#include "cx_timer.h"
#include "frame_offsets.h"

void write_frame(Video *vid) {
  memcpy(REAL_SCREEN_BASE_ADDRESS, vid->Frame, sizeof(short) * PIXELS);
}

unsigned int load_frame(short frame, Video *vid) {
  if (frame < 0 || frame >= FRAMES)
    return 0;
  unsigned int length = frame_offsets[frame + 1] - frame_offsets[frame];
  if (length > ENCODEDBUFFERCHACHE ||
      fseek(vid->File, frame_offsets[frame], SEEK_SET))
    return 0;
  vid->encoded_length = fread(vid->frame_encoded, 1, length, vid->File);
  return vid->encoded_length == length ? length : 0;
}

int init_Video(char *path, Video *vid) {
  memset(vid, 0, sizeof(*vid));
  vid->File = fopen(path, "rb");
  if (!vid->File)
    return -1;
  vid->frame_encoded = malloc(ENCODEDBUFFERCHACHE);
  vid->Frame = malloc(sizeof(*vid->Frame) * PIXELS);
  if (!vid->frame_encoded || !vid->Frame)
    return -2;

  /* Reject files with a different size than the generated index. */
  if (fseek(vid->File, 0, SEEK_END) ||
      ftell(vid->File) != (long)frame_offsets[FRAMES])
    return 3;
  return 0;
}

char decode_frame(Video *vid) {
  unsigned int count = 0;
  for (unsigned int offset = 0; offset < vid->encoded_length; ++offset) {
    unsigned int run = vid->frame_encoded[offset];
    if (run > PIXELS - count)
      return 1;
    memset(vid->Frame + count, (offset & 1) ? 0xFF : 0,
           run * sizeof(*vid->Frame));
    count += run;
  }
  return count != PIXELS;
}

void Player(Video *vid) {
  lcd_init(SCR_320x240_16);

  // setup timer for 30FPS
  cx_timer_init(1092);
  cx_timer_start();

  char autoplay = 0;
  unsigned int period = 1092;
  int old_frame = -1;
  while (1) {

    if (cx_timer_pending()) {
      cx_timer_ack();
      if (autoplay)
        vid->current_frame = (vid->current_frame + 1) % FRAMES;
    }

    if (old_frame != vid->current_frame) {
      if (!load_frame(vid->current_frame, vid) || decode_frame(vid))
        break;
      old_frame = vid->current_frame;
      write_frame(vid);
    }
    if (isKeyPressed(KEY_NSPIRE_PLUS)) {
      vid->current_frame = (vid->current_frame + 1) % FRAMES;
      while (isKeyPressed(KEY_NSPIRE_PLUS));
    }
    if (isKeyPressed(KEY_NSPIRE_MINUS)) {
      vid->current_frame = (vid->current_frame + FRAMES - 1) % FRAMES;
      while (isKeyPressed(KEY_NSPIRE_MINUS));
    }
    if (isKeyPressed(KEY_NSPIRE_ENTER)) {
      autoplay = !autoplay;
      while (isKeyPressed(KEY_NSPIRE_ENTER));
    }
    if (isKeyPressed(KEY_NSPIRE_MULTIPLY)) {
      if (period > 273)
        period /= 2;
      cx_timer_set_period(period);
      while (isKeyPressed(KEY_NSPIRE_MULTIPLY));
    }
    if (isKeyPressed(KEY_NSPIRE_DIVIDE)) {
      if (period < 4368)
        period *= 2;
      cx_timer_set_period(period);
      while (isKeyPressed(KEY_NSPIRE_DIVIDE));
    }
    if (isKeyPressed(KEY_NSPIRE_0))
      vid->current_frame = 0;
    if (isKeyPressed(KEY_NSPIRE_ESC))
      break;
  }

  cx_timer_restore();
}

void closevid(Video *vid) {
  free(vid->frame_encoded);
  free(vid->Frame);
  if (vid->File)
    fclose(vid->File);
  vid->current_frame = -1;
}

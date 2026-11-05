#include "Video.h"
#include "cx_timer.h"

void write_frame(Video *vid) {
  memcpy(REAL_SCREEN_BASE_ADDRESS, vid->Frame, sizeof(short) * PIXELS);
}

unsigned int load_frame(short frame, Video *vid) {
  if (frame < 0 || frame >= FRAMES)
    return 0;
  unsigned int length = vid->frame_offsets[frame + 1] - vid->frame_offsets[frame];
  if (length > ENCODEDBUFFERCHACHE ||
      fseek(vid->File, vid->frame_offsets[frame], SEEK_SET))
    return 0;
  vid->encoded_length = fread(vid->frame_encoded, 1, length, vid->File);
  return vid->encoded_length == length ? length : 0;
}

int init_Video(char *path, Video *vid) {
  memset(vid, 0, sizeof(*vid));
  vid->File = fopen(path, "rb");
  if (!vid->File)
    return -1;
  vid->frame_offsets = calloc(FRAMES + 1, sizeof(*vid->frame_offsets));
  vid->frame_encoded = malloc(ENCODEDBUFFERCHACHE);
  vid->Frame = malloc(sizeof(*vid->Frame) * PIXELS);
  if (!vid->frame_offsets || !vid->frame_encoded || !vid->Frame)
    return -2;

  int frame = 0, count = 0, offset = 0, value;
  while ((value = fgetc(vid->File)) != EOF) {
    if (frame >= FRAMES)
      return 3;
    count += value;
    ++offset;
    if (count > PIXELS)
      return 3;
    if (count == PIXELS) {
      if (offset - vid->frame_offsets[frame] > ENCODEDBUFFERCHACHE)
        return 3;
      vid->frame_offsets[++frame] = offset;
      count = 0;
    }
    if (isKeyPressed(KEY_NSPIRE_ESC))
      return 4;
  }
  return ferror(vid->File) || count || frame != FRAMES ? 3 : 0;
}

char decode_frame(Video *vid) {
  unsigned int count = 0;
  for (unsigned int offset = 0; offset < vid->encoded_length; ++offset) {
    unsigned int run = vid->frame_encoded[offset];
    if (run > PIXELS - count)
      return 1;
    for (unsigned int i = 0; i < run; ++i)
      vid->Frame[count++] = (offset % 2) ? 65535 : 0;
  }
  return count != PIXELS;
}

void Player(Video *vid) {
  lcd_init(SCR_320x240_16);

  // setup timer for 30FPS
  cx_timer_init(1092);

  cx_timer_start();

  char autoplay = 0;
  unsigned int frame_counter = 0;
  int old_frame = -1;
  while (1) {

    if (cx_timer_pending()) {
      cx_timer_ack();
      if (autoplay)
        ++frame_counter;
    }

    if (old_frame != (int)(frame_counter % FRAMES)) {
      vid->current_frame = frame_counter % FRAMES;

      if (!load_frame(vid->current_frame, vid) || decode_frame(vid))
        break;
      old_frame = vid->current_frame;
      write_frame(vid);
    }
    if (isKeyPressed(KEY_NSPIRE_PLUS)) {
      while(isKeyPressed(KEY_NSPIRE_PLUS));
      frame_counter = vid->current_frame + 1;
    }
    if (isKeyPressed(KEY_NSPIRE_MINUS)) {
      while(isKeyPressed(KEY_NSPIRE_MINUS));
      frame_counter = (vid->current_frame + FRAMES - 1) % FRAMES;
    }
    if (isKeyPressed(KEY_NSPIRE_ENTER)) {
      autoplay = !autoplay;
      while(isKeyPressed(KEY_NSPIRE_ENTER));
    }
    if (isKeyPressed(KEY_NSPIRE_0))
      frame_counter = 0;
    if (isKeyPressed(KEY_NSPIRE_ESC))
      break;
  }

  // restore Timer
  cx_timer_restore();
}

void closevid(Video *vid) {
  free(vid->frame_offsets);
  free(vid->frame_encoded);
  free(vid->Frame);
  if (vid->File)
    fclose(vid->File);
  vid->current_frame = -1;
}

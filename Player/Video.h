#pragma once
#include <os.h>
#include <nspireio/nspireio.h>
#include "util.h"

#define ENCODEDBUFFERCHACHE 21627
#define PIXELS (320*240)
#define FRAMES 6571

extern nio_console console;

typedef struct {
	FILE* File;
	short current_frame;
    unsigned int encoded_length;
	unsigned char* frame_encoded;
	short* Frame;
} Video;

void write_frame(Video* vid);


unsigned int load_frame(short frame, Video* vid);


int init_Video(char* path, Video* vid);


char decode_frame(Video* vid);


void Player(Video* vid);


void closevid(Video *vid);

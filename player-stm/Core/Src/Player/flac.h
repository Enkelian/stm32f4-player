#pragma once

#include <stdint.h>
#include "fatfs.h"
#include "FLAC/stream_decoder.h"

typedef struct FlacFrame {
	int samples;
	int size;
	uint8_t *data;
} FlacFrame;

typedef struct Flac {
	FLAC__StreamDecoder* decoder;
	FIL* input;
	FIL* output;
	FlacFrame* read_frame;
} Flac;

Flac* Flac_Create();
void Flac_Delete(Flac* flac);
int Flac_GetMetadata(Flac* flac);
int Flac_GetFrame(Flac* flac
//		, FlacFrame** frame
		);

#pragma once

#include "flac.h"

typedef struct FlacAdapter {
	Flac* flac;
	FlacFrame* processed_frame;
	int frame_offset;
} FlacAdapter;

FlacAdapter FlacAdapter_Create(Flac* flac);
void FlacAdapter_Delete(FlacAdapter* adapter);
int FlacAdapter_Read(FlacAdapter* adapter, void* target, int bytes);

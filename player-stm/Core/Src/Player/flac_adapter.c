#include "term_io.h"
#include "flac_adapter.h"

FlacAdapter FlacAdapter_Create(Flac* flac) {
	return (FlacAdapter) {
		.flac = flac
	};
}

void FlacAdapter_Delete(FlacAdapter* adapter) {
}

int FlacAdapter_Get(FlacAdapter* adapter, void* target, int bytes) {
//	xprintf("FlacAdapter_Get\n");
	int read_bytes = 0;
	while(read_bytes < bytes) {
//		xprintf("%d of %d bytes read...\n", read_bytes, bytes);
		if(adapter->processed_frame == NULL) {
			adapter->frame_offset = 0;
			if(Flac_GetFrame(adapter->flac, &adapter->processed_frame)) {
				xprintf("ERROR: cannot get frame\n");
				return read_bytes;
			}
		}

		int current_frame_bytes_left = adapter->processed_frame->size - adapter->frame_offset;
		int read_bytes_left = bytes - read_bytes;

		if(current_frame_bytes_left <= read_bytes_left) {
			// current frame has less or equal bytes than we need
			// if it has less, in next iteration next frame will be read
			memcpy(
					target + read_bytes,
					&adapter->processed_frame->data[adapter->frame_offset],
					current_frame_bytes_left
			);
			read_bytes += current_frame_bytes_left;
			adapter->processed_frame = NULL;
			adapter->frame_offset = 0;
		} else {
			// current frame has more bytes than we need
			// it will be used in next FlacAdapter_Get invocation

			for(int i = 0; i < read_bytes_left; i++) {
				xprintf("%d ", adapter->processed_frame->data[adapter->frame_offset + i]);
			}
			xprintf("\n");

			memcpy(
					target + read_bytes,
					&adapter->processed_frame->data[adapter->frame_offset],
					read_bytes_left
			);
			read_bytes += read_bytes_left;
			adapter->frame_offset += read_bytes_left;
		}
	}
//	xprintf("%d of %d bytes read...\n", read_bytes, bytes);
	return read_bytes;
}

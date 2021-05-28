#include "term_io.h"
#include "flac_adapter.h"
#include "dbgu.h"

FlacAdapter FlacAdapter_Create(Flac* flac) {
	return (FlacAdapter) {
		.flac = flac,
		.frame_offset = 0
	};
}

void FlacAdapter_Delete(FlacAdapter* adapter) {
}

int FlacAdapter_Get(FlacAdapter* adapter, void* target, int bytes) {
	xprintf("FlacAdapter_Get\n");
	//PROBABLY THE PROBLEM IS HERE
	int read_bytes = 0;
   HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
   HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
	while(read_bytes < bytes) {
//		xprintf("read_offset=%d\n", adapter->frame_offset);
//		xprintf("%d of %d bytes read...\n", read_bytes, bytes);
		if(adapter->flac->read_frame == NULL) {
//			xprintf("get new frame... already read %d\n", read_bytes);

			adapter->frame_offset = 0;
			if(Flac_GetFrame(adapter->flac)) {
				xprintf("ERROR: cannot get frame\n");
				   HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
				   HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
				while(1);
				return read_bytes;
			}
		}

		int current_frame_bytes_left = adapter->flac->read_frame->size - adapter->frame_offset;
		int read_bytes_left = bytes - read_bytes;

		if(current_frame_bytes_left <= read_bytes_left) {
			// current frame has less or equal bytes than we need
			// if it has less, in next iteration next frame will be read
			memcpy(
					target + read_bytes,
					&adapter->flac->read_frame->data[adapter->frame_offset],
					current_frame_bytes_left
			);
//			xprintf("IF: current_frame_bytes_left=%d\n", current_frame_bytes_left);
//			xprintf("IF: read_bytes=%d\n", read_bytes);

//			debug_dump(&adapter->processed_frame->data[adapter->frame_offset], current_frame_bytes_left);

			read_bytes += current_frame_bytes_left;
//
//
//			UINT written;
//			if(f_write(adapter->flac->output, &adapter->flac->read_frame->data[adapter->frame_offset], current_frame_bytes_left, &written) != FR_OK) {
//				xprintf("cannot write to file\n");
//			} else {
//				f_sync(adapter->flac->output);
//				xprintf("written %d bytes to file\n", written);
//			}
//
//			if(f_write(adapter->flac->output, target + read_bytes, current_frame_bytes_left, &written) != FR_OK) {
//				xprintf("cannot write to file\n");
//			} else {
//				f_sync(adapter->flac->output);
//				xprintf("written %d bytes to file\n", written);
//			}



			free(adapter->flac->read_frame->data);
			free(adapter->flac->read_frame);
//		    xprintf("FREE FRAME...\n");

			adapter->flac->read_frame = NULL;
			adapter->frame_offset = 0;
		} else {
			// current frame has more bytes than we need
			// it will be used in next FlacAdapter_Get invocation

//			for(int i = 0; i < read_bytes_left; i++) {
//				xprintf("%d ", adapter->processed_frame->data[adapter->frame_offset + i]);
//			}
//			xprintf("\n");

			memcpy(
					target + read_bytes,
					&adapter->flac->read_frame->data[adapter->frame_offset],
					read_bytes_left
			);
//			xprintf("ELSE: read_bytes_left=%d\n", read_bytes_left);
//			xprintf("ELSE: read_bytes=%d\n", read_bytes);
//
//
//			UINT written;
//			if(f_write(adapter->flac->output, &adapter->flac->read_frame->data[adapter->frame_offset], read_bytes_left, &written) != FR_OK) {
//				xprintf("cannot write to file\n");
//			} else {
//				f_sync(adapter->flac->output);
//				xprintf("written %d bytes to file\n", written);
//			}

//			if(f_write(adapter->flac->output, target + read_bytes, read_bytes_left, &written) != FR_OK) {
//				xprintf("cannot write to file\n");
//			} else {
//				f_sync(adapter->flac->output);
//				xprintf("written %d bytes to file\n", written);
//			}
//

//			debug_dump(&adapter->processed_frame->data[adapter->frame_offset], read_bytes_left);

			read_bytes += read_bytes_left;
			adapter->frame_offset += read_bytes_left;
		}
//			xprintf("%d of %d bytes read...\n", read_bytes, bytes);

	}
   HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
   HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);

	xprintf("%d of %d bytes read...\n", read_bytes, bytes);
	return read_bytes;
}

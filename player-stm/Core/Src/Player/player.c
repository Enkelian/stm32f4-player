#include "fatfs.h"
#include "term_io.h"
#include "usbh_platform.h"
#include "stm32f4_discovery_audio.h"
#include "flac.h"

FIL file;
const char *FNAME = "barka.wav";
extern ApplicationTypeDef Appli_state;
extern USBH_HandleTypeDef hUsbHostHS;
enum {
	BUFFER_OFFSET_NONE = 0, BUFFER_OFFSET_HALF, BUFFER_OFFSET_FULL,
};
#define AUDIO_BUFFER_SIZE 4096
uint8_t buff[AUDIO_BUFFER_SIZE];
static uint8_t player_state = 0;
static uint8_t buf_offs = BUFFER_OFFSET_NONE;
static uint32_t fpos = 0;

/**
 * @brief  Manages the DMA Half Transfer complete interrupt.
 * @param  None
 * @retval None
 */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
//  xprintf("HalfTransfer_CallBack\n");
	buf_offs = BUFFER_OFFSET_HALF;
}

/**
 * @brief  Calculates the remaining file size and new position of the pointer.
 * @param  None
 * @retval None
 */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
//  xprintf("TransferComplete_CallBack\n");
	buf_offs = BUFFER_OFFSET_FULL;
	BSP_AUDIO_OUT_ChangeBuffer((uint16_t*) &buff[0], AUDIO_BUFFER_SIZE / 2);
}

static void f_disp_res(FRESULT r) {
	switch (r) {
	case FR_OK:
		xprintf("FR_OK\n");
		break;
	case FR_DISK_ERR:
		xprintf("FR_DISK_ERR\n");
		break;
	case FR_INT_ERR:
		xprintf("FR_INT_ERR\n");
		break;
	case FR_NOT_READY:
		xprintf("FR_NOT_READY\n");
		break;
	case FR_NO_FILE:
		xprintf("FR_NO_FILE\n");
		break;
	case FR_NO_PATH:
		xprintf("FR_NO_PATH\n");
		break;
	case FR_INVALID_NAME:
		xprintf("FR_INVALID_NAME\n");
		break;
	case FR_DENIED:
		xprintf("FR_DENIED\n");
		break;
	case FR_EXIST:
		xprintf("FR_EXIST\n");
		break;
	case FR_INVALID_OBJECT:
		xprintf("FR_INVALID_OBJECT\n");
		break;
	case FR_WRITE_PROTECTED:
		xprintf("FR_WRITE_PROTECTED\n");
		break;
	case FR_INVALID_DRIVE:
		xprintf("FR_INVALID_DRIVE\n");
		break;
	case FR_NOT_ENABLED:
		xprintf("FR_NOT_ENABLED\n");
		break;
	case FR_NO_FILESYSTEM:
		xprintf("FR_NO_FILESYSTEM\n");
		break;
	case FR_MKFS_ABORTED:
		xprintf("FR_MKFS_ABORTED\n");
		break;
	case FR_TIMEOUT:
		xprintf("FR_TIMEOUT\n");
		break;
	case FR_LOCKED:
		xprintf("FR_LOCKED\n");
		break;
	case FR_NOT_ENOUGH_CORE:
		xprintf("FR_NOT_ENOUGH_CORE\n");
		break;
	case FR_TOO_MANY_OPEN_FILES:
		xprintf("FR_TOO_MANY_OPEN_FILES\n");
		break;
	case FR_INVALID_PARAMETER:
		xprintf("FR_INVALID_PARAMETER\n");
		break;
	default:
		xprintf("result code unknown (%d = 0x%X)\n", r, r);
	}
}

void Player_Task() {
	/* Infinite loop */

	HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_RESET);

	vTaskDelay(1000);

	xprintf("waiting for USB mass storage\n");

	do {
		xprintf(".");
		vTaskDelay(250);
	} while (Appli_state != APPLICATION_READY);

	flac_example();
	while(1);

	FRESULT res = f_open(&file, "0:/barka.wav", FA_READ);

	if (res == FR_OK) {
		xprintf("wave file open OK\n");
	} else {
		xprintf("wave file open ERROR, res = %d\n", res);
		f_disp_res(res);
		while (1);
	}

	if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, 70, 44100) == 0) {
		xprintf("audio init OK\n");
	} else {
		xprintf("audio init ERROR\n");
	}

	/* Infinite loop */
	for (;;) {

		char key = debug_inkey();

		if (key) {
			xprintf("kod znaku to %02X\n", (unsigned int) key);
		}

		switch (key) {
		case 'p': {
			xprintf("play command...\n");
			if (player_state) {
				xprintf("already playing\n");
				break;
			}
			player_state = 1;
			BSP_AUDIO_OUT_Play((uint16_t*) &buff[0], AUDIO_BUFFER_SIZE);
			fpos = 0;
			buf_offs = BUFFER_OFFSET_NONE;
			break;
		}
		}

		//  	xprintf("buf_offs=%d\n",buf_offs);
		//  	xprintf("player_state=%d\n",player_state);

		if (player_state) {
			uint32_t br;

			if (buf_offs == BUFFER_OFFSET_HALF) {
				if (f_read(&file, &buff[0], AUDIO_BUFFER_SIZE / 2, (void*) &br) != FR_OK) {
					BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
					xprintf("f_read error on half\n");
				}

				buf_offs = BUFFER_OFFSET_NONE;
				fpos += br;
			}

			if (buf_offs == BUFFER_OFFSET_FULL) {
				if (f_read(&file, &buff[AUDIO_BUFFER_SIZE / 2], AUDIO_BUFFER_SIZE / 2, (void*) &br) != FR_OK) {
					BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
					xprintf("f_read error on full\n");
				}

				buf_offs = BUFFER_OFFSET_NONE;
				fpos += br;
			}

//			if( br < AUDIO_BUFFER_SIZE/2 ) {
//				xprintf("stop at eof\n");
//				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//				player_state = 0;
//			}
		}
	}
}

#include "fatfs.h"
#include "term_io.h"
#include "usbh_platform.h"
#include "stm32f4_discovery_audio.h"
#include "flac.h"
#include "flac_adapter.h"

extern ApplicationTypeDef Appli_state;
extern USBH_HandleTypeDef hUsbHostHS;

#define AUDIO_BUFFER_UNIT_SIZE 2048
#define AUDIO_BUFFER_UNIT_COUNT 32
#define AUDIO_BUFFER_SIZE (AUDIO_BUFFER_UNIT_COUNT * AUDIO_BUFFER_UNIT_SIZE)
uint8_t buff[AUDIO_BUFFER_SIZE];

static FIL file;
static Flac* flac;
static FlacAdapter flac_adapter;

static uint8_t player_state = 0;
static uint32_t fpos = 0;

static uint8_t write_offset = 0;
static uint8_t read_offset = 0;

/**
 * @brief  Manages the DMA Half Transfer complete interrupt.
 * @param  None
 * @retval None
 */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
	read_offset = (read_offset + 1) % AUDIO_BUFFER_UNIT_COUNT;
	BSP_AUDIO_OUT_ChangeBuffer((uint16_t*) &buff[AUDIO_BUFFER_UNIT_SIZE * read_offset], AUDIO_BUFFER_UNIT_SIZE);
}

/**
 * @brief  Calculates the remaining file size and new position of the pointer.
 * @param  None
 * @retval None
 */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
	read_offset = (read_offset + 1) % AUDIO_BUFFER_UNIT_COUNT;
	BSP_AUDIO_OUT_ChangeBuffer((uint16_t*) &buff[AUDIO_BUFFER_UNIT_SIZE * read_offset], AUDIO_BUFFER_UNIT_SIZE);
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

void Player_Setup() {
	xprintf("Initializing audio...\n");
	if(BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, 70, 44100)) {
		xprintf("ERROR: cannot initialize audio\n");
		while(1);
	}
	xprintf("Initializing audio OK\n");

// FLAC start
	xprintf("Opening file...\n");
	FRESULT res = f_open(&file, "barka.flac", FA_READ);
	if(res != FR_OK) {
		xprintf("ERROR: cannot open file\n");
		while(1);
	}
	xprintf("Opening file OK\n");

   	flac = Flac_Create();
   	flac->input = &file;
   	flac_adapter = FlacAdapter_Create(flac);
// FLAC end
}

void Player_Task() {
	/* Infinite loop */
	HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_RESET);
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);

	vTaskDelay(1000);

	xprintf("Waiting for USB mass storage");

	do {
		xprintf(".");
		vTaskDelay(250);
	} while (Appli_state != APPLICATION_READY);
	xprintf("\n");

	Player_Setup();

// FLAC start
	xprintf("Decoding metadata...\n");
	if(Flac_GetMetadata(flac)) {
		xprintf("ERROR: decoding metadata\n");
		while(1);
	}
	xprintf("Decoding metadata OK\n");
// FLAC end

// WAVE start
//	FRESULT fr = f_open(&file, "0:/barka.wav", FA_READ);
//	f_disp_res(fr);
//	if (f_read(&file, &buff[0], AUDIO_BUFFER_SIZE, NULL) != FR_OK) {
//		xprintf("f_read error\n");
//	}
// WAVE end


// FLAC start
	FlacAdapter_Get(&flac_adapter, &buff[0], AUDIO_BUFFER_SIZE);
// FLAC end
	write_offset = 0;
	BSP_AUDIO_OUT_Play((uint16_t*) &buff[0], AUDIO_BUFFER_UNIT_SIZE);

	read_offset = 0;
	while(1) {
		if(write_offset != read_offset) {
// WAVE start
//			if (f_read(&file, &buff[write_offset * AUDIO_BUFFER_UNIT_SIZE], AUDIO_BUFFER_UNIT_SIZE, NULL) != FR_OK) {
//				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//				xprintf("f_read error\n");
//			}
// WAVE end
// FLAC start
			if(FlacAdapter_Get(&flac_adapter, &buff[write_offset * AUDIO_BUFFER_UNIT_SIZE], AUDIO_BUFFER_UNIT_SIZE) < AUDIO_BUFFER_UNIT_SIZE) {
				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
			}
			write_offset = (write_offset + 1) % AUDIO_BUFFER_UNIT_COUNT;
		}
// FLAC end
	}

	while(1);
}

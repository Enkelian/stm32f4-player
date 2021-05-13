/*
 * This example shows how to use libFLAC to decode a FLAC file to a WAVE
 * file.  It only supports 16-bit stereo files.
 *
 * Complete API documentation can be found at:
 *   http://xiph.org/flac/api/
 */

#include "term_io.h"
#include "fatfs.h"
#include "share/compat.h"
#include "FLAC/stream_decoder.h"
#include "flac.h"

static FLAC__StreamDecoderReadStatus read_callback(
	    const FLAC__StreamDecoder *decoder,
	    FLAC__byte *buffer,
	    size_t *bytes,
	    void *client_data);
static FLAC__StreamDecoderWriteStatus write_callback(
		const FLAC__StreamDecoder *decoder,
		const FLAC__Frame *frame,
		const FLAC__int32 *const buffer[],
		void *client_data);
static void metadata_callback(
		const FLAC__StreamDecoder *decoder,
		const FLAC__StreamMetadata *metadata,
		void *client_data);
static void error_callback(
		const FLAC__StreamDecoder *decoder,
		FLAC__StreamDecoderErrorStatus status,
		void *client_data);

static FLAC__uint64 total_samples = 0;
static unsigned sample_rate = 0;
static unsigned channels = 0;
static unsigned bps = 0;
static FIL file;

//static FLAC__bool write_little_endian_uint16(FILE *f, FLAC__uint16 x) {
//	return fputc(x, f) != EOF && fputc(x >> 8, f) != EOF;
//}
//
//static FLAC__bool write_little_endian_int16(FILE *f, FLAC__int16 x) {
//	return write_little_endian_uint16(f, (FLAC__uint16) x);
//}
//
//static FLAC__bool write_little_endian_uint32(FILE *f, FLAC__uint32 x) {
//	return	fputc(x, f) != EOF &&
//			fputc(x >> 8, f) != EOF &&
//			fputc(x >> 16, f) != EOF &&
//			fputc(x >> 24, f) != EOF;
//}



static Flac flac;

Flac* Flac_Create() {
//	Flac* flac = (Flac*) malloc(sizeof(Flac));

	// create an instance of a decoder with default settings
	FLAC__StreamDecoder* decoder;
	decoder = FLAC__stream_decoder_new();
	if(decoder == NULL) {
		xprintf("ERROR: allocating decoder\n");
		return NULL;
	}

	// here FLAC_stream_decoder_set_* can be invoked to override default settings

	// TODO: investigate whether this one is needed
	//	(void) FLAC__stream_decoder_set_md5_checking(decoder, true);

	flac.decoder = decoder;

	// initialize the instance to validate the settings and prepare for decoding
	// decode FLAC data from the client via callbacks
	FLAC__StreamDecoderInitStatus init_status = FLAC__stream_decoder_init_stream(
        decoder,
		&read_callback,
        NULL, // seek_callback
        NULL, // tell_callback
        NULL, // length_callback
        NULL, // eof_callback
        &write_callback,
        &metadata_callback,
        &error_callback,
        &flac // client_data
    );

	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		xprintf("ERROR: initializing decoder: %s\n", FLAC__StreamDecoderInitStatusString[init_status]);
		return NULL;
	}

	return &flac;
}

void Flac_Delete(Flac* flac) {
	if(flac != NULL) {
		if(flac->decoder != NULL) {
			// according to docs it is good practice to match every stream_decoder_init_* call with stream_decoder_finish
			FLAC__stream_decoder_finish(flac->decoder);
			FLAC__stream_decoder_delete(flac->decoder);
		}
		free(flac);
	}
}

int Flac_GetMetadata(Flac* flac) {
	FLAC__bool is_success = FLAC__stream_decoder_process_until_end_of_metadata(flac->decoder);
	if(!is_success) {
		return 1;
	}
	return 0;
}

int Flac_GetFrame(Flac* flac, FlacFrame** frame) {
	if(!FLAC__stream_decoder_process_single(flac->decoder)) {
		return 1;
	}
	if(flac->read_frame == NULL) {
		return 1;
	}

	*frame = flac->read_frame;
	flac->read_frame = NULL;
	return 0;
}

int flac_example() {
	//	UINT bytes_read;
	//	int len = 10;
	//	void * buf = malloc(len * sizeof(int));
	//
	//	FRESULT rc = f_read(&file, buf, (UINT) len, &bytes_read);
	//
	//	xprintf("%d bytes read", bytes_read);

	Flac_Create();

	const char* input_file = "0:/barka.flac";
	FIL* file = malloc(sizeof(FIL));
	FRESULT res = f_open(file, input_file, FA_READ);


	flac.input = &file;

	/*
	 * process the stream from the current location until the read callback returns
	 * FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM or FLAC__STREAM_DECODER_READ_STATUS_ABORT
	 * client will get one metadata, write, or error callback per metadata block, audio frame, or sync error, respectively
	 */
	FLAC__bool is_success = FLAC__stream_decoder_process_until_end_of_stream(flac.decoder);
	if(!is_success) {
		FLAC__StreamDecoderState state = FLAC__stream_decoder_get_state(flac.decoder);
		xprintf("ERROR: while decoding stream: %s\n", FLAC__StreamDecoderStateString[state]);
	} else {
		xprintf("stream decoded successfully\n");
	}

//	if(!FLAC__stream_decoder_process_single(flac->decoder)) {
//		FLAC__StreamDecoderState state = FLAC__stream_decoder_get_state(flac->decoder);
//		xprintf("ERROR: while decoding frame: %s\n", FLAC__StreamDecoderStateString[state]);
//	} else {
//		xprintf("frame decoded successfully\n");
//	}

//	Flac_Delete(flac);

	return 0;
}

FLAC__StreamDecoderReadStatus read_callback(
	    const FLAC__StreamDecoder *decoder,
	    FLAC__byte *buffer,
	    size_t *bytes,
	    void *client_data) {
//	xprintf("read_callback, bytes = %d\n", *bytes);

//	Flac* flac = (Flac*) client_data;

    if (*bytes <= 0) {
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }

    UINT bytes_read;
    FRESULT rc = f_read(flac.input, buffer, (UINT) *bytes, &bytes_read);
    if (rc != FR_OK) {
        xprintf("ERROR: cannot read file\n");
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }

//    xprintf("read successful, %d bytes read\n", bytes_read);

    if(bytes_read > 0) {
    	*bytes = (size_t) bytes_read;
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    } else if(bytes_read == 0) {
    	*bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    } else {
    	*bytes = 0;
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
}

FLAC__StreamDecoderWriteStatus write_callback(
		const FLAC__StreamDecoder *decoder,
		const FLAC__Frame *frame, // description of the decoded frame
		const FLAC__int32 *const buffer[], // array of pointers to decoded channels of data
		void *client_data) {
//	xprintf("write_callback\n");
	Flac* flac = (Flac*) client_data;

    for(int i = 0; i < frame->header.channels; i++) {
        if (buffer[i] == NULL) {
        	xprintf("ERROR: buffer is null\n");
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
    }

    int samples = frame->header.blocksize;
    int channels = frame->header.channels;
    int bytes_per_sample = frame->header.bits_per_sample / 8;
    int size = samples * channels * bytes_per_sample;

    flac->read_frame = malloc(sizeof(FlacFrame));
    *flac->read_frame = (FlacFrame) {
        .size = size,
        .data = malloc(size)
    };

    for (int sample = 0; sample < samples; sample++) {
        for (int channel = 0; channel < channels; channel++) {
            for (int byte = 0; byte < bytes_per_sample; byte++) {
                flac->read_frame->data[(sample * channels + channel) * bytes_per_sample + byte] =
                    (uint8_t)((buffer[channel][sample] >> (byte * 8)) & 0xFF);
            }
        }
    }

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;

//	const FLAC__uint32 total_size = (FLAC__uint32) (total_samples * channels * (bps / 8));
//	size_t i;
//
//	(void) decoder;
//
//	if (total_samples == 0) {
//		xprintf("ERROR: this example only works for FLAC files that have a total_samples count in STREAMINFO\n");
//		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
//	}
//	if (channels != 2 || bps != 16) {
//		xprintf("ERROR: this example only supports 16bit stereo streams\n");
//		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
//	}
//	if (frame->header.channels != 2) {
//		xprintf("ERROR: This frame contains %d channels (should be 2)\n", frame->header.channels);
//		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
//	}
//	if (buffer[0] == NULL) {
//		xprintf("ERROR: buffer [0] is NULL\n");
//		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
//	}
//	if (buffer[1] == NULL) {
//		xprintf("ERROR: buffer [1] is NULL\n");
//		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
//	}

	/* write WAVE header before we write the first frame */
//	if (frame->header.number.sample_number == 0) {
//		if (
//			fwrite("RIFF", 1, 4, f) < 4 ||
//			!write_little_endian_uint32(f, total_size + 36) ||
//			fwrite("WAVEfmt ", 1, 8, f) < 8 ||
//			!write_little_endian_uint32(f, 16) ||
//			!write_little_endian_uint16(f, 1) ||
//			!write_little_endian_uint16(f, (FLAC__uint16) channels) ||
//			!write_little_endian_uint32(f, sample_rate) ||
//			!write_little_endian_uint32(f, sample_rate * channels * (bps / 8)) ||
//			!write_little_endian_uint16(f, (FLAC__uint16) (channels * (bps / 8))) || /* block align */
//			!write_little_endian_uint16(f, (FLAC__uint16) bps) ||
//			fwrite("data", 1, 4, f) < 4 ||
//			!write_little_endian_uint32(f, total_size)) {
//			xprintf("ERROR: write error\n");
//			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
//		}
//	}

	/* write decoded PCM samples */
//	for (i = 0; i < frame->header.blocksize; i++) {
//		xprintf("Frame no: %d\n", i);
//		if (
//			!write_little_endian_int16(f, (FLAC__int16) buffer[0][i]) || /* left channel */
//			!write_little_endian_int16(f, (FLAC__int16) buffer[1][i]) /* right channel */
//		) {
//			xprintf("ERROR: write error\n");
//			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
//		}
//	}
}

void metadata_callback(
		const FLAC__StreamDecoder *decoder,
		const FLAC__StreamMetadata *metadata,
		void *client_data) {
	//	(void) decoder, (void) client_data;
	xprintf("metadata_callback\n");

	/* print some stats */
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		/* save for later */
		total_samples = metadata->data.stream_info.total_samples;
		sample_rate = metadata->data.stream_info.sample_rate;
		channels = metadata->data.stream_info.channels;
		bps = metadata->data.stream_info.bits_per_sample;

		xprintf("total samples  : %" PRIu64 "\n", total_samples);
		xprintf("sample rate    : %u Hz\n", sample_rate);
		xprintf("channels       : %u\n", channels);
		xprintf("bits per sample: %u\n", bps);
	}
}

void error_callback(
	const FLAC__StreamDecoder *decoder,
	FLAC__StreamDecoderErrorStatus status,
	void *client_data) {
	(void) decoder, (void) client_data;

	xprintf("Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

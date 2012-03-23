
/* Static function prototypes */
static void *audeng_mad_play ();
static enum mad_flow audeng_mad_input (void *data, struct mad_stream *stream);
static enum mad_flow audeng_mad_output (void *data, 
	struct mad_header const *header, struct mad_pcm *pcm);
static int audeng_mp3_bufferframes ();
static void audeng_mp3_findframe (FILE *file, int offset, audeng_mp3_frame *frame);
static int audeng_getxing ();
static void audeng_mp3_scan ();

void audeng_mp3_init () {
	audeng_mp3.play = audeng_mp3_play;
	audeng_mp3.pause = audeng_mp3_pause;
	audeng_mp3.resume = audeng_mp3_resume;
	audeng_mp3.stop = audeng_mp3_stop;
	audeng_mp3.seek = audeng_mp3_seek;
	audeng_mp3.getpos = audeng_mp3_getpos;
	audeng_mp3.getlen = audeng_mp3_getlen;
}

void audeng_mp3_play () {
	audeng_mp3_scan();
	audeng_mode = playing;
	// Launch decoder in separate thread
	pthread_create(&audeng_mp3_thread, NULL, audeng_mad_play, NULL);
}

void audeng_mp3_pause () {
	audeng_mode = paused;
}

void audeng_mp3_resume () {
	audeng_mode = playing;
}

void audeng_mp3_stop () {
	audeng_song_pos = audeng_seek_pos = audeng_sample_pos = 0;
	audeng_mode = stopped;
	/* Wait for the decoder thread to exit */
	pthread_join(audeng_mp3_thread, 0);
	//pthread_detach(audeng_mp3_thread);
	// *Does anything else needs to be cleared/reset?
	//	such as...threads, structs
}

// Seek to position in 1/10 seconds (ex. position of 1000 is 1:40 or 100 seconds)
void audeng_mp3_seek (int position) {
	if (audeng_mp3.vbr < 2) {	// No Xing TOC, so just seek CBR style
		// Just get position as percentage of file duration and multiply by size of file in bytes
		audeng_seek_pos = (position * 100 / audeng_mp3.duration) * audeng_mp3.size / 100;
	} else {
		// Make a better educated guess with Xing TOC
		/* Similar to CBR but we look up the percentage in xing, 
		   which is per 255, and apply that to the file size 	*/
		audeng_seek_pos = (audeng_mp3.size * 100 / 255) * audeng_mp3.xing[position * 100  / audeng_mp3.duration] / 100;
	}
	audeng_sample_pos = position * audeng_mp3.samplerate / 10;
	while (audeng_mode == paused) {}
	audeng_mode = seeking;
}

unsigned int audeng_mp3_getpos () {
	return audeng_sample_pos * 10 / audeng_mp3.samplerate;
}

unsigned int audeng_mp3_getlen () {
	return audeng_mp3.duration;
}

void *audeng_mad_play () {
	struct mad_decoder decoder;
	int result;
	
	// Initiliaze decoder
	mad_decoder_init(&decoder, &audeng_buffer, audeng_mad_input, 0, 0, audeng_mad_output, 0, 0);
	// Run the decoder
	result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
	// Clean up
	mad_decoder_finish(&decoder);
	pthread_exit(0);
}

static enum mad_flow audeng_mad_input (void *data, struct mad_stream *stream) {
	while (audeng_mode == paused) {}
	switch (audeng_mode) {
		case seeking:
			audeng_song_pos = audeng_seek_pos;
			audeng_mode = playing;
		case playing:
			audeng_mp3_bufferframes();
			mad_stream_buffer(stream, audeng_buffer, audeng_buffer_sz);
			return MAD_FLOW_CONTINUE;
		case stopped:
			/* Just so it's visible through all the clutter */
			return MAD_FLOW_STOP;
		default:
			return MAD_FLOW_CONTINUE;
	}
}

static enum mad_flow audeng_mad_output (void *data, struct mad_header const *header, struct mad_pcm *pcm) {
	unsigned int nsamples = pcm->length;
	mad_fixed_t const *left_ch, *right_ch;
	static struct audeng_dither_t dither;
	
	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];
	audeng_sample_pos += 1152;
	if (audeng_mode == stopped) {
		return MAD_FLOW_STOP;
	}
	//return MAD_FLOW_CONTINUE;
	
	while (nsamples--) {
		signed int sample;
		sample = audio_linear_dither(16, *left_ch++, &dither);
		putc(sample, audeng_output_fd);
		putc(sample >> 8, audeng_output_fd);
		if (pcm->channels == 2) {
			sample = audio_linear_dither(16, *right_ch++, &dither);
			putc(sample, audeng_output_fd);
			putc(sample >> 8, audeng_output_fd);
		} else {
			sample = audio_linear_dither(16, *left_ch++, &dither);
			putc(sample, audeng_output_fd);
			putc(sample >> 8, audeng_output_fd);
		}
	}

	return MAD_FLOW_CONTINUE;
}

static int audeng_mp3_bufferframes () {
	audeng_mp3_frame cur_frame;
	int x;
	
	audeng_mp3_findframe(audeng_song, audeng_song_pos, &cur_frame);
	audeng_song_pos = cur_frame.start;
	x = cur_frame.size;
	while (x < audeng_buff_sz) {
		audeng_mp3_findframe(audeng_song, audeng_song_pos + x, &cur_frame);
		x += cur_frame.size;
	}
	x -= cur_frame.size;
	fseek(audeng_song, audeng_song_pos, SEEK_SET);
	fread(audeng_buffer, audeng_buff_sz, 1, audeng_song);
	audeng_song_pos += x;
	audeng_buffer_sz = audeng_buff_sz;
	
	return 0;
}

static void audeng_mp3_findframe (FILE *file, int offset, audeng_mp3_frame *frame) {
	unsigned char temp[2];
	unsigned char result = 0;
	fseek(file, offset, SEEK_SET);
	while (fread(&temp, 2, 1, audeng_song)) {
		offset += 2;
		if (temp[0] == 0xFF && (temp[1] == 0xFA || temp[1] == 0xFB)) {	// Find Frame Sync
			frame->crc = (temp[1] & 1) ? 0 : 1;
			fread(&temp, 2, 1, audeng_song);
			offset += 2;
			frame->bitrate = temp[0] >> 4;
			frame->samplerate = mp3_samplerates[(temp[0] & 15) >> 2];
			frame->padding = (temp[0] & 3) >> 1;
			frame->channelmode = temp[1] >> 6;
			frame->bitrate = mp3_bitrates[frame->bitrate];
			if (frame->samplerate > 0 && frame->bitrate > 0) {	// Check for problems
				frame->size = 144 * frame->bitrate * 1000 / frame->samplerate + frame->padding;
				frame->start = offset - 4;
				fseek(audeng_song, frame->start + frame->size, SEEK_SET);
				fread(&temp, 2, 1, audeng_song);	// BUG: will not work on last frame
				if (temp[0] == 0xFF && (temp[1] == 0xFA || temp[1] == 0xFB)) {
					result = 1;	// Success
				}
			}
		} else if (temp[1] == 0xFF) {
			offset--;
		}
		fseek(audeng_song, offset, SEEK_SET);
		if (result) {	// Valid frame found so return it...
			return;
		}
	}
}

static int audeng_getxing () {
	int xing_start;
	char temp[4];
	unsigned int tempi;
	audeng_mp3_frame first_frame;
	audeng_mp3_findframe(audeng_song, 0, &first_frame);
	switch (first_frame.channelmode) {
		case Mono:
			xing_start = 17;
			break;
		default:	// All other modes (stereo/joint stereo/dual channel)
			xing_start = 32;
	}
	xing_start += first_frame.start + 4;
	fseek(audeng_song, xing_start, SEEK_SET);
	fread(&tempi, 4, 1, audeng_song);
	if (tempi == 0x676E6958) {	// Check for Xing Header
		fread(&temp, 4, 1, audeng_song);
		/* Finds start of Xing TOC by adding 4 bytes for each optional field, 
		   indicated by bit 0 (Frames) and bit 1 (Bytes)					*/
		xing_start += (temp[3] & 2) * 2 + (temp[3] & 1) * 4;
		if (temp[3] & 2) {	// Get frame count to calculate song duration
			fread(&temp, 4, 1, audeng_song);
			tempi = (temp[0] << 24) + (temp[1] << 16) + (temp[2] << 8) + temp[3];
			audeng_mp3.duration = tempi / (audeng_mp3.samplerate / 1152) * 10;
		}
		fseek(audeng_song, xing_start, SEEK_SET);
		fread(&(audeng_mp3.xing), 100, 1, audeng_song);		// Get Xing TOC
		return 1;	// Xing found...definately vbr file
	}
	return 0;	// No Xing found...treat it as a CBR file
}

// Basically retrieve file info (easy for CBR), and get Xing TOC (for VBR)
static void audeng_mp3_scan () {
	struct stat file_info;
	unsigned int avg_bitrate = 0;
	audeng_mp3_frame cur_frame;
	unsigned char i;
	unsigned char sec;
	
	stat(audeng_mp3.path, &file_info);
	audeng_mp3.size = file_info.st_size;
	audeng_mp3_findframe(audeng_song, 0, &cur_frame);
	audeng_mp3.samplerate = cur_frame.samplerate;
	audeng_mp3.channelmode = cur_frame.channelmode;
	if (audeng_getxing()) {	// Check if it's VBR
		// Skim through the file to get an average bitrate
		for (i = 1; i < 100; i ++) {
			audeng_mp3_findframe(audeng_song, audeng_mp3.size * i / 100, &cur_frame);
			avg_bitrate += cur_frame.bitrate;
		}
		audeng_mp3.bitrate = avg_bitrate / 99;
		audeng_mp3.vbr = 2;
	} else {
		audeng_mp3.bitrate = cur_frame.bitrate;
		audeng_mp3.duration = audeng_mp3.size / (audeng_mp3.bitrate * 125) * 10;
	}
	audeng_debug("audeng_mp3 file info:\n", audeng_mp3.path);
	audeng_debug("\tFile name: %s\n", audeng_mp3.path);
	audeng_debug("\tBitrate: %dKbps %s\n", audeng_mp3.bitrate, audeng_mp3.vbr ? "VBR":"");
	audeng_debug("\tFilesize: %u bytes\n", (unsigned int)file_info.st_size);
	sec = audeng_mp3.duration / 10 % 60;
	audeng_debug("\tDuration: %u:%s%u\n", audeng_mp3.duration / 600, sec < 10 ? "0": "", sec);
}

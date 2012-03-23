			frame->version = (temp[1] & 31) >> 3;
			frame->layer = (temp[1] & 7) >> 1;

			switch (frame->version) {		// Check MPEG version
				case VReserved:		// Reserved (invalid)
					error = 1;
					break;
				case V1:		// MPEG 1
					switch (frame->layer) {
						case L3:		// Layer 3
							frame->bitrate = mpeg_bitrates[2 + 5 * frame->bitrate];
							break;
						case L2:		// Layer 2
							frame->bitrate = mpeg_bitrates[1 + 5 * frame->bitrate];
							break;
						case L1:		// Layer 1
							frame->bitrate = mpeg_bitrates[0 + 5 * frame->bitrate];
					}
					break;
				case V25:		// MPEG 2.5
				case V2:		// MPEG 2 (same thing for bitrates)
					switch (frame->layer) {
						case L3:		// Layer 3
						case L2:		// Layer 2
							frame->bitrate = mpeg_bitrates[4 + 5 * frame->bitrate];
							break;
						case L1:		// Layer 1
							frame->bitrate = mpeg_bitrates[3 + 5 * frame->bitrate];
							break;
					}
			}
			
			enum mpeg_version {
	V25			= 0,
	VReserved	= 1,
	V2			= 2,
	V1			= 3
};

enum mpeg_layer {
	LReserved	= 0,
	L3			= 1,
	L2			= 2,
	L1			= 3
};


		switch (mp3.layer) {
			case L1:
				framesize = 384;
				break;
			case L2:
				framesize = 1152;
				break;
			case L3:
				switch (mp3.version) {
					case V1:
						framesize = 1152;
						break;
					case V2:
					case V25:
						framesize = 576;
				}
		}
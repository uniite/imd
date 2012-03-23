/*******************************
 IMD Input v2
 * Created Feb 3, 2008
 * by Jon Botelho
********************************/

/* GPIO assignments */
# define imd_input_3way_up 58
# define imd_input_3way_center 59
# define imd_input_3way_down 60

/* IMD main loop: checks for input and handles gui */
void imd_main () {

	# ifdef imd_debug_stdin
	/* Use stdin (numpad) instead of GPIOs for input */
	goto use_stdin;
	# endfif
	
	/* Says whether acceleroemeter has been zero'd */
	int accel_ok = 0;
	/* Relative zeros of accelerometer */
	int accel_x0;
	int accel_y0;
	int accel_z0;
	/* Accelerometer readings */
	int accel_x;
	int accel_y;
	int accel_z;
	/* Senstivity of tilting */
	int accel_sens = 15;
	
	/* Setup GPIOs */
	gpio_function(imd_input_3way_up, GPIO);
	gpio_function(imd_input_3way_center, GPIO);
	gpio_function(imd_input_3way_down, GPIO);
	
	gpio_direction(imd_input_3way_up, IN);
	gpio_direction(imd_input_3way_center, IN);
	gpio_direction(imd_input_3way_down, IN);
	
	/* Set up the ADC */
	FILE *imd_input_AD0 = fopen("/sys/class/ucb1x00/ucb1x00/AD0", "r");
	FILE *imd_input_AD1 = fopen("/sys/class/ucb1x00/ucb1x00/AD1", "r");
	FILE *imd_input_AD2 = fopen("/sys/class/ucb1x00/ucb1x00/AD2", "r");
	//FILE *imd_input_AD3 = fopen("/sys/class/ucb1x00/ucb1x00/AD3", "r", imd_input_AD3);
	
	/* Poll inputs */
	int val;
	while (1) {
		/* If center pressed, use the accelerometer */
		if (!gpio_status(imd_input_3way_center)) {
			/* Re-open ADC files to refresh readings */
			freopen("/sys/class/ucb1x00/ucb1x00/AD0", "r", imd_input_AD0);
			freopen("/sys/class/ucb1x00/ucb1x00/AD1", "r", imd_input_AD1);
			freopen("/sys/class/ucb1x00/ucb1x00/AD2", "r", imd_input_AD2);
			//freopen("/sys/class/ucb1x00/ucb1x00/AD3", "r", imd_input_AD3);
			/* Get the actual readings */
			fscanf(imd_input_AD0, "%d\n", &accel_x);
			fscanf(imd_input_AD1, "%d\n", &accel_y);
			fscanf(imd_input_AD2, "%d\n", &accel_z);
			
			if (accel_ok) {
				/* See if we have any tilt triggers */
				if (accel_x > accel_x0 + accel_sens) {
					/* Tilt X+ */
					/* Menu up */
					imd_scr_event(3);
				} else if (accel_x < accel_x0 - accel_sens) {
					/* Tilt X- */
					/* Menu down */
					imd_scr_event(2);
				} else if (accel_y > accel_y0 + accel_sens) {
					/* Tilt left */
					imd_scr_event(0);
				} else if (accel_y < accel_y0 - accel_sens) {
					/* Tilt right */
					imd_scr_event(1);
				}
				usleep(100000);
			} else {
				/* Get the baseline readings */
				accel_x0 = accel_x;
				accel_y0 = accel_y;
				accel_z0 = accel_z;
				/*printf("accel_x0: %d\n", accel_x0);
				printf("accel_y0: %d\n", accel_y0);
				printf("accel_z0: %d\n", accel_z0);*/
				accel_ok = 1;
			}
		} else {
			/* Accelerometer not in use */
			accel_ok = 0;
			
			/* Check other buttons */
			if (!gpio_status(imd_input_3way_up)) {
				/* Volume up */
				imd_scr_event(8);
			} if (!gpio_status(imd_input_3way_down)) {
				/* Volume down */
				imd_scr_event(10);
			}
		}
		usleep(100000);
	}



use_stdin:

	/* Use stdin for input */
	char buff;
	while (read(0, &buff, 1)) {
		switch (buff) {
			case '8': /* Tilt up */
				imd_scr_event(2);
				break;
			case '2': /* Tilt down */
				imd_scr_event(3);
				break;
			case '4': /* Tilt left */
				imd_scr_event(0);
				break;
			case '6': /* Tilt right */
				imd_scr_event(1);
				break;
			case '7': /* Tilt up+down */
				imd_scr_event(4);
				break;
			case '1': /* Tilt left+right */
				imd_scr_event(5);
				break;
			case '9': /* 3-way switch up */
				imd_scr_event(8);
				break;
			case '3': /* 3-way Switch down */
				imd_scr_event(10);
				break;
			case 'q': /* Quit */
				/* Useful for debug system... */
				execlp("sh", "sh", "-", 0);
				return;
		}
		usleep(100000);
		//printf("Cursor: %u\n", menu.cursor);
		//oled_flush();
	}
}

/*
 * Send a file over a specified serial port at the given speed with
 * an optional specified delay after a specified number of characters.
 * No flow control is required or used. Files are sent "straight" and
 * no checksum or transfer protocol is employed. Connections are 8-N-1.
 * Connections must be 8-bit safe to send binary data.
 *
 * Usage: ./sendfile port speed filename [everychars,everydelay_ms]
 * where optional everychars,everydelay_ms = delay everydelay_ms after
 * everychars bytes sent. If unspecified, then there is no pause.
 * Example: ./sendfile /dev/ttyUSB0 9600 readme.txt 1,10
 * Example: ./sendfile /dev/ttyUSB0 9600 bletch.bin
 *
 * Copyright (C) 2024 Cameron Kaiser. All rights reserved.
 * BSD license.
 * http://oldvcr.blogspot.com/
 */

#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

struct termios tty, tty_saved, tty_saved_in, tty_saved_out, tty_saved_err;
struct sigaction sig;
int port;

speed_t
baudr(int baud) {
	switch (baud) {
		case 300: return B300;
		case 600: return B600;
		case 1200: return B1200;
		case 2400: return B2400;
		case 4800: return B4800;
		case 9600: return B9600;
		case 19200: return B19200;
		case 38400: return B38400;
		case 57600: return B57600;
		case 115200: return B115200;
		default: return B0;
	}
	return B0; /* not reached */
}

void
cleanup()
{
	fprintf(stderr, "restoring terminal settings\n");
	(void)tcsetattr(port, TCSAFLUSH, &tty_saved);
	(void)close(port);
	(void)tcsetattr(STDIN_FILENO, TCSANOW, &tty_saved_in);
	(void)tcsetattr(STDOUT_FILENO, TCSANOW, &tty_saved_out);
	(void)tcsetattr(STDERR_FILENO, TCSANOW, &tty_saved_err);
}

void
deadme()
{
	fprintf(stderr, "terminating on user signal\n");
	exit(0);
}

int
main(int argc, char **argv)
{
	unsigned int every, delay, ccount, dcount;
	struct timeval tv;
	int port, speed;
	unsigned char c;
	speed_t speedd;
	fd_set wfd;
	FILE *f;

	/* it's not enough to just restore the serial port settings */
	if (tcgetattr(STDIN_FILENO, &tty_saved_in) ||
			tcgetattr(STDOUT_FILENO, &tty_saved_out) ||
			tcgetattr(STDERR_FILENO, &tty_saved_err)) {
		perror("tcgetattr (tty)");
		return 1;
	}

	if (argc < 4) {
		fprintf(stderr, "usage: port speed filename [everychars,everydelay_ms]\n");
		return 1;
	}
	speed = atoi(argv[2]);
	if (!speed || ((speedd = baudr(speed)) == B0)) {
		fprintf(stderr, "unsupported baud rate %d\n", speed);
		return 1;
	}
	if (!(f = fopen(argv[3], "rb"))) {
		perror("fopen");
		return 255;
	}
	if (argc > 4) {
		every = 0;
		delay = 0;
		if (sscanf(argv[4], "%u,%u", &every, &delay) != 2 || !every || !delay) {
			fprintf(stderr, "nonsensical %u delay for %u characters\n", delay, every);
			if (!every && !delay)
				fprintf(stderr, "(omit speed parameter if you want no delays)\n");
			return 255;
		}
		fprintf(stderr, "delaying %ums after %u characters\n", delay, every);
		if (argc > 5)
			fprintf(stderr, "warning: excess arguments after delay specifier\n");
		delay *= 1000; /* for usleep */
	} else {
		every = 0;
		delay = 0;
	}

	fprintf(stderr, "opening %s\n", argv[1]);
	port = open(argv[1], O_RDWR | O_NOCTTY);
	if (!isatty(port)) {
		fprintf(stderr, "warning: isatty returned false for this path\n");
	}
	fprintf(stderr, "setting up for serial access\n");
	if (tcgetattr(port, &tty)) {
		perror("tcgetattr");
		close(port);
		fclose(f);
		return 1;
	}
	tty_saved = tty;

	fprintf(stderr, "setting flags on serial port fd=%d\n", port);
	memset(&tty, 0, sizeof(tty));
	tty.c_cflag |= (CS8 | CREAD | CLOCAL);
	tty.c_cc[VTIME] = 5;
	tty.c_cc[VMIN] = 1;
	(void)cfsetospeed(&tty, speedd);
	(void)cfsetispeed(&tty, speedd);
	if (tcsetattr(port, TCSAFLUSH, &tty)) {
		perror("tcsetattr (tty)");
		close(port);
		fclose(f);
		return 1;
	}
	atexit(cleanup);
	sig.sa_handler = deadme;
	sigaction(SIGINT, &sig, NULL);
	sigaction(SIGHUP, &sig, NULL);
	sigaction(SIGTERM, &sig, NULL);

	fprintf(stderr, "sending %s at %dbps\n", argv[3], speed);

	ccount = 0;
	dcount = 0;
	for(;;) {
		/* read and send the byte */
		if (fread(&c, 1, 1, f) != 1) break;
		ccount++;

		/* wait to send if instructed */
		if (every) {
			dcount++;
			if (every == dcount) {
				dcount = 0;
				usleep(delay);
			}
		}

		/* time out if we can't write. check each byte, the
  			connection could be very slow */
		FD_ZERO(&wfd);
		FD_SET(port, &wfd);
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		if (select(port+1, NULL, &wfd, NULL, &tv) < 1) {
			perror("timeout write select on port");
			return 1;
		}
		write(port, &c, 1);
	}
	fprintf(stderr, "%u bytes transmitted at termination\n", ccount);

	if (feof(f)) {
		/* clean exit, atexit restores serial port settings */
		fclose(f);
		return 0;
	}

	/* unclean exit, ring bell */
	perror("fread");
	fclose(f);
	return 255;
}

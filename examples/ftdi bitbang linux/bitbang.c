/* This program is distributed under the GPL, version 2 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ftdi.h>

int main(int argc, char **argv)
{
	struct ftdi_context *ftdi;
	int f,i;
	unsigned char buf[1];
	int retval = 0;

	if ((ftdi = ftdi_new()) == 0)
	{
		fprintf(stderr, "ftdi_new failed\n");
		return EXIT_FAILURE;
	}

	f = ftdi_usb_open(ftdi, 0x0403, 0x6010);

	if (f < 0 && f != -5)
	{
		fprintf(stderr, "unable to open ftdi device: %d (%s)\n", f, ftdi_get_error_string(ftdi));
		retval = 1;
		goto done;
	}

	ftdi_set_bitmode(ftdi, 0, BITMODE_BITBANG);

	while (1) {
		f = ftdi_read_data(ftdi, buf, 1);
		if (f < 0)
		{
			fprintf(stderr, "read failed, error %d (%s)\n", f, ftdi_get_error_string(ftdi));
			break;
		}
		printf("%02hhx ", buf[0]);
		/*
		if (buf[0] == 0xff)
			putchar('-');
		else
			putchar('_');
		*/
	}
	ftdi_disable_bitbang(ftdi);

	ftdi_usb_close(ftdi);
done:
	ftdi_free(ftdi);

	return retval;
}

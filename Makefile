/*
 * I found this source at http://web.archive.org/web/20090418040040/http://www.wiili.org/Wii_bluetooth_specs
 *
 * Edited by Michael Lumish <michael.lumish@gmail.com>
 * 
 * License GPLv3
 */

CFLAGS=-Wall
LIBS=-lbluetooth

hidattack: sdp.o
	$(CC) $(CFLAGS) $(LIBS) sdp.o -o hidattack

clean:
	rm -f *.o hidattack

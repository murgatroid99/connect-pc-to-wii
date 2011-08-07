
CFLAGS=-Wall
LIBS=-lbluetooth

hidattack: sdp.o
	$(CC) $(CFLAGS) $(LIBS) sdp.o -o hidattack

clean:
	rm -f *.o hidattack

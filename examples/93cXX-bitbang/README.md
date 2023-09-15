93c66 microwire ft232 bitbang mode flasher

```
./93cXX chipId [read|write|verify|fill|erase|write-disable] readOffset [readSize|'max']
	read to stdout
	writes from stdin
	verify from stdin and return non 0 if failed
	fill read 1 byte from stdin and fill region. Vcc should be 5v!
	erase erase region. if 0 max then fast erase all cmd is used. Vcc should be 5v!
	write-disable temporary prevent future writing or erasing
		any modification operation unlock it automatically

	accepted env vars FT232__DEV_VID=num FT232__DEV_PID=num FT232__BAUDRATE=num
```


Examples

```
	./93cXX 93c66 read 0 max > out.bin
	./93cXX 93c66 write 0 max < in.bin
	./93cXX 93c66 verify 0 max < in.bin || echo Verification failed
	printf '\077' | ./93cXX 93c66 fill 0 max
	./93cXX 93c66 erase 0 max
	./93cXX 93c66 write-disable 0 max
```

TODO
- ~~support all 93cXX~~
- ~~support word mode~~

Build for linux

```
	make TARGET_EXEC=93cXX PLATFORM=linux 93cXX
	make TARGET_EXEC=93cXX PLATFORM=linux 93cXX-w
```

Build for windows

```
	make TARGET_EXEC=93cXX PLATFORM=windows 93cXX
	make TARGET_EXEC=93cXX PLATFORM=windows 93cXX-w
```

Testing in byte mode (ORG = GND)

```
	make TARGET_EXEC=93cXX PLATFORM=linux test-read
	make TARGET_EXEC=93cXX PLATFORM=linux test-write
	make TARGET_EXEC=93cXX PLATFORM=linux test-verify
	make TARGET_EXEC=93cXX PLATFORM=linux test # write + verify
```

Testing in word mode(ORG = VCC)

```
	make TARGET_EXEC=93cXX-w PLATFORM=linux test-read
	make TARGET_EXEC=93cXX-w PLATFORM=linux test-write
	make TARGET_EXEC=93cXX-w PLATFORM=linux test-verify
	make TARGET_EXEC=93cXX-w PLATFORM=linux test # write + verify
```

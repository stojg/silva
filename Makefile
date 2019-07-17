build:
	platformio run

upload:
	platformio run --target upload

clean:
	platformio run --target clean

test:
	platformio test -e pro16MHzatmega328 --verbose

install:
	# https://libraries.io/platformio/FastLED
	platformio lib install 126

monitor:
	platformio device monitor

.PHONY: build upload clean test install monitor

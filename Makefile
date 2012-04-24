all: bin/unpack

bin/unpack:
	make --no-print-directory -C unpack

.PHONY: bin/unpack

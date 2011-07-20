CGCC    := arm-linux-gcc

clean:
	find . \
	 	\( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' \
		-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \
		-o -name 'drivertester' \) \
		-type f -print | xargs rm -f

all: drivertester

drivertester: drivertester.c
	$(CGCC) -o drivertester drivertester.c


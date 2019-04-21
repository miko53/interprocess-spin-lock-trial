all: interprocess_acq.c interprocess_acq.h reader.c
	gcc -Wall --std=gnu99 -W -Wall -Wextra -Wno-aggregate-return -Wno-suggest-attribute=format -Wno-undef -fms-extensions -Wno-pointer-sign interprocess_acq.c -o acq
	gcc -Wall --std=gnu99 -W -Wall -Wextra -Wno-aggregate-return -Wno-suggest-attribute=format -Wno-undef -fms-extensions -Wno-pointer-sign reader.c -o reader

clean:
	rm -rf acq reader


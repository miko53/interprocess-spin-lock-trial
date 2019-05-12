all: acq reader 

acq:	interprocess_acq.c interprocess_acq.h
	gcc -Wall --std=gnu99 -O2 -W -Wall -Wextra -Wno-aggregate-return -Wno-suggest-attribute=format -Wno-undef -fms-extensions -Wno-pointer-sign interprocess_acq.c -o acq

reader: reader.c interprocess_acq.h
	gcc -Wall --std=gnu99 -O2 -W -Wall -Wextra -Wno-aggregate-return -Wno-suggest-attribute=format -Wno-undef -fms-extensions -Wno-pointer-sign reader.c -o reader

clean:
	rm -rf acq reader


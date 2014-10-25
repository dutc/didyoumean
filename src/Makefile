.PHONY: test test-2.20 test-2.19

CC=gcc -std=c99 -Wall

didyoumean.so: didyoumean.c hook.c safe_PyObject_Dir.c levenshtein.c
	${CC} `python-config --cflags` `python-config --includes` -Wl,--export-dynamic -fPIC -shared -o $@ $^ -ldl `python-config --libs`

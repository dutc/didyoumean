.PHONY: clean

maybeyoumeant.so: maybeyoumeant.c hook.c safe_PyObject_Dir.c levenshtein.c
	gcc `python-config --cflags` `python-config --includes` -Wl,--export-dynamic -fPIC -shared -o $@ $^ -ldl `python-config --libs`

clean:
	rm maybeyoumeant.so

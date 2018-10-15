CFLAGS += -O2 -fPIC -m32
LDFLAGS += -shared

OUT = wine_mesa_discard_profiles.so


$(OUT): wrapper.c
	$(CC) $(CFLAGS) $(LDFLAGS) wrapper.c -o $(OUT)


.PHONY: clean
clean:
	rm -f $(OUT)

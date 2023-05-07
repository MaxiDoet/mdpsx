objs := mdpsx.o log.o cpu/r3000.o mem/mem.o

CFLAGS := -Iinclude -Wbuiltin-declaration-mismatch

all: mdpsx

clean:
	rm -rf $(objs)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

mdpsx: ${objs}
	gcc -o $@ $^ $(CFLAGS)
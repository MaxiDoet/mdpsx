objs := mdpsx.o log.o bios/bios.o cpu/r3000.o bus/bus.o gpu/gpu.o timer/timer.o renderer/renderer.o

CFLAGS := -Iinclude -lglfw -lGL -lSDL2 -lGLEW -lSDL2_image -g3 -O0 # -Wall -Wextra


all: mdpsx

clean:
	rm -rf $(objs)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

mdpsx: ${objs}
	gcc -o $@ $^ $(CFLAGS)
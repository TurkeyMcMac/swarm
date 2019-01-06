executable = swarm
source = swarm.c
num-circle-points = 10
circle-points = circle-points.h
jwb = libjwb.so

c-flags = `sdl2-config --cflags` -O3 -L. -I../just-wheels-bouncing/include \
	$(CFLAGS)
link-flags = -l:./$(jwb) `sdl2-config --libs` -lm

$(executable): $(jwb) $(circle-points) $(source)
	$(CC) $(c-flags) -o $@ $(source) $(link-flags)

$(circle-points):
	./generate-circle-pts $(num-circle-points) > $@

$(jwb):
	@echo "Please provide the library '$(jwb)' in this directory." \
		"Use a symlink or just copy it in."
	@[ ] # This will make the recipe fail always.

.PHONY: clean
clean:
	$(RM) $(executable) $(circle-points)

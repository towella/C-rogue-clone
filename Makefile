build:
	gcc -Wall -Werror -O0 -I/Library/Frameworks/SDL2.framework/Headers main.c -F/Library/Frameworks -framework SDL2 -o "rogue clone"
run:
	./"rogue clone"
clean:
	rm "rogue clone"

#include "game/motion.h"
#include "game/planner.h"
#include "include/SDL_events.h"
#include "gfx/gfx.h"
#include <stdio.h>
#include <string.h>

// this fixes the magic windows bugs
#define SDL_main main

#define EGRESS() do {          \
	printf("what the fuck\n"); \
	exit(-1);                  \
} while(0)

int main(int argc, char *argv[]) {
	char buf[100];
	scanf_s("%s", buf);

	char team  = buf[0];
	char spawn = buf[1];

	StartPos start;
	if (team == 'r') {
		if (spawn == '1') start = RED_ONE; else
		if (spawn == '2') start = RED_TWO; else
		if (spawn == '3') start = RED_THREE;

		else EGRESS();
	} else if (team == 'b') {
		if (spawn == '1') start = BLUE_ONE; else
		if (spawn == '2') start = BLUE_TWO; else
		if (spawn == '3') start = BLUE_THREE;

		else EGRESS();
	} else EGRESS();

	init_gfx(argc > 1 && strcmp(argv[1], "deeeen") == 0);
	init_planner(start);

	SDL_Event e;
	while (1) {
		planner_loop(&e);
	}

	return 0;
}

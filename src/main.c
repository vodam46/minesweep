#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct _tile {
	int mine, surrounding, discovered, flagged;
} tile;

void draw(tile map[], int height, int width) {
	clear();
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (map[y*height + x].discovered) {
				if (!map[y*height + x].mine) {
					attron(COLOR_PAIR(0));
					mvprintw(y+1, x, "%d ", map[y*height + x].surrounding);
					attroff(COLOR_PAIR(0));
				}
				else {
					attron(COLOR_PAIR(1));
					mvaddch(y+1, x, 'X');
					attroff(COLOR_PAIR(1));
				}
			}
			else if (map[y*height + x].flagged) {
				attron(COLOR_PAIR(2));
				mvaddch(y+1, x, 'B');
				attroff(COLOR_PAIR(2));
			} else {
				attron(COLOR_PAIR(0));
				mvaddch(y+1, x, '.');
				attroff(COLOR_PAIR(0));
			}
		}
	}
}

tile* map;
tile* generate_minefield(int height, int width, int mines, int player_y, int player_x) {

	map = calloc(height*width, sizeof(tile));
	for (int i = 0; i < height*width; i++) {
		map[i] = (tile){0, 0, 0, 0};
	}

	for (int x = -1; x <= 1; x++)
		for (int y = -1; y <= 1; y++)
			if (player_y+y >= 0 && player_y+y < height && player_x+x >= 0 && player_x+x < width)
				map[(player_y+y)*width + (player_x+x)].discovered = 1;

	// generate the minefield
	int true_pos = 0;
	for (int i = 0; i < mines; i++) {
		true_pos = 0;
		int position = rand() % ((height) * (width) - i - 9);
		for (int j = 0; j < position;true_pos++) {
			if (!map[true_pos].mine || map[true_pos].discovered)
				j++;
		}
		if (map[true_pos].mine || map[true_pos].discovered) {
			i--;
			continue;
		}

		map[true_pos].mine = 1;
		if (true_pos+width < height*width) {
			map[true_pos+width].surrounding++;
		}
		if (true_pos-width >= 0) {
			map[true_pos-width].surrounding++;
		}
		if (!(true_pos%width+1 == width)) {
			map[true_pos+1].surrounding++;
		}
		if (!(true_pos%width == 0)) {
			map[true_pos-1].surrounding++;
		}
		if (true_pos+width+1 < height*width && (!(true_pos%width+1 == width))) {
			map[true_pos+width+1].surrounding++;
		}
		if (true_pos+width-1 < height*width && (!(true_pos%width == 0))) {
			map[true_pos+width-1].surrounding++;
		}
		if (true_pos-width+1 >= 0 && (!(true_pos%width+1 == width))) {
			map[true_pos-width+1].surrounding++;
		}
		if (true_pos-width-1 >= 0 && (!(true_pos%width == 0))) {
			map[true_pos-width-1].surrounding++;
		}
	}
	return map;
}

int main(int argc, char** argv) {
	srand(time(NULL));
	int height = 20;
	int width = 20;
	float percentage = 30.0f;
	int mines = 0;
	int opt;
	while ((opt = getopt(argc, argv, "hwpm")) != -1) {
		switch(opt) {
			case 'h':
				height = atol(argv[optind]);
				break;
			case 'w':
				width = atol(argv[optind]);
				break;
			case 'p':
				percentage = atof(argv[optind]);
				break;
			case 'm':
				mines = atol(argv[optind]);
				break;
			default:
				fprintf(stderr, "usage\n[-hwpm number]\n");
				exit(1);
				break;
		}
	}
	map = calloc(height*width, sizeof(tile));
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	start_color();
	init_pair(0, COLOR_BLACK, COLOR_BLACK);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);
	if (!mines) {
		mines = (height * width * percentage) / 100;
	}

	int mines_flagged = 0;
	int map_generated = 0;
	int c = 0;
	int player_y = (int)height/2;
	int player_x = (int)width/2;
	int keep_running = 1;
	draw(map, height, width);
	mvprintw(0, 0, "%d", mines-mines_flagged);
	move(player_y+1, player_x);
	refresh();
	while (keep_running && (c = getch()) != -1 ) {
		switch(c) {
			case 'h': case KEY_LEFT:
				player_x--;
				if (player_x < 0) player_x++;
				break;
			case 'j': case KEY_DOWN:
				player_y++;
				if (player_y > height-1) player_y--;
				break;
			case 'k': case KEY_UP:
				player_y--;
				if (player_y < 0) player_y++;
				break;
			case 'l': case KEY_RIGHT:
				player_x++;
				if (player_x > width-1) player_x--;
				break;
			case ' ':
				if (!map_generated) {
					map = generate_minefield(height, width, mines, player_y, player_x);
					map_generated = 1;
				}
				if (!(map[player_y*width + player_x].flagged)) {
					map[player_y*width + player_x].discovered = 1;
				}
				if (map[player_y*width + player_x].mine) {
					keep_running = 0;
					draw(map, height, width);
					mvprintw(0, 0, "You lost!");
					getch();
					break;
				}
				break;
			case 'f':
				if (!map[player_y*width + player_x].discovered) {
					if (map[player_y*width + player_x].flagged) {
						map[player_y*width + player_x].flagged = 0;
						mines_flagged--;
					} else {
						map[player_y*width + player_x].flagged = 1;
						mines_flagged++;
					}
				}
				break;
			case 'q':
				keep_running = 0;
				break;
			default: break;
		}
		int minefield_clear = 1;
		for (int i = 0; i < height*width; i++) {
			if (!map[i].discovered && !map[i].flagged) {
				minefield_clear = 0;
				break;
			}
		}
		if (minefield_clear) {
			keep_running = 0;
			draw(map, height, width);
			mvprintw(0, 0, "You win!");
			getch();
		}
		draw(map, height, width);
		mvprintw(0, 0, "%d", mines-mines_flagged);
		move(player_y+1, player_x);
		refresh();
	};
	endwin();
}

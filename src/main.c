#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct _tile {
	int mine, surrounding, discovered, flagged;
} tile;

tile* map;
int height = 20;
int width = 20;
float percentage = 25.0f;
int mines = 0;
int keep_running = 1;

int* get_neighbors(int position) {
	// first int is length of array, others are neighbor positions
	int* ret_val = malloc(9*sizeof(int));
	ret_val[0] = 1;
	if (position+width < height*width) {
		ret_val[ret_val[0]] = position+width;
		ret_val[0]++;
	}
	if (position-width >= 0) {
		ret_val[ret_val[0]] = position-width;
		ret_val[0]++;
	}
	if (!(position%width+1 == width)) {
		ret_val[ret_val[0]] = position+1;
		ret_val[0]++;
	}
	if (!(position%width == 0)) {
		ret_val[ret_val[0]] = position-1;
		ret_val[0]++;
	}
	if (position+width+1 < height*width && (!(position%width+1 == width))) {
		ret_val[ret_val[0]] = position+width+1;
		ret_val[0]++;
	}
	if (position+width-1 < height*width && (!(position%width == 0))) {
		ret_val[ret_val[0]] = position+width-1;
		ret_val[0]++;
	}
	if (position-width+1 >= 0 && (!(position%width+1 == width))) {
		ret_val[ret_val[0]] = position-width+1;
		ret_val[0]++;
	}
	if (position-width-1 >= 0 && (!(position%width == 0))) {
		ret_val[ret_val[0]] = position-width-1;
		ret_val[0]++;
	}
	return ret_val;
}

void draw() {
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

void discover_tile(int position, int automated) {
	if (map[position].discovered) {
		int surrounding_mines = 0;
		int surrounding_empty = 0;
		int* neighbors = get_neighbors(position);
		for (int neighbor_i = 1; neighbor_i < neighbors[0]; neighbor_i++) {
			if (map[neighbors[neighbor_i]].flagged)
				surrounding_mines++;
			else if (!map[neighbors[neighbor_i]].discovered)
				surrounding_empty++;
		}
		if (surrounding_mines == map[position].surrounding) {
			for (int neighbor_i = 1; neighbor_i < neighbors[0]; neighbor_i++) {
				if (!map[neighbors[neighbor_i]].flagged &&
						!map[neighbors[neighbor_i]].discovered) {
					discover_tile(neighbors[neighbor_i], 1);
					if (map[neighbors[neighbor_i]].mine) {
						keep_running = 0;
						draw();
						mvprintw(0, 0, "You lost!");
						getch();
						return;
					}
				}
			}
		} else if (
			surrounding_empty + surrounding_mines == map[position].surrounding
			&& !automated
		) {
			for (int neighbor_i = 1; neighbor_i < neighbors[0]; neighbor_i++) {
				if (!map[neighbors[neighbor_i]].flagged &&
						!map[neighbors[neighbor_i]].discovered) {
					map[neighbors[neighbor_i]].flagged = 1;
				}
			}

		}
		return;
	}
	if (!(map[position].flagged)) {
		map[position].discovered = 1;
		if (map[position].surrounding == 0 && !map[position].mine) {
			int* neighbors = get_neighbors(position);
			for (int neighbor_i = 1; neighbor_i < neighbors[0]; neighbor_i++) {
				discover_tile(neighbors[neighbor_i], 1);
			}
		}
	}
	if (map[position].mine) {
		keep_running = 0;
		draw();
		mvprintw(0, 0, "You lost!");
		getch();
		return;
	}
}

void generate_minefield(int player_y, int player_x) {
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
	int num_neighbors = get_neighbors(player_x*width + player_x)[0];
	for (int i = 0; i < mines; i++) {
		true_pos = 0;
		int position = rand() % ((height) * (width) - i - num_neighbors);
		for (int j = 0; j < position;true_pos++) {
			if (!map[true_pos].mine || map[true_pos].discovered)
				j++;
		}
		if (map[true_pos].mine || map[true_pos].discovered) {
			i--;
			continue;
		}

		map[true_pos].mine = 1;
		int* neighbors = get_neighbors(true_pos);
		for (int neighbor_i = 1; neighbor_i < neighbors[0]; neighbor_i++) {
			map[neighbors[neighbor_i]].surrounding++;
		}
	}
	for (int x = -1; x <= 1; x++)
		for (int y = -1; y <= 1; y++)
			if (player_y+y >= 0 && player_y+y < height && player_x+x >= 0 && player_x+x < width) map[(player_y+y)*width + (player_x+x)].discovered = 0;
	discover_tile(player_y*width+player_x, 1);
}

int main(int argc, char** argv) {
	srand(time(NULL));
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
	draw();
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
					generate_minefield(player_y, player_x);
					map_generated = 1;
				}
				discover_tile(player_y*width+player_x, 0);
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
			draw();
			mvprintw(0, 0, "You win!");
			getch();
		}
		draw();
		mvprintw(0, 0, "%d", mines-mines_flagged);
		move(player_y+1, player_x);
		refresh();
	};
	endwin();
}

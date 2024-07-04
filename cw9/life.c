#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <signal.h>

typedef struct {
	char** foreground;
	char** background;
	int start_index;
	int end_index;
} thread_arg;

void *update(void *arg)
{
	thread_arg *args = (thread_arg *)arg;
	while(true) {
		for (int i = args->start_index; i < args->end_index; i++)
		{
			int row = i / 30;
			int col = i % 30;
			(*args->background)[i] = is_alive(row, col, *args->foreground);
		}
		pause();
	}
}

void handle_sigusr1(int sig)
{}

int main(int argc, char **argv)
{
	struct sigaction sa;
	sa.sa_handler = handle_sigusr1;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, NULL);

	int n = atoi(argv[1]);

	srand(time(NULL));
	// setlocale(int category, const char *locale)
	// ustawia kategorie lokalizacji na podana w locale
	// LC_CTYPE - kategoria okreslajaca formatowanie znakow
	// "" - ustawia kategorie na domyslna
	setlocale(LC_CTYPE, "");
	initscr(); // Start curses mode

	char *foreground = create_grid();
	char *background = create_grid();
	char *tmp;

	pthread_t threads[n];
	thread_arg args[n];

	int cells_per_thread = (int)ceil(30 * 30 / n); 


	for (int i = 0; i < n; i++)
	{
		args[i].foreground = &foreground;
		args[i].background = &background;
		args[i].start_index = i * cells_per_thread;
		args[i].end_index = (i + 1) * cells_per_thread;
		// pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
		// tworzy nowy watek
		// thread - wskaznik do zmiennej w ktorej zostanie zapisany identyfikator watku
		// attr - atrybuty watku
		// start_routine - funkcja ktora ma byc wywolana przez watek
		// arg - argument przekazywany do funkcji
		pthread_create(&threads[i], NULL, update, &args[i]);
	}

	init_grid(foreground);

	while (true)
	{
		draw_grid(foreground);

		for(int i = 0; i < n; i++)
		{
			// pthread_kill(pthread_t thread, int sig)
			// wysyla sygnal do watku
			// thread - identyfikator watku
			// sig - sygnal
			pthread_kill(threads[i], SIGUSR1);
		}
		
		usleep(500 * 1000);
		// Step simulation
		tmp = foreground;
		foreground = background;
		background = tmp;
	}

	endwin(); // End curses mode
	destroy_grid(foreground);
	destroy_grid(background);

	return 0;
}

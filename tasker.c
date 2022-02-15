/*
 * This file is part of the tasker project.
 * See the LICENSE file for license (GPLv3)
 *
 * Author:
 * Steve Grubb <sgrubb@redhat.com>
 *
 */
#include "config.h"
#include <stdio.h>
#include <omp.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>


atomic_int *pids;
unsigned int threads;
#define MAX_ARGS 12
char *my_argv[MAX_ARGS];
unsigned int alter = 0;
const char *begin = NULL, *end = NULL;
pthread_mutex_t pid_lock;
pthread_cond_t pid_nonempty;

void initialize_lookup(void)
{
	for (unsigned int i=0; i < threads; i++)
		pids[i] = 0;

	return;
}

static inline int find_empty_lookup(void)
{
	for (unsigned int i=0; i < threads; i++) {
		if (pids[i] == 0)
			return i;
	}
	return -1;
}

int lookup_not_empty(void)
{
	for (unsigned int i=0; i < threads; i++) {
		if (pids[i])
			return 1;
	}
	return 0;
}

static inline void zero_lookup(pid_t pid)
{
	for (unsigned int i=0; i < threads; i++) {
		if (pids[i] == pid) {
			pids[i] = 0;
			return;
		}
	}
}


static void child_handler(int sig __attribute__((unused)))
{
	pid_t pid;

	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
		zero_lookup(pid);
		pthread_cond_signal(&pid_nonempty);
	}
}

static void safe_exec(int slot, const char *line)
{
	int pid = fork();
	if (pid < 0)
		return;

	// Parent
	if (pid) {
		pids[slot] = pid;
		return;
	}

	// Child - getline always ends with a carriage return
	char *ptr = strrchr(line, '\n');
	if (ptr)
		*ptr = 0;
	if (alter) {
		// This leaks but we don't care
		if (asprintf(&my_argv[alter], "%s%s%s", begin ? begin : "",
			 line, end ? end : "") < 0)
			return;
	}

	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(slot, &mask);
	sched_setaffinity(0, sizeof(cpu_set_t), &mask);

	/*  For debugging
	unsigned int i = 1; printf("%s ", my_argv[0]);
	while (my_argv[i]) printf("%s ", my_argv[i++]); printf("\n"); */
	execve(my_argv[0], my_argv, NULL);
	exit(1);
}

int main(int argc, char *argv[])
{
	// count hyperthreads
	threads = omp_get_max_threads();
//	printf("Threads: %u\n", threads);

	// validate command line options
	if (argc > MAX_ARGS) {
		fprintf(stderr, "Too many arguments\n");
		return 1;
	} else if (argc < 2) {
		fprintf(stderr, "Not enough arguments\n");
		return 1;
	} else if (argc == 2) {
		if (strcmp(argv[1], "--version") == 0) {
			printf("%s\n", VERSION);
			return 0;
		} else if (strcmp(argv[1], "--help") == 0) {
			   puts("tasker command [args]");
			   return 0;
		}
	}

	for (int cnt = 0; cnt < MAX_ARGS; cnt++) {
		// Need to look for @@ and remember which one it is
		char *ptr = NULL;
		if (cnt+1 < argc)
			ptr = strstr(argv[cnt+1], "@@");
		if (ptr) {
			// If argc does not start with @@
			if (ptr != argv[cnt+1]) {
				begin = argv[cnt+1];
				*ptr = 0;
			}
			// If the whole string is not @@
			if (strcmp(ptr, "@@")) {
				end = ptr + 2;
			}
			// Remember the one to build
			alter = cnt;
		}

		if (cnt < argc) {
			my_argv[cnt] = argv[cnt+1];
			if (cnt == 0) {
				if (my_argv[0][0] != '/' &&
				    strncmp(my_argv[0], "./", 2)) {
					fprintf(stderr,
				      "Error - commands need the full path\n");
					return 1;
				} else if (access(my_argv[0], X_OK)) {
					fprintf(stderr,
			      "Error - command not found or not executable\n");
					return 1;
				}
			}
		} else {
			my_argv[cnt] = NULL;
		}
	}
//printf("Begin: %s, end: %s, alter: %u\n", begin, end, alter);

	if (!alter)
		fprintf(stderr, "No @@ found - likely a mistake\n");

	// allocate a lookup table
	pids = malloc(threads * sizeof(atomic_int));
	initialize_lookup();
	pthread_mutex_init(&pid_lock, NULL);
	pthread_cond_init(&pid_nonempty, NULL);

	// setup child signal handler
	struct sigaction sa;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = child_handler;
	sigaction(SIGCHLD, &sa, NULL);

	// loop reading stdin
	ssize_t nread;
	char *line = NULL;
	size_t len = 0;
	unsigned ran = 0;

	// This might be better done with select and fd_fgets
	while ((nread = getline(&line, &len, stdin)) != -1) {
	// find empty slot, fork, set affinity, launch application, update table
retry:
		int slot = find_empty_lookup();
		if (slot >= 0) {
			safe_exec(slot, line);
			ran = 1;
		} else {
			// when the table is full, stop and wait
			// if sigchild, erase the entry in table, cont the loop
			pthread_cond_wait(&pid_nonempty, &pid_lock);
			goto retry;
		}

	}

	// when done, wait for children
	while (ran && lookup_not_empty()) {
		pthread_cond_wait(&pid_nonempty, &pid_lock);
	}

	// free lookup table
	pthread_mutex_destroy(&pid_lock);
	pthread_cond_destroy(&pid_nonempty);
	free(pids);
	return 0;
}


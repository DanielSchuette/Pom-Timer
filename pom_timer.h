/* Pom_Timer Copyright (C) 2021 Daniel Schuette
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef __TIMER_H__

#include <stdbool.h>
#include <stdlib.h>

#define WORK_TIME_DEFAULT   25
#define BREAK_TIME_DEFAULT  5
#define MODE_FAIL           0
#define MODE_CONTINUE       1

/* ANSI escape codes for terminal colors */
#define ANSI_RESET   "\x1b[0m"
#define ANSI_BLACK   "\x1b[30m"
#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_WHITE   "\x1b[37m"

const char* help_msg =
"pom_timer is a minimalistic Pomodoro Timer that you can run in a terminal.\n"
"The default timer counts to 25 minutes in 1 second intervals, asking you\n"
"to do work. Then, a break of 5 minutes is recommended. These intervals\n"
"can be changed via options described below. You can save statistic about\n"
"how much you got done in a log file, too.\n"
"Author: Daniel Schuette <d.schuette@online.de>\n"
"Release: v0.1.0 (2021/04/27)\n"
"Sources: https://github.com/DanielSchuette/pom_timer\n"
"License: GPLv3 (https://www.gnu.org/licenses/gpl-3.0.en.html)\n"
"\tPom-Timer Copyright (C) 2021  Daniel Schuette\n"
"\tThis program comes with ABSOLUTELY NO WARRANTY. This is free\n"
"\tsoftware, and you are welcome to redistribute it under certain\n"
"\tconditions.\n"
"\n"
"USAGE:\n"
"\ttimer [FLAGS] [OPTIONS]\n"
"\n"
"FLAGS:\n"
"\t-h, --help\t\tDisplay this help message\n"
"\n"
"OPTIONS:\n"
"\t-w, --work <TIME>\tChange the default work time to TIME\n"
"\t-b, --break <TIME>\tChange the default break time to TIME\n"
"\t-f, --log-file <FILE>\tPath to a log file (without, no logs are saved)\n";

typedef struct ptimer {
    int secs;
    int mins;
    int num_work;  /* number of work cycles */
    int num_break; /* number of break cycles */
} ptimer;

typedef struct configs {
    int work_time;
    int break_time;
    char *save_path;
} configs;

/* global because it's tedious to pass these around (and signals need them) */
configs config = { WORK_TIME_DEFAULT, BREAK_TIME_DEFAULT, NULL };
ptimer timer = { 0, 0, 0, 0 };

/* print_and_sleep: print `msg' and timer information, then sleep `s' secs. */
void print_and_sleep(ptimer *, int, const char *);

/* inc: increment fields of a timer. */
void inc(ptimer *);

/* take_break: display break message and timer for `config.break_time' secs. */
void take_break(ptimer *);

/* is_work, is_break: determine if it's appropriate to work/take break. */
bool is_work(ptimer *);
bool is_break(ptimer *);

/* clear_line: a dirty hack to clear a terminal line. */
void clear_line(void);

/* consume_args: adds parsed options to global `config' struct. */
void consume_args(int, char **);

/* get_option: add an option to `config' and modify argc & argv. */
void get_option(int *, char ***, const char *);

/* bad_option: used by `consume_args'; make sure to provide a valid mode. */
void bad_option(int, const char *, int);

/* sigint_handler: catch signals to be able to save stats before exiting. */
void sigint_handler(int);

/* save_stats: save some stats to a log file. */
void save_stats(const char *);

/* setup_term: disable ECHOCTL, i.e. don't print ^C on sigint. */
void setup_term(void);

/* print_info: print info about initial setup to terminal. */
void print_info(void);

#endif /* __TIMER_H__ */

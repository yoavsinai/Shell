#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include "core/command.h"
#include "jobs/job_table.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

void sigint_handler(int signo);

void sigchld_handler(int signo);

#endif

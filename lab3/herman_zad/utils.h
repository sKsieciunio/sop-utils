#pragma once
#include <stdio.h>
#include <stdlib.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void msleep(unsigned int milisec);
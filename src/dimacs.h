#pragma once

#include <stdio.h>

#include "types.h"

int parse_dimacs_input(FILE*, struct formula*);
int parse_dimacs_output(FILE*, struct solution*);
void compose_dimacs_output(FILE*, struct solution*);

/*
 *
 *
 *   ----------------------  smooth values ------------------------------
 *
 *
 */

#ifndef SMOOTH_H
#define SMOOTH_H

// #include "config.h"
#include "esp_log.h"
#include <stddef.h>
// #include "header.h"
#include "main.h"

void addReading(int pos, int sensor, float value);
void test();
float getMiddle(int sensor);
// long map(long x, long in_min, long in_max, long out_min, long out_max);

#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define DATA_SIZE 256
#define N_STATE 2
char basedir[DATA_SIZE];
char *basedir_end = NULL;
char content[DATA_SIZE];
char command[DATA_SIZE*4];

char *ROT[]   = {"normal", 				"inverted", 			"left", 				"right"};
char *COOR[]  = {"1 0 0 0 1 0 0 0 1",	"-1 0 1 0 -1 1 0 0 1", 	"0 -1 1 1 0 0 0 0 1", 	"0 1 0 -1 0 1 0 0 1"};
char *TOUCH[] = {"enable", 				"disable", 				"disable", 				"disable"};

double accel_y = 0.0, accel_x = 0.0, accel_g = 7.0;
int current_state = 0;

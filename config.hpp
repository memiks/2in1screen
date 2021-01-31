#include <string>
#include <json/json.h>
#include <iio.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iio.h>

/*
* 0 deg cw:     90 deg cw:		180 deg cw:		270 deg cw:
* [ 1 0  1 ]    [ 0 -1 1]		[ -1  0 1]		[  0 1 0 ]
* [ 0 1 -1 ]    [ 1  0 0]		[  0 -1 1]		[ -1 0 1 ]
*/

static int degrees_cw[4] = {0,90,180,270};

static std::string CoordinateTransformationMatrix = "Coordinate Transformation Matrix";


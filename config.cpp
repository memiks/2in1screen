#include "config.hpp"

class Config {

  Config() {}

private:
  int data_size = 256;
  int n_state = 2;
  char basedir[256];
  char *basedir_end = NULL;
  char content[256];
  char command[256 * 4];

  double accel_y = 0.0, accel_x = 0.0, accel_g = 7.0;
  int current_state = 0;
  std::string rotation[4] = {"normal", "inverted", "left", "right"};
  std::string coordinates[4] = {"1 0 0 0 1 0 0 0 1", "-1 0 1 0 -1 1 0 0 1",
                                "0 -1 1 1 0 0 0 0 1", "0 1 0 -1 0 1 0 0 1"};
  std::string touch[4] = {"enable", "disable", "disable", "disable"};
};
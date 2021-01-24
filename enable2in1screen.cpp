#include "config.hpp"
#include "include.h"
#include <iio.h>
#include <string>

class Enable2In1Screen {
private:
  double accel_x;
  double accel_y;
  std::string attr_accel_x;
  std::string attr_accel_y;
  iio_context *iiocontext;
  iio_channel *channel;

  const std::string name_accel_x = "accel_x";
  const std::string name_accel_y = "accel_y";

public:
  /**
   * @brief
   *
   * @return int
   */
  int rotation_changed() {
    int state = 0;
    /*
        if (accel_y < -accel_g)
          state = 0;
        else if (accel_y > accel_g)
          state = 1;
    #if n_state == 4
        else if (accel_x > accel_g)
          state = 2;
        else if (accel_x < -accel_g)
          state = 3;
    #endif

        if (current_state != state) {
          current_state = state;
          return 1;
        } else
              */
    return 0;
  }

  /**
   * @brief
   *
   */
  void rotate_screen() {
    /*
sprintf(command, "xrandr -o %s", ROT[current_state]);
system(command);
sprintf(command,
      "xinput set-prop \"%s\" \"Coordinate Transformation Matrix\" %s",
      "Wacom HID 4846 Finger", COOR[current_state]);
system(command);
  */
  }

  void iio_search_accelerator() {
    this->iiocontext = iio_create_local_context();

    int nb_iio_devices = iio_context_get_devices_count(this->iiocontext);
    printf("Number of iio devices = %d\n", nb_iio_devices);

    bool display = false;

    for (int i = 0; i < nb_iio_devices; i++) {
      iio_device *device = iio_context_get_device(this->iiocontext, i);
      int nb_channels = iio_device_get_channels_count(device);
      for (int j = 0; j < nb_channels; j++) {
        iio_channel *channel = iio_device_get_channel(device, j);

        int nb_attrs = iio_channel_get_attrs_count(channel);
        for (int k = 0; k < nb_attrs; k++) {
          const char *attr = iio_channel_get_attr(channel, k);
          std::string filename = iio_channel_attr_get_filename(channel, attr);
          if (filename.find(this->name_accel_x) != std::string::npos) {
            if (!this->channel)
              this->channel = channel;
            this->attr_accel_x = attr;
            break;
          } else if (filename.find(this->name_accel_y) != std::string::npos) {
            if (!this->channel)
              this->channel = channel;
            this->attr_accel_y = attr;
            break;
          }
        }
      }
    }
  }

  void read_accel() {
	  iio_channel_attr_read_double(const this->channel, this->attr_accel_x, this->accel_x);

  }
};

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char const *argv[]) {
  Enable2In1Screen enable2in1screen;

  enable2in1screen.iio_search_accelerator();

  return 0;
}

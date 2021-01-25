#include "config.hpp"
#include "include.h"
#include <iio.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

class Enable2In1Screen {
private:
  double accel_x;
  double accel_y;
  double accel_scale;

  long long old_accel_x;
  long long old_accel_y;

  static const long long accel_g = 7;
  int current_state;

  const char *attr_accel_x;
  const char *attr_accel_y;
  const iio_context *iiocontext = NULL;
  const iio_channel *channel = NULL;
  const iio_device *device = NULL;
  const iio_buffer *buffer = NULL;

  const std::string name_accel_x = "in_accel_x_raw";
  const std::string name_accel_y = "in_accel_y_raw";
  const std::string name_in_accel_scale = "in_accel_scale";

  const int number_of_rotation = 4;

public:
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

        if (iio_channel_get_type(channel) != IIO_ACCEL) {
          break;
        }

        int nb_attrs = iio_channel_get_attrs_count(channel);
        for (int k = 0; k < nb_attrs; k++) {
          const char *attr = iio_channel_get_attr(channel, k);
          std::string filename = iio_channel_attr_get_filename(channel, attr);
          if (filename.find(this->name_accel_x) != std::string::npos) {

            if (!this->channel) {
              this->channel = channel;
              this->device = device;
              iio_channel_enable(channel);
            }
            this->attr_accel_x = attr;
            break;
          } else if (filename.find(this->name_accel_y) != std::string::npos) {
            if (!this->channel) {
              this->channel = channel;
              this->device = device;
              iio_channel_enable(channel);
            }
            this->attr_accel_y = attr;
            break;
          } else if (filename.find(this->name_in_accel_scale) != std::string::npos) {
            if (!this->channel) {
              this->channel = channel;
              this->device = device;
              iio_channel_enable(channel);
            }
            iio_channel_attr_read_double(this->channel, attr,
                                   &this->accel_scale);
            break;
          }
        }
      }
    }
  }

  int read_accel() {
    if (!this->channel)
      return -1;
    iio_channel_attr_read_double(this->channel, this->attr_accel_x,
                                   &this->accel_x);
    this->accel_x *= this->accel_scale;                                   
    iio_channel_attr_read_double(this->channel, this->attr_accel_y,
                                   &this->accel_y);
    this->accel_y *= this->accel_scale;                                   

    printf("READ accel_x %f\n", this->accel_x);
    printf("READ accel_y %f\n", this->accel_y);

    return 1;
  }

  int get_state() {
    if (this->accel_y < -this->accel_g)
      return 0;
    
    if (this->accel_y > this->accel_g)
      return 1;
    
    if (this->accel_x > this->accel_g)
      return 2;
    
    if (this->accel_x < -this->accel_g)
      return 3;
    
    return -1;
  }
  /**
   * @brief
   *
   * @return int
   */
  bool rotation_changed() {
    int state = get_state();

    if (current_state != state) {
      current_state = state;
      return true;
    }

    return false;
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

  while (enable2in1screen.read_accel() > 0) {
    if(enable2in1screen.rotation_changed())
      puts("Rotation changed !!");

    sleep(1);
  }

  return 0;
}

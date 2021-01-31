#include "config.hpp"
#include <cstdio>
#include <ctime>
#include <fcntl.h>
#include <iio.h>
#include <iostream>
#include <libinput.h>
#include <libudev.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/randr.h>

#include "transform.h"
/*
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
*/

int catcher(Display *disp, XErrorEvent *xe) {
  printf("Something had happened, bruh.\n");
  return 0;
}

static int open_restricted(const char *path, int flags, void *user_data) {
  int fd = open(path, flags);
  return fd < 0 ? -errno : fd;
}

std::string COOR[4] = {"1 0 0 0 1 0 0 0 1", "0 -1 1 1 0 0 0 0 1",
                       "-1 0 1 0 -1 1 0 0 1", "0 1 0 -1 0 1 0 0 1"};
static const float MatrixRotatation[4][6] = {{1, 0, 0, 0, 1, 0},
                                             {0, -1, 1, 1, 0, 0},
                                             {-1, 0, 1, 0, -1, 1},
                                             {0, 1, 0, -1, 0, 1}};

static void close_restricted(int fd, void *user_data) { close(fd); }

const static struct libinput_interface interface = {
    .open_restricted = open_restricted,
    .close_restricted = close_restricted,
};

class Enable2In1Screen {
private:
  double accel_x;
  double accel_y;
  double accel_z;
  double accel_scale;
  double accel_g = 7.0;

  std::vector<libinput_device *> li_touch_devices;

  int current_state = 0;

  const char *attr_accel_x;
  const char *attr_accel_y;
  const char *attr_accel_z;
  const iio_context *iiocontext = NULL;
  const iio_channel *channel = NULL;
  const iio_device *device = NULL;
  const iio_buffer *buffer = NULL;

  const std::string name_accel_x = "in_accel_x_raw";
  const std::string name_accel_y = "in_accel_y_raw";
  const std::string name_accel_z = "in_accel_z_raw";
  const std::string name_in_accel_scale = "in_accel_scale";

  const int number_of_rotation = 4;

public:
  void iio_search_accelerator() {
    this->iiocontext = iio_create_local_context();

    int nb_iio_devices = iio_context_get_devices_count(this->iiocontext);

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

          } else if (filename.find(name_accel_y) != std::string::npos) {
            if (!this->channel) {
              this->channel = channel;
              this->device = device;
              iio_channel_enable(channel);
            }
            this->attr_accel_y = attr;

          } else if (filename.find(name_accel_z) != std::string::npos) {
            if (!this->channel) {
              this->channel = channel;
              this->device = device;
              iio_channel_enable(channel);
            }
            this->attr_accel_z = attr;

          } else if (filename.find(name_in_accel_scale) != std::string::npos) {
            if (!this->channel) {
              this->channel = channel;
              this->device = device;
              iio_channel_enable(channel);
            }
            iio_channel_attr_read_double(channel, attr, &accel_scale);
          }
        }
      }
    }
  }

  int read_accel() {
    if (!this->channel)
      return -1;

    iio_channel_attr_read_double(channel, attr_accel_x, &accel_x);
    accel_x *= accel_scale;
    iio_channel_attr_read_double(channel, attr_accel_y, &accel_y);
    accel_y *= accel_scale;
    iio_channel_attr_read_double(channel, attr_accel_z, &accel_z);
    accel_z *= accel_scale;

    /*
        printf("accel_x %2.2f ",accel_x);
        printf("accel_y %2.2f ",accel_y);
        printf("accel_z %2.2f\n",accel_z);
        printf("accel_g %2.2f\n",accel_g);
    */
    return 1;
  }
  int change_state() {
    if ((accel_x < -accel_g) && (accel_y < -accel_g) && (accel_z < -accel_g))
      return 3;

    if ((accel_x < accel_g) && (accel_y < accel_g) && (accel_z < accel_g))
      return 0;

    if ((accel_x > accel_g) && (accel_y > accel_g) && (accel_z > accel_g))
      return 1;

    // ??? return 2;

    return 0;
  }

  int get_state() { return this->current_state; }
  /**
   * @brief
   *
   * @return int
   */
  bool rotation_changed() {
    int state = change_state();

    if (current_state != state) {
      current_state = state;
      return true;
    }

    return false;
  }

  void list_attr(struct udev *udev, struct udev_device *child) {
    struct udev_list_entry *sysattrs =
        udev_device_get_sysattr_list_entry(child);
    struct udev_list_entry *attrentry;

    udev_list_entry_foreach(attrentry, sysattrs) {
      const char *attrname = udev_list_entry_get_name(attrentry);
      printf("scsi=%s, %s = %s\n", udev_device_get_devnode(child), attrname,
             udev_device_get_sysattr_value(child, attrname));
    }
  }

  void enumerate_usb_mass_storage() {
    struct udev *udev = udev_new();
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_add_match_sysattr(enumerate, "id/vendor", "056a");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;

    udev_list_entry_foreach(entry, devices) {
      const char *path = udev_list_entry_get_name(entry);
      struct udev_device *scsi = udev_device_new_from_syspath(udev, path);

      list_attr(udev, scsi);

      udev_device_unref(scsi);
    }

    udev_enumerate_unref(enumerate);
  }

  static void print_device_notify(struct libinput_device *dev) {
    struct libinput_seat *seat = libinput_device_get_seat(dev);
    struct libinput_device_group *group;
    struct udev_device *udev_device;
    double w, h;
    static int next_group_id = 0;
    intptr_t group_id;
    const char *devnode;
    char *str;

    group = libinput_device_get_device_group(dev);
    group_id = (intptr_t)libinput_device_group_get_user_data(group);
    if (!group_id) {
      group_id = ++next_group_id;
      libinput_device_group_set_user_data(group, (void *)group_id);
    }

    udev_device = libinput_device_get_udev_device(dev);
    devnode = udev_device_get_devnode(udev_device);

    printf("Device:           %s\n"
           "Kernel:           %s\n"
           "Group:            %d\n"
           "Seat:             %s, %s\n",
           libinput_device_get_name(dev), devnode, (int)group_id,
           libinput_seat_get_physical_name(seat),
           libinput_seat_get_logical_name(seat));

    udev_device_unref(udev_device);

    if (libinput_device_get_size(dev, &w, &h) == 0)
      printf("Size:             %.fx%.fmm\n", w, h);
    printf("Capabilities:     ");
    if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_KEYBOARD))
      printf("keyboard ");
    if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_POINTER))
      printf("pointer ");
    if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_TOUCH))
      printf("touch ");
    if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_TABLET_TOOL))
      printf("tablet ");
    if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_TABLET_PAD))
      printf("tablet-pad");
    if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_GESTURE))
      printf("gesture");
    if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_SWITCH))
      printf("switch");
    printf("\n");

    printf("\n");
  }

  void testlibinput() {

    puts("testlibinput");
    struct libinput *li;
    struct udev *udev = udev_new();
    struct libinput_event *ev;

    li = libinput_udev_create_context(&interface, NULL, udev);

    if (libinput_udev_assign_seat(li, "seat0") == -1)
      perror("error in libinput_udev_assign_seat");

    libinput_dispatch(li);
    struct libinput_device *dev;
    while ((ev = libinput_get_event(li))) {
      if (libinput_event_get_type(ev) == LIBINPUT_EVENT_DEVICE_ADDED) {
        dev = libinput_event_get_device(ev);
        if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_TOUCH) ||
            libinput_device_has_capability(dev,
                                           LIBINPUT_DEVICE_CAP_TABLET_TOOL)) {
          li_touch_devices.push_back(libinput_event_get_device(ev));
        }
        libinput_event_destroy(ev);
        libinput_dispatch(li);
      }
    }

    for (int i = 0; i < li_touch_devices.size(); i++) {
      print_device_notify(li_touch_devices[i]);
    }

    libinput_unref(li);
  }

  void rotate_screen(int screen = 0) {

    Rotation new_rotation;

    switch (this->current_state) {
    case 0:
      new_rotation = RR_Rotate_0;
      break;
    case 1:
      new_rotation = RR_Rotate_90;
      break;
    case 2:
      new_rotation = RR_Rotate_180;
      break;
    case 3:
      new_rotation = RR_Rotate_270;
      break;
    default:
      return;
    }

    Rotation current_rotation;

    Display *dpy = XOpenDisplay(NULL);
    Window root = RootWindow(dpy, screen);

    XRRScreenConfiguration *config = XRRGetScreenInfo(dpy, root);
    SizeID current_size_id =
        XRRConfigCurrentConfiguration(config, &current_rotation);

    Time timestamp = std::time(nullptr);

    XRRSetScreenConfig(dpy, config, root, current_size_id, new_rotation,
                       timestamp);
    float matrix[6];
    struct libinput *li;
    struct udev *udev = udev_new();
    struct libinput_event *ev;

    li = libinput_udev_create_context(&interface, NULL, udev);

    if (libinput_udev_assign_seat(li, "seat0") == -1)
      perror("error in libinput_udev_assign_seat");

    libinput_dispatch(li);
    struct libinput_device *dev;
    while ((ev = libinput_get_event(li))) {
      if (libinput_event_get_type(ev) == LIBINPUT_EVENT_DEVICE_ADDED) {
        dev = libinput_event_get_device(ev);
        if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_TOUCH) ||
            libinput_device_has_capability(dev,
                                           LIBINPUT_DEVICE_CAP_TABLET_TOOL)) {

          switch (libinput_device_config_calibration_set_matrix(
              dev, MatrixRotatation[this->current_state])) {
          case LIBINPUT_CONFIG_STATUS_SUCCESS:
            puts("LIBINPUT_CONFIG_STATUS_SUCCESS");
            break;
          case LIBINPUT_CONFIG_STATUS_UNSUPPORTED:
            puts("LIBINPUT_CONFIG_STATUS_UNSUPPORTED");
            break;
          case LIBINPUT_CONFIG_STATUS_INVALID:
            puts("LIBINPUT_CONFIG_STATUS_INVALID");
            break;
          }

          libinput_device_config_calibration_get_matrix(dev, matrix);
          printf("{%f,%f,%f,%f,%f,%f}\n", matrix[0], matrix[1], matrix[2],
                 matrix[3], matrix[4], matrix[5]);
        }
        libinput_event_destroy(ev);
        libinput_dispatch(li);
      }
    }
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

  // enable2in1screen.enumerate_usb_mass_storage();
  enable2in1screen.iio_search_accelerator();

  // return 0;
  // XGetDeviceProperty

  enable2in1screen.testlibinput();
  while (true) {
    enable2in1screen.read_accel();
    if (enable2in1screen.rotation_changed()) {
      enable2in1screen.rotate_screen();

      Display *dpy = XOpenDisplay(NULL);
      int nbDevices, nbProperties;

      XDeviceInfo *devices = XListInputDevices(dpy, &nbDevices);

      char command[256 * 4];

      for (int i = 0; i < nbDevices; i++) {
        std::string atomName;
        bool found = false;

        if (devices[i].type != 0l) {
          atomName.assign(XGetAtomName(dpy, devices[i].type));
          found = (atomName.compare(XI_TOUCHSCREEN) == 0);
          //found = ((atomName.compare(XI_TABLET) == 0) ||
          //         (atomName.compare(XI_TOUCHSCREEN) == 0));
        } else {
          atomName.assign("BadAtom");
        }
        if (found) {

          sprintf(
              command,
              "xinput set-prop \"%s\" \"Coordinate Transformation Matrix\" %s",
              devices[i].name, COOR[enable2in1screen.get_state()].c_str());
          printf("%s\n", command);
          system(command);
          /*
          std::vector<float> data4;

          long l;
          printf("size of long (%lu)", sizeof(l));
          if ((sizeof l) == 4) {
            data4 = {1., 0., 0., 0., 1., 0., 0., 0., 1.};

          } else if ((sizeof l) == 8) {
            // Xlib needs the floats long-aligned, so add "buffer" elements.
            data4 = {1., 0., 0., 0., 0., 0., 0., 0., 1.,
                     0., 0., 0., 0., 0., 0., 0., 1., 0.};
          }

          try {

          XIChangeProperty(
              dpy, devices[i].id,
              XInternAtom(dpy, CoordinateTransformationMatrix.c_str(), 0),
              XInternAtom(dpy, "FLOAT", 0), 32, PropModeReplace,
              (unsigned char *)data4.data(), 9);

          } catch (XErrorEvent xee) {
          puts("XErrorEvent error");
          }
          */
        }
      }
      XFreeDeviceList(devices);
    }
    sleep(3);
  }

  return 0;
}

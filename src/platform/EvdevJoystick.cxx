/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* common headers */
#include "common.h"

/* interface headers */
#include "EvdevJoystick.h"

#ifdef HAVE_LINUX_INPUT_H

/* system headers */
#include <vector>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

/* implementation headers */
#include "ErrorHandler.h"

#define test_bit(nr, addr) \
	(((1UL << ((nr) & 31)) & (((const unsigned int *) addr)[(nr) >> 5])) != 0)



bool             EvdevJoystick::isEvdevAvailable()
{
  /* Test whether this driver should be used without actually
   * loading it. Will return false if no event devices can be
   * located, or if it has been specifically disabled by setting
   * the environment variable BZFLAG_ENABLE_EVDEV=0
   */

  char *envvar = getenv("BZFLAG_ENABLE_EVDEV");
  if (envvar)
    return atoi(envvar) != 0;

  std::map<std::string,EvdevJoystickInfo> joysticks;
  scanForJoysticks(joysticks);
  return !joysticks.empty();
}


EvdevJoystick::EvdevJoystick()
{
  joystickfd = 0;
  currentJoystick = NULL;
  ff_rumble = new struct ff_effect;
  scanForJoysticks(joysticks);
}

EvdevJoystick::~EvdevJoystick()
{
  initJoystick("");
  delete ff_rumble;
}

void             EvdevJoystick::scanForJoysticks(std::map<std::string,
							EvdevJoystickInfo> &joysticks)
{
  joysticks.clear();

  const std::string inputdirName = "/dev/input";
  DIR* inputdir = opendir(inputdirName.c_str());
  if (!inputdir)
    return;

  struct dirent *dent;
  while ((dent = readdir(inputdir))) {
    EvdevJoystickInfo info;

    /* Does it look like an event device? */
    if (strncmp(dent->d_name, "event", 5))
      continue;

    /* Can we open it? */
    info.filename = inputdirName + "/" + dent->d_name;
    int fd = open(info.filename.c_str(), O_RDWR);
    if (!fd)
      continue;

    /* Does it look like a joystick? */
    if (!(collectJoystickBits(fd, info) && isJoystick(info))) {
      close(fd);
      continue;
    }

    /* Can we get its name? */
    char jsname[128];
    if (ioctl(fd, EVIOCGNAME(sizeof(jsname)-1), jsname) < 0) {
      close(fd);
      continue;
    }
    jsname[sizeof(jsname)-1] = '\0';

    close(fd);

    /* Yay, add it to our map.
     *
     * FIXME: we can't handle multiple joysticks with the same name yet.
     *        This could be fixed by disambiguating jsname if it already
     *        exists in 'joysticks', but the user would still have a hard
     *        time knowing which device to pick.
     */
    joysticks[jsname] = info;
  }

  closedir(inputdir);
}

bool                    EvdevJoystick::collectJoystickBits(int fd, struct EvdevJoystickInfo &info)
{
  /* Collect all the bitfields we're interested in from an event device
   * at the given file descriptor.
   */
  if (ioctl(fd, EVIOCGBIT(0, sizeof(info.evbit)), info.evbit) < 0)
    return false;
  if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(info.keybit)), info.keybit) < 0)
    return false;
  if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(info.absbit)), info.absbit) < 0)
    return false;
  if (ioctl(fd, EVIOCGBIT(EV_FF, sizeof(info.ffbit)), info.ffbit) < 0)
    return false;

  /* Collect information about our absolute axes */
  int axis;
  for (axis=0; axis<2; axis++) {
    if (ioctl(fd, EVIOCGABS(axis + ABS_X), &info.axis_info[axis]) < 0)
      return false;
  }

  return true;
}

bool			EvdevJoystick::isJoystick(struct EvdevJoystickInfo &info)
{
  /* Look at the capability bitfields in the given EvdevJoystickInfo, and
   * decide whether the device is indeed a joystick. This uses the same criteria
   * that SDL does- it at least needs X and Y axes, and one joystick-like button.
   */
  if (!test_bit(EV_KEY, info.evbit))
    return false;
  if (!(test_bit(BTN_TRIGGER, info.keybit) ||
	test_bit(BTN_A, info.keybit) ||
	test_bit(BTN_1, info.keybit)))
    return false;

  if (!test_bit(EV_ABS, info.evbit))
    return false;
  if (!(test_bit(ABS_X, info.absbit) &&
	test_bit(ABS_Y, info.absbit)))
    return false;

  return true;
}

void			EvdevJoystick::initJoystick(const char* joystickName)
{
  /* Close the previous joystick */
  ffResetRumble();
  if (joystickfd > 0)
    close(joystickfd);
  currentJoystick = NULL;
  joystickfd = 0;

  if (!strcmp(joystickName, "off") || !strcmp(joystickName, "")) {
    /* No joystick configured, we're done */
    return;
  }

  std::map<std::string,EvdevJoystickInfo>::iterator iter;
  iter = joysticks.find(joystickName);
  if (iter == joysticks.end()) {
    printError("The selected joystick no longer exists.");
    return;
  }

  /* Looks like we might have a valid joystick, try to open it */
  EvdevJoystickInfo *info = &iter->second;
  joystickfd = open(info->filename.c_str(), O_RDWR | O_NONBLOCK);
  if (joystickfd > 0) {
    /* Yay, it worked */
    currentJoystick = info;
  }
  else {
    printError("Error opening the selected joystick.");
  }

  buttons = 0;
}

bool			EvdevJoystick::joystick() const
{
  return currentJoystick != NULL;
}

void                    EvdevJoystick::poll()
{
  /* Read as many input events as are available, and update our current state
   */
  struct input_event ev;
  while (read(joystickfd, &ev, sizeof(ev)) > 0) {
    switch (ev.type) {

    case EV_ABS:
      switch (ev.code) {
      case ABS_X: currentJoystick->axis_info[0].value = ev.value; break;
      case ABS_Y: currentJoystick->axis_info[1].value = ev.value; break;
      }
      break;

    case EV_KEY:
      /* Just map linux input bits directly to bits in 'buttons' */
      setButton(ev.code, ev.value);
      break;

    }
  }
}

void                    EvdevJoystick::setButton(int button_num, int state)
{
  int mask = 1<<button_num;
  if (state)
    buttons |= mask;
  else
    buttons &= ~mask;
}

void			EvdevJoystick::getJoy(int& x, int& y)
{
  if (currentJoystick) {
    poll();

    int axes[2];
    int axis;
    int value;
    for (axis=0; axis<2; axis++) {

      /* Each axis gets scaled from evdev's reported minimum
       * and maximum into bzflag's [-1000, 1000] range.
       */
      value = currentJoystick->axis_info[axis].value;
      value -= currentJoystick->axis_info[axis].minimum;
      value = value * 2000 / (currentJoystick->axis_info[axis].maximum -
			      currentJoystick->axis_info[axis].minimum);
      value -= 1000;

      /* No cheating by modifying joystick drivers, or using some that rate
       * their maximum and minimum conservatively like the input spec allows.
       */
      if (value < -1000) value = -1000;
      if (value >  1000) value =  1000;

      /* All the cool kids are doing it... */
      value = (value * abs(value)) / 1000;

      axes[axis] = value;
    }
    x = axes[0];
    y = axes[1];
  }
  else {
    x = y = 0;
  }
}

unsigned long		EvdevJoystick::getJoyButtons()
{
  if (currentJoystick) {
    poll();
    return buttons;
  }
  else {
    return 0;
  }
}

void                    EvdevJoystick::getJoyDevices(std::vector<std::string>
						 &list) const
{
  std::map<std::string,EvdevJoystickInfo>::const_iterator i;
  for (i = joysticks.begin(); i != joysticks.end(); ++i)
    list.push_back(i->first);
}

bool                    EvdevJoystick::ffHasRumble() const
{
#ifdef HAVE_FF_EFFECT_RUMBLE
  if (!currentJoystick)
    return false;
  else
    return test_bit(EV_FF, currentJoystick->evbit) &&
           test_bit(FF_RUMBLE, currentJoystick->ffbit);
#else
  return false;
#endif
}

void                    EvdevJoystick::ffResetRumble()
{
#ifdef HAVE_FF_EFFECT_RUMBLE
  /* Erase old effects before closing a device,
   * if we had any, then initialize the ff_rumble struct.
   */
  if (ffHasRumble() && ff_rumble->id != -1) {

    /* Stop the effect first */
    struct input_event event;
    event.type = EV_FF;
    event.code = ff_rumble->id;
    event.value = 0;
    write(joystickfd, &event, sizeof(event));

    /* Erase the downloaded effect */
    ioctl(joystickfd, EVIOCRMFF, ff_rumble->id);
  }

  /* Reinit the ff_rumble struct. It starts out with
   * an id of -1, prompting the driver to assign us one.
   * Once that happens, we stick with the same effect slot
   * as long as we have the device open.
   */
  memset(&ff_rumble, 0, sizeof(ff_rumble));
  ff_rumble->type = FF_RUMBLE;
  ff_rumble->id = -1;
#endif
}

void                    EvdevJoystick::ffRumble(int count,
						float delay, float duration,
						float strong_motor, float weak_motor)
{
#ifdef HAVE_FF_EFFECT_RUMBLE
  if (!ffHasRumble())
    return;

  /* Stop the previous effect we were playing, if any */
  if (ff_rumble->id != -1) {
    struct input_event event;
    event.type = EV_FF;
    event.code = ff_rumble->id;
    event.value = 0;
    write(joystickfd, &event, sizeof(event));
  }

  if (count > 0) {
    /* Download an updated effect */
    ff_rumble->u.rumble.strong_magnitude = (int) (0xFFFF * strong_motor + 0.5);
    ff_rumble->u.rumble.weak_magnitude = (int) (0xFFFF * weak_motor + 0.5);
    ff_rumble->replay.length = (int) (duration * 1000 + 0.5);
    ff_rumble->replay.delay = (int) (delay * 1000 + 0.5);
    ioctl(joystickfd, EVIOCSFF, &ff_rumble);

    /* Play it the indicated number of times */
    struct input_event event;
    event.type = EV_FF;
    event.code = ff_rumble->id;
    event.value = count;
    write(joystickfd, &event, sizeof(event));
  }
#endif
}

#endif /* HAVE_LINUX_INPUT_H */

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

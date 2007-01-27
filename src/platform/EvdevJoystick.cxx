/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
#include "bzfio.h"

#define test_bit(nr, addr) \
	(((1UL << ((nr) & 31)) & (((const unsigned int *) addr)[(nr) >> 5])) != 0)


bool	     EvdevJoystick::isEvdevAvailable()
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

void	     EvdevJoystick::scanForJoysticks(std::map<std::string,
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

    /* Can we open it for r/w? */
    info.filename = inputdirName + "/" + dent->d_name;
    int fd = open(info.filename.c_str(), O_RDWR);
    /* if we can't open read/write, try just read...if it's not ff it'll work anyhow */
    if (!fd) {
      fd = open(info.filename.c_str(), O_RDONLY);
      if (fd) logDebugMessage(4,"Opened event device %s as read-only.\n", info.filename.c_str());
    } else {
      logDebugMessage(4,"Opened event device %s as read-write.\n", info.filename.c_str());
    }
    /* no good, can't open it */
    if (!fd) {
      logDebugMessage(4,"Can't open event device %s.  Check permissions.\n", info.filename.c_str());
      continue;
    }

    /* Does it look like a joystick? */
    if (!(collectJoystickBits(fd, info) && isJoystick(info))) {
      logDebugMessage(4,"Device %s doesn't seem to be a joystick.  Skipping.\n", info.filename.c_str());
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
     *	This could be fixed by disambiguating jsname if it already
     *	exists in 'joysticks', but the user would still have a hard
     *	time knowing which device to pick.
     */
    joysticks[jsname] = info;
  }

  closedir(inputdir);
}

bool		    EvdevJoystick::collectJoystickBits(int fd, struct EvdevJoystickInfo &info)
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
  for (axis=0; axis<8; axis++) {
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
  ffResetEffect();
  if (joystickfd > 0)
    close(joystickfd);
  currentJoystick = NULL;
  joystickfd = 0;

  if (!strcasecmp(joystickName, "off") || !strcmp(joystickName, "")) {
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
    currentJoystick->readonly = false;
  } else {
    joystickfd = open(info->filename.c_str(), O_RDONLY | O_NONBLOCK);
    if (joystickfd > 0) {
      /* Got it in read only */
      currentJoystick = info;
      currentJoystick->readonly = true;
      printError("No write access to joystick device, force feedback disabled.");
    } else {
      printError("Error opening the selected joystick.");
    }
  }

  useaxis[0] = ABS_X;
  useaxis[1] = ABS_Y;
  buttons = 0;
}

bool			EvdevJoystick::joystick() const
{
  return currentJoystick != NULL;
}

void		    EvdevJoystick::poll()
{
  /* Read as many input events as are available, and update our current state
   */
  struct input_event ev;
  while (read(joystickfd, &ev, sizeof(ev)) > 0) {
    switch (ev.type) {

    case EV_ABS:
      if (ev.code - ABS_X > 8)
	break;
      currentJoystick->axis_info[ev.code - ABS_X].value = ev.value; break;
      break;

    case EV_KEY:
      setButton(mapButton(ev.code), ev.value);
      break;

    }
  }
}

int		     EvdevJoystick::mapButton(int bit_num)
{
  /* Given an evdev button number, map it back to a small integer that most
   * people would consider the button's actual number. This also ensures
   * that we can fit all buttons in "buttons" as long as the number of buttons
   * is less than the architecture's word size ;)
   *
   * We just scan through the joystick's keybits, counting how many
   * set bits we encounter before this one. If the indicated bit isn't
   * set in keybits, this is a bad event and we return -1.
   * If this linear scan becomes a noticeable performance drain, this could
   * easily be precomputed and stored in an std:map.
   */
  int i;
  int button_num = 0;
  const int total_bits = sizeof(currentJoystick->keybit)*sizeof(unsigned long)*8;

  for (i=0; i<total_bits; i++) {
    if (i == bit_num)
      return button_num;
    if (test_bit(i, currentJoystick->keybit))
      button_num++;
  }
  return -1;
}

void		    EvdevJoystick::setButton(int button_num, int state)
{

  if (button_num >= 0) {
    int mask = 1<<button_num;
    if (state)
      buttons |= mask;
    else
      buttons &= ~mask;
  }
}

void			EvdevJoystick::getJoy(int& x, int& y)
{
  if (currentJoystick) {
    poll();

    int axes[9];
    int axis;
    int value;
    for (axis=0; axis<9; axis++) {
      if (!(test_bit(ABS_X + axis, currentJoystick->absbit)))
	continue;

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
    x = axes[useaxis[0]];
    y = axes[useaxis[1]];
  } else {
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

void		    EvdevJoystick::getJoyDevices(std::vector<std::string>
						 &list) const
{
  std::map<std::string,EvdevJoystickInfo>::const_iterator i;
  for (i = joysticks.begin(); i != joysticks.end(); ++i)
    list.push_back(i->first);
}

static const std::string anames[9] = { "X", "Y", "Z", "Rx", "Ry", "Rz", "Throttle", "Rudder", "Wheel" };

void		EvdevJoystick::getJoyDeviceAxes(std::vector<std::string>
						    &list) const
{
  list.clear();

  if (!currentJoystick)
    return;

  for (int i = 0; i < 9; ++i)
    if (test_bit(ABS_X + i, currentJoystick->absbit))
      list.push_back(anames[i]);
}

void		    EvdevJoystick::setXAxis(const std::string axis)
{
  for (int i = 0; i < 9; ++i)
    if (anames[i] == axis)
      useaxis[0] = ABS_X + i;
}

void		    EvdevJoystick::setYAxis(const std::string axis)
{
  for (int i = 0; i < 9; ++i)
    if (anames[i] == axis)
      useaxis[1] = ABS_X + i;
}

bool		    EvdevJoystick::ffHasRumble() const
{
#ifdef HAVE_FF_EFFECT_RUMBLE
  if (!currentJoystick)
    return false;
  else
    return test_bit(EV_FF, currentJoystick->evbit) &&
	   test_bit(FF_RUMBLE, currentJoystick->ffbit) &&
	   !currentJoystick->readonly;
#else
  return false;
#endif
}

void		    EvdevJoystick::ffResetEffect()
{
#if (defined HAVE_FF_EFFECT_DIRECTIONAL || defined HAVE_FF_EFFECT_RUMBLE)
  /* Erase old effects before closing a device,
   * if we had any, then initialize the ff_rumble struct.
   */
  if ((ffHasRumble() || ffHasDirectional()) && ff_rumble->id != -1) {

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
  memset(ff_rumble, 0, sizeof(*ff_rumble));
  ff_rumble->id = -1;
#endif
}

#ifdef HAVE_FF_EFFECT_RUMBLE
void		    EvdevJoystick::ffRumble(int count,
						float delay, float duration,
						float strong_motor,
						float weak_motor)
{
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
    ff_rumble->type = FF_RUMBLE;
    ff_rumble->u.rumble.strong_magnitude = (int) (0xFFFF * strong_motor + 0.5);
    ff_rumble->u.rumble.weak_magnitude = (int) (0xFFFF * weak_motor + 0.5);
    ff_rumble->replay.length = (int) (duration * 1000 + 0.5);
    ff_rumble->replay.delay = (int) (delay * 1000 + 0.5);
    ioctl(joystickfd, EVIOCSFF, ff_rumble);

    /* Play it the indicated number of times */
    struct input_event event;
    event.type = EV_FF;
    event.code = ff_rumble->id;
    event.value = count;
    write(joystickfd, &event, sizeof(event));
  }
}
#else
void EvdevJoystick::ffRumble(int, float, float, float, float)
{
}
#endif

bool EvdevJoystick::ffHasDirectional() const
{
#ifdef HAVE_FF_EFFECT_DIRECTIONAL
  if (!currentJoystick)
    return false;
  else
    return test_bit(EV_FF, currentJoystick->evbit) &&
	   test_bit(FF_PERIODIC, currentJoystick->ffbit) &&
	   test_bit(FF_CONSTANT, currentJoystick->ffbit) &&
	   !currentJoystick->readonly;
#else
  return false;
#endif
}

#ifdef HAVE_FF_EFFECT_DIRECTIONAL
void EvdevJoystick::ffDirectionalConstant(int count, float delay, float duration,
					  float x_direction, float y_direction,
					  float strength)
{
  if (!ffHasDirectional())
    return;

  /* whenever we switch effect types we must reset the effect
   * this could be avoided by tracking the slot numbers and only
   * using one for each type.
   * When we don't switch types we still must stop the previous
   * effect we were playing, if any.
   */
  if (ff_rumble->type != FF_CONSTANT) {
    ffResetEffect();
  } else if (ff_rumble->id != -1) {
    struct input_event event;
    event.type = EV_FF;
    event.code = ff_rumble->id;
    event.value = 0;
    write(joystickfd, &event, sizeof(event));
  }

  if (count > 0) {
    /* Download an updated effect */
    ff_rumble->type = FF_CONSTANT;
    ff_rumble->u.constant.level = (int) (0x7FFF * strength + 0.5f);
    ff_rumble->direction = (int) (0xFFFF * (1.0 / (2.0 * M_PI)) *
				  atan2(x_direction, -y_direction) + 0.5);
    ff_rumble->u.constant.envelope.attack_length = FF_NOMINAL_MIN;
    ff_rumble->u.constant.envelope.fade_length = FF_NOMINAL_MIN;
    ff_rumble->u.constant.envelope.attack_level = ff_rumble->u.constant.level;
    ff_rumble->u.constant.envelope.fade_level = ff_rumble->u.constant.level;
    ff_rumble->replay.length = (int) (duration * 1000 + 0.5f);
    ff_rumble->replay.delay = (int) (delay * 1000 + 0.5f);
    if (ioctl(joystickfd, EVIOCSFF, ff_rumble) == -1)
      printError("Effect upload failed.");

    /* Play it the indicated number of times */
    struct input_event event;
    event.type = EV_FF;
    event.code = ff_rumble->id;
    event.value = count;
    write(joystickfd, &event, sizeof(event));
  }
}
#else
void EvdevJoystick::ffDirectionalConstant(int, float, float, float, float, float)
{
}
#endif

#ifdef HAVE_FF_EFFECT_DIRECTIONAL
void EvdevJoystick::ffDirectionalPeriodic(int count, float delay, float duration,
					  float x_direction, float y_direction,
					  float amplitude, float period,
					  PeriodicType type)
{
  if (!ffHasDirectional())
    return;

  /* whenever we switch effect types we must reset the effect
   * this could be avoided by tracking the slot numbers and only
   * using one for each type.
   * When we don't switch types we still must stop the previous
   * effect we were playing, if any.
   */
  if (ff_rumble->type != FF_PERIODIC) {
    ffResetEffect();
  } else if (ff_rumble->id != -1) {
    struct input_event event;
    event.type = EV_FF;
    event.code = ff_rumble->id;
    event.value = 0;
    write(joystickfd, &event, sizeof(event));
  }

  if (count > 0) {
    /* Download an updated effect */
    ff_rumble->type = FF_PERIODIC;
    int wave = 0;
    switch (type) {
      case FF_Sine: wave = FF_SINE; break;
      case FF_Square: wave = FF_SQUARE; break;
      case FF_Triangle: wave = FF_TRIANGLE; break;
      case FF_SawtoothUp: wave = FF_SAW_UP; break;
      case FF_SawtoothDown: wave = FF_SAW_DOWN; break;
      default: printError("Unknown periodic force feedback waveform."); return;
    }
    ff_rumble->u.periodic.waveform = wave;
    ff_rumble->u.periodic.magnitude = (int) (0x7FFF * amplitude + 0.5f);
    ff_rumble->u.periodic.period = (int) (period * 1000 + 0.5f);
    ff_rumble->u.periodic.offset = ff_rumble->u.periodic.phase = 0;
    ff_rumble->direction = (int) (0xFFFF * (1.0 / (2.0 * M_PI)) *
				  atan2(x_direction, -y_direction) + 0.5);
    ff_rumble->u.periodic.envelope.attack_length = FF_NOMINAL_MIN;
    ff_rumble->u.periodic.envelope.fade_length = FF_NOMINAL_MIN;
    ff_rumble->u.periodic.envelope.attack_level = ff_rumble->u.periodic.magnitude;
    ff_rumble->u.periodic.envelope.fade_level = ff_rumble->u.periodic.magnitude;
    ff_rumble->replay.length = (int) (duration * 1000 + 0.5f);
    ff_rumble->replay.delay = (int) (delay * 1000 + 0.5f);
    if (ioctl(joystickfd, EVIOCSFF, ff_rumble) == -1)
      printError("Effect upload failed.");

    /* Play it the indicated number of times */
    struct input_event event;
    event.type = EV_FF;
    event.code = ff_rumble->id;
    event.value = count;
    write(joystickfd, &event, sizeof(event));
  }
}
#else
void EvdevJoystick::ffDirectionalPeriodic(int, float, float, float, float, float,
					  float, PeriodicType)
{
}
#endif

#ifdef HAVE_FF_EFFECT_DIRECTIONAL
void EvdevJoystick::ffDirectionalResistance(float time, float coefficient,
					    float saturation, ResistanceType type)
{
  if (!ffHasDirectional())
    return;

  /* whenever we switch effect types we must reset the effect
   * this could be avoided by tracking the slot numbers and only
   * using one for each type.
   * When we don't switch types we still must stop the previous
   * effect we were playing, if any.
   */
  if ((ff_rumble->type != FF_SPRING) &&
      (ff_rumble->type != FF_FRICTION) &&
      (ff_rumble->type != FF_DAMPER)) {
    ffResetEffect();
  } else if (ff_rumble->id != -1) {
    struct input_event event;
    event.type = EV_FF;
    event.code = ff_rumble->id;
    event.value = 0;
    write(joystickfd, &event, sizeof(event));
  }

  if (1 > 0) {
    /* Download an updated effect */
    int lintype;
    switch (type) {
      case FF_Position: lintype = FF_SPRING; break;
      case FF_Velocity: lintype = FF_FRICTION; break;
      case FF_Acceleration: lintype = FF_DAMPER; break;
      default: printError("Unrecognized force feedback resistance type."); return;
    }
    ff_rumble->type = lintype;
    ff_rumble->u.condition[0].right_saturation =
    ff_rumble->u.condition[0].left_saturation = (int) (0x7FFF * saturation + 0.5f);
    ff_rumble->u.condition[0].right_coeff =
    ff_rumble->u.condition[0].left_coeff = (int) (0x7FFF * coefficient + 0.5f);
    ff_rumble->u.condition[0].deadband = 0;
    ff_rumble->u.condition[0].center = 0;
    ff_rumble->u.condition[1] = ff_rumble->u.condition[0];
    ff_rumble->replay.length = (int) (time * 1000 + 0.5f);
    ff_rumble->replay.delay = 0;
    if (ioctl(joystickfd, EVIOCSFF, ff_rumble) == -1)
      printError("Effect upload failed.");

    /* Play it just once */
    struct input_event event;
    event.type = EV_FF;
    event.code = ff_rumble->id;
    event.value = 1;
    write(joystickfd, &event, sizeof(event));
  }
}
#else
void EvdevJoystick::ffDirectionalResistance(float, float, float, ResistanceType)
{
}
#endif

#endif /* HAVE_LINUX_INPUT_H */

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

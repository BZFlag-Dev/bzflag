/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* EvdevJoystick:
 *	Encapsulates a joystick accessed via a linux event device.
 *      This is basically equivalent to what SDL does on linux, but we
 *      have the added bonus of supporting force feedback where SDL
 *      does not.
 *
 *      SDL does not yet support force feedback, and there is no good
 *      way to graft linux-specific force feedback support onto SDL-
 *      so we just do it all ourselves without even initializing SDL's
 *      joystick support.
 */

#ifndef BZF_EVDEV_JOY_H
#define	BZF_EVDEV_JOY_H

#include "BzfJoystick.h"
#include <map>
#include <string>
#include <vector>

struct EvdevJoystickInfo {
  /* Information about a joystick that's installed on the
   * system but not necessarily the one we're using. This
   * is collected at EvdevJoystick initialization time.
   */

  std::string filename;

  /* Bits describing the capabilities of this device. linux/input.h
   * includes macros and constants necessary to get useful info
   * out of this.
   */
  unsigned long evbit [16];
  unsigned long keybit[64];
  unsigned long absbit[16];
  unsigned long ffbit [16];

  struct {
    int value;
    int minimum;
    int maximum;
    int fuzz;
    int flat;
  } axis_info[9];

  /* if we can only read from this joystick, remember that */
  bool readonly;
};

class EvdevJoystick : public BzfJoystick {
  public:
		EvdevJoystick();
		~EvdevJoystick();

    void	initJoystick(const char* joystickName);
    bool	joystick() const;
    void	getJoy(int& x, int& y);
    unsigned long getJoyButtons();
    void	getJoyDevices(std::vector<std::string> &list) const;

    void	getJoyDeviceAxes(std::vector<std::string> &list) const;
    void	setXAxis(const std::string axis);
    void	setYAxis(const std::string axis);

    bool	ffHasRumble() const;
    void	ffRumble(int count,
			 float delay, float duration,
			 float strong_motor, float weak_motor=0.0f);

    bool	ffHasDirectional() const;
    void	ffDirectionalConstant(int count,
				      float delay, float duration,
				      float x_direction, float y_direction,
				      float strength);
    void	ffDirectionalPeriodic(int count,
				      float delay, float duration,
				      float x_direction, float y_direction,
				      float amplitude, float period,
				      PeriodicType type);
    void	ffDirectionalResistance(float time, float coefficient,
					float saturation, ResistanceType type);

    /* Test whether this driver should be used without actually
     * loading it. Will return false if no event devices can be
     * located, or if it has been specifically disabled by setting
     * the environment variable BZFLAG_ENABLE_EVDEV=0
     */
    static bool isEvdevAvailable();

  private:
    static void scanForJoysticks(std::map<std::string,EvdevJoystickInfo> &joysticks);
    static bool collectJoystickBits(int fd, struct EvdevJoystickInfo &info);
    static bool isJoystick(struct EvdevJoystickInfo &info);

    void	poll();
    void	setButton(int button_num, int state);
    int	 mapButton(int bit_num);
    void	ffResetEffect();

    std::map<std::string,EvdevJoystickInfo> joysticks;

    int		useaxis[2];
    EvdevJoystickInfo*	  currentJoystick;
    int			 joystickfd;
    int			 buttons;
    struct ff_effect*	   ff_rumble;
};

// The drivers don't define a minimum delay that I can tell.  This seems to work.
#define FF_NOMINAL_MIN 0x10;

#endif // BZF_EVDEV_JOY_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

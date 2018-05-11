/* bzflag
 * Copyright (c) 1993-2018 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* DXJoystick:
 *	Encapsulates a Windows DirectInput 7+ joystick with
 *	force feedback support
 */

#ifndef BZF_DXJOY_H
#define	BZF_DXJOY_H

#include "BzfJoystick.h"

// only require a runtime that has what we actually use (e.g. force feedback support)
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <vector>
#include <string>
#include <map>

typedef std::map<std::string, LPDIRECTINPUTEFFECT> EffectMap;

class DXJoystick : public BzfJoystick {
  public:
		DXJoystick();
		~DXJoystick();

    void	initJoystick(const char* joystickName);
    bool	joystick() const;
    void	getJoy(int& x, int& y);
    unsigned long getJoyButtons();
    int	getNumHats();
    void	getJoyHat(int hat, float &hatX, float &hatY);
    void	getJoyDevices(std::vector<std::string> &list) const;
    void	getJoyDeviceAxes(std::vector<std::string> &list) const;
    void	setXAxis(const std::string &axis);
    void	setYAxis(const std::string &axis);
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

	int		numberOfHats;
  private:
    DIJOYSTATE	pollDevice();
    void	reaquireDevice();
    void	enumerateDevices();
    void	resetFF();



    void	DXError(const char* situation, HRESULT problem);

    std::map<std::string,bool> axes;
    std::string xAxis;
    std::string yAxis;

    std::vector<float> hataxes;

    static std::vector<DIDEVICEINSTANCE> devices;
    static EffectMap effectDatabase;

    IDirectInput8* directInput;
    IDirectInputDevice8* device;

    /* Nasty callbacks 'cause DirectX sucks */

    static BOOL CALLBACK deviceEnumCallback(LPCDIDEVICEINSTANCE device, void*);
};

#endif // BZF_DXJOY_H

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

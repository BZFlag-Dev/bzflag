/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
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
	#define BZF_DXJOY_H

	#include "BzfJoystick.h"

	#if !defined(BROKEN_DINPUT)

// only require a runtime that has what we actually use (e.g. force feedback support)
		#define DIRECTINPUT_VERSION 0x0700
		#include <dinput.h>

// Don't try compile this if we don't have an up-to-date DX
		#if defined(DIRECTINPUT_HEADER_VERSION) && (DIRECTINPUT_HEADER_VERSION >= 0x0700)
// We can use DInput.  It's not broken, and it's new enough
			#define USE_DINPUT 1
		#else 
// DInput is not new enough to use
			#if defined(USE_DINPUT)
				#undef USE_DINPUT
			#endif 
		#endif 

	#else 

// Make sure we don't use DInput at all (even headers) if it's broken
		#if defined(USE_DINPUT)
			#undef USE_DINPUT
		#endif 

	#endif 

	#if defined(USE_DINPUT)

		#include <vector>
		#include <string>
		#include <map>

typedef std::map < std::string, LPDIRECTINPUTEFFECT > EffectMap;

class DXJoystick: public BzfJoystick
{
public:
	DXJoystick();
	~DXJoystick();

	void initJoystick( const char *joystickName );
	bool joystick()const;
	void getJoy( int &x, int &y );
	unsigned long getJoyButtons();
	void getJoyDevices( std::vector < std::string >  &list )const;
	void getJoyDeviceAxes( std::vector < std::string >  &list )const;
	void setXAxis( const std::string axis );
	void setYAxis( const std::string axis );
	bool ffHasRumble()const;
	void ffRumble( int count, float delay, float duration, float strong_motor, float weak_motor = 0.0f );
	bool ffHasDirectional()const;
	void ffDirectionalConstant( int count, float delay, float duration, float x_direction, float y_direction, float strength );
	void ffDirectionalPeriodic( int count, float delay, float duration, float x_direction, float y_direction, float amplitude, float period, PeriodicType type );
	void ffDirectionalResistance( float time, float coefficient, float saturation, ResistanceType type );

private:
	DIJOYSTATE pollDevice();
	void reaquireDevice();
	void enumerateDevices();
	void resetFF();

	void DXError( const char *situation, HRESULT problem );

	std::map < std::string, bool > axes;
	std::string xAxis;
	std::string yAxis;

	static std::vector < DIDEVICEINSTANCE > devices;
	static EffectMap effectDatabase;

	IDirectInput7 *directInput;
	IDirectInputDevice7 *device;

	/* Nasty callbacks 'cause DirectX sucks */

	static BOOL CALLBACK deviceEnumCallback( LPCDIDEVICEINSTANCE device, void* );
};

	#endif // USE_DINPUT

#endif // BZF_DXJOY_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

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

/* interface header */
#include "DXJoystick.h"

/* system headers */
#include <vector>
#include <string>

std::vector<DIDEVICEINSTANCE> DXJoystick::devices;

DXJoystick::DXJoystick() : device(NULL)
{
  HINSTANCE hinst = GetModuleHandle(NULL);
  HRESULT success = DirectInputCreateEx(hinst, DIRECTINPUT_VERSION, IID_IDirectInput7,
					(void**)&directInput, NULL);

  if (success != DI_OK) {
    // Oops, could not create directinput object - what to do now?
    return;
  }

  directInput->EnumDevices(DIDEVTYPE_JOYSTICK, &deviceEnumCallback,
			   NULL, DIEDFL_ATTACHEDONLY);

  if (success != DI_OK) {
    // Oops, could not enumerate devices - what to do now?
    return;
  }

}

DXJoystick::~DXJoystick()
{
  // unacquire the joystick
  if (device)
    device->Unacquire();

  // release DX objects
  if (device) {
    device->Release();
    device = NULL;
  }
  if (directInput) {
    directInput->Release();
    directInput = NULL;
  }
}

void	      DXJoystick::initJoystick(const char* joystickName)
{
  /*
   * Find this device, and try to initialize it.
   */
  GUID thisDevice;
  for (unsigned int i = 0; i < devices.size(); i++) {
    if (joystickName = devices[i].tszProductName)
      thisDevice = devices[i].guidInstance;
  }
  HRESULT success = directInput->CreateDeviceEx(thisDevice,
						IID_IDirectInputDevice7,
						(void**)&device, NULL);

  if (success != DI_OK) {
    // couldn't create (device might not exist)
    return;
  }

  /*
   * Set device cooperation level - all input on this device goes to BZFlag,
   * because we're greedy.
   */

  /* FIXME - don't have hwnd here, would have to get it from WinWindow
  success = device->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_EXCLUSIVE);

  if (success != DI_OK) {
    // couldn't grab device, what to do now?
    device = NULL;
    return;
  }
  */

  /*
   * Set the device data format.  We want buttons and axis, so we'll use a
   * predefined Joystick structure.
   */

  success = device->SetDataFormat(&c_dfDIJoystick);

  if (success != DI_OK) {
    // couldn't set data format, what to do now?
    device = NULL;
    return;
  }

  /*
   * Set data ranges for both axes.  This has the side effect
   * of ensuring that the joystick has at least two axes.
   */

  DIPROPRANGE range;
  range.diph.dwSize       = sizeof(range);
  range.diph.dwHeaderSize = sizeof(range.diph);
  range.diph.dwObj        = DIJOFS_X;
  range.diph.dwHow        = DIPH_BYOFFSET;
  range.lMin              = -1000;
  range.lMax              = +1000;

  success = device->SetProperty(DIPROP_RANGE, &range.diph);

  if (success != DI_OK) {
     // couldn't set x axis range, what to do now?
    device = NULL;
    return;
  }

  range.diph.dwObj	  = DIJOFS_Y;

  success = device->SetProperty(DIPROP_RANGE, &range.diph);

  if (success != DI_OK) {
    // couldn't set y axis range, what to do now?
    device = NULL;
    return;
  }

  /*
   * Acquire the device so that we can get input from it.
   */

  success = device->Acquire();
  
  if (success != DI_OK) {
    // couldn't acquire, what to do now?
    device = NULL;
    return;
  }
}

bool	      DXJoystick::joystick() const
{
  return (device != NULL);
}

void	      DXJoystick::getJoy(int& x, int& y)
{
  if (!device) return;

  DIJOYSTATE state = pollDevice();

  x = state.lX;
  y = state.lY;

  // ballistics
  x = (x * abs(x)) / 1000;
  y = (y * abs(y)) / 1000;

  return;
}

unsigned long DXJoystick::getJoyButtons()
{
  if (!device) return 0;

  DIJOYSTATE state = pollDevice();

  unsigned long buttons = 0;

  for (int i = 0; i < 32; i++) {
    if (state.rgbButtons[i] & 0x80)
      buttons |= (1 << i);
  }
  
  return buttons;
}

DIJOYSTATE    DXJoystick::pollDevice()
{
  DIJOYSTATE state;

  HRESULT success = device->Poll();
  // umm, ignore that result...yeah

  success = device->GetDeviceState(sizeof(DIJOYSTATE), &state);
  if (success != DI_OK) {
    // got no state, what's wrong?
    if (success == DIERR_INPUTLOST) {
      // try to reaquire the device
      success = device->Acquire();
  
      if (success != DI_OK) {
	// couldn't acquire, what to do now?
	device = NULL;
	return state;
      }
    }
  }

  return state;
}

void	      DXJoystick::getJoyDevices(std::vector<std::string> &list) const
{
  for (unsigned int i = 0; i < devices.size(); i++) {
    list.push_back(devices[i].tszProductName);
  }
}

/*
 * Force feedback functions for future use.
 */

bool	      DXJoystick::ffHasRumble() const
{
  return false;
}

void	      DXJoystick::ffRumble(int count, float delay, float duration,
				   float strong_motor, float weak_motor)
{
  return;
}


/* Nasty callbacks 'cause DirectX sucks */

BOOL CALLBACK DXJoystick::deviceEnumCallback(LPCDIDEVICEINSTANCE device, void* /*pvRef*/)
{
  if (!device)
    return DIENUM_STOP;

  devices.push_back(*device);

  return DIENUM_CONTINUE;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

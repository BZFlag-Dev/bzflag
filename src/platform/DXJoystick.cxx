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

/* local impl. headers */
#include "WinWindow.h"
#include "ErrorHandler.h"
#include "TextUtils.h"

std::vector<DIDEVICEINSTANCE> DXJoystick::devices;

DXJoystick::DXJoystick() : device(NULL)
{
  HINSTANCE hinst = GetModuleHandle(NULL);
  HRESULT success = DirectInputCreateEx(hinst, DIRECTINPUT_VERSION, IID_IDirectInput7,
					(void**)&directInput, NULL);

  if (success != DI_OK) {
    printError("Could not initialize DirectInput.");
    return;
  }

  directInput->EnumDevices(DIDEVTYPE_JOYSTICK, &deviceEnumCallback,
			   NULL, DIEDFL_ATTACHEDONLY);

  if (success != DI_OK) {
    printError("Could not enumerate DirectInput devices.");
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
		{
      thisDevice = devices[i].guidInstance;
      break;
    }
  }
  HRESULT success = directInput->CreateDeviceEx(thisDevice,
						IID_IDirectInputDevice7,
						(void**)&device, NULL);

  if (success != DI_OK) {
    printError(string_util::format("Could not initialize DirectInput device: %s", joystickName));
    return;
  }

  /*
   * Set device cooperation level - all input on this device goes to BZFlag,
   * because we're greedy.
   */

  success = device->SetCooperativeLevel(WinWindow::getHandle(), 
					DISCL_BACKGROUND | DISCL_EXCLUSIVE);

  if (success != DI_OK) {
    // couldn't grab device, what to do now?
    printError(string_util::format("Could not set exclusive mode on %s", joystickName));
    device = NULL;
    return;
  }

  /*
   * Set the device data format.  We want buttons and axis, so we'll use a
   * predefined Joystick structure.
   */

  success = device->SetDataFormat(&c_dfDIJoystick);

  if (success != DI_OK) {
    // couldn't set data format, what to do now?
    printError(string_util::format("Could not set return data format for %s", joystickName));
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
    printError(string_util::format("Could not set X-axis ranges for %s", joystickName));
    device = NULL;
    return;
  }

  range.diph.dwObj	  = DIJOFS_Y;

  success = device->SetProperty(DIPROP_RANGE, &range.diph);

  if (success != DI_OK) {
    // couldn't set y axis range, what to do now?
    printError(string_util::format("Could not set Y-axis ranges for %s", joystickName));
    device = NULL;
    return;
  }

  /*
   * Acquire the device so that we can get input from it.
   */

  success = device->Acquire();
  
  if (success != DI_OK) {
    // couldn't acquire, what to do now?
    printError(string_util::format("Could not aquire %s", joystickName));
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
    } else {
      printError("Acquisition succeeded, but could not get joystick status.");
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
 * Cool force feedback functions.
 */

bool	      DXJoystick::ffHasRumble() const
{
  if (!device)
    return false;
  
  DIDEVCAPS caps;
  caps.dwSize = sizeof(DIDEVCAPS);

  HRESULT success = device->GetCapabilities(&caps);

  if (success != DI_OK) {
    // couldn't get capabilities, assume no force feedback
    printError("Could not get joystick capabilities, assuming no force feedback.");
    return false;
  }

  // if we support force feedback, assume we support rumble
  if (caps.dwFlags & DIDC_FORCEFEEDBACK)
    return true;

  return false;
}

void	      DXJoystick::ffRumble(int count, float delay, float duration,
				   float strong_motor, float weak_motor)
{
  /*
   * Create a constant "rumbling" effect with the specified parameters
   * Note that on joysticks that support "real" force feedback this will
   * probably just feel like a constant pressure.
   */
  DICONSTANTFORCE constantForce;

  /* This is about consistent with the relative strength of the motors
   * in the Logitech Cordless Rumblepad, which seems to be what the Linux
   * FF_RUMBLE API (which bzflag's is patterned after) was designed for.
   */
  float combined = strong_motor + weak_motor / 2.0f;
  if (combined > 1.0f)
    combined = 1.0f;

  constantForce.lMagnitude = (LONG)(DI_FFNOMINALMAX * combined);

  /*
   * Build the actual effect
   */
  DWORD axes[3] = {DIJOFS_X, DIJOFS_Y, DIJOFS_Z};
  LONG  dir[3] = {1, 1, 1};

  LPDIRECTINPUTEFFECT createdEffect;

  DIEFFECT effect;
  effect.dwSize = sizeof(DIEFFECT);
  // coordinate system really doesn't matter for rumbles but we need to specify it.
  effect.dwFlags = DIEFF_OBJECTOFFSETS | DIEFF_CARTESIAN;
  // duration
  effect.dwDuration = (DWORD)(duration * DI_SECONDS);
  // defaults
  effect.dwSamplePeriod = 0;
  effect.dwGain = DI_FFNOMINALMAX;
  effect.dwTriggerButton = DIEB_NOTRIGGER;
  effect.dwTriggerRepeatInterval = 0;
  // all axes
  effect.cAxes = 3;
  effect.rgdwAxes = &axes[0];
  // direction doesn't matter
  effect.rglDirection = &dir[0];
  // no envelope
  effect.lpEnvelope = NULL;
  // use the constant force data
  effect.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
  effect.lpvTypeSpecificParams = &constantForce;
  // start delay
  effect.dwStartDelay = (DWORD)(delay * DI_SECONDS);

  // create the effect
  HRESULT success = device->CreateEffect(GUID_ConstantForce, &effect, &createdEffect, NULL);

  if (success != DI_OK) {
    // uh-oh, no worky
    char buffer[40] = {0};
    if (success == DIERR_DEVICENOTREG)
      sprintf(buffer, "Device not registered");
    else if (success == DIERR_DEVICEFULL)
      sprintf(buffer, "Device is full");
    else if (success == DIERR_INVALIDPARAM)
      sprintf(buffer, "Invalid parameter");
    else if (success == DIERR_NOTINITIALIZED)
      sprintf(buffer, "Device not initialized");
    else
      sprintf(buffer, "Unknown error");
    printError(string_util::format("Could not create rumble effect (%s).", buffer));
    return;
  }

  // play the thing
  success = createdEffect->Start(count, 0);

  if (success != DI_OK) {
    // uh-oh, no worky
    printError("Could not play rumble effect.");
  }

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

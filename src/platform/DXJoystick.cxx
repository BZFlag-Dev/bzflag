/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
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

// Don't try compile this if we don't have an up-to-date, working DX
#if defined(USE_DINPUT)

/* system headers */
#include <vector>
#include <string>
#include <map>

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
    DXError("Could not initialize DirectInput", success);
    return;
  }

  enumerateDevices();
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
  // turn it off
  if (!joystickName || strcasecmp(joystickName, "off") == 0) {
    device = NULL;
    return;
  }

  /*
   * Find this device, and try to initialize it.
   */
  GUID thisDevice;
  for (unsigned int i = 0; i < devices.size(); i++) {
    if (strcmp(joystickName, devices[i].tszProductName) == 0) {
      thisDevice = devices[i].guidInstance;
      break;
    }
  }
  HRESULT success = directInput->CreateDeviceEx(thisDevice,
						IID_IDirectInputDevice7,
						(void**)&device, NULL);

  if (success != DI_OK) {
    DXError("Could not initialize device", success);
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
    DXError("Could not set exclusive mode", success);
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
    DXError("Could not set data format", success);
    device = NULL;
    return;
  }

  /*
   * Set data ranges for all axes, and find out which ones succeeded.
   * This has the side effect of ensuring that the joystick has at least two axes.
   */

  // we assume the presence of an X and Y axis and bail if there's not one
  axes["X"]  = true;
  xAxis = "X";
  axes["Y"]  = true;
  yAxis = "Y";
  // the rest we assume don't exist, and correct ourselves if we're wrong
  axes["Z"]  = false;
  axes["Rx"] = false;
  axes["Ry"] = false;
  axes["Rz"] = false;

  DIPROPRANGE range;
  range.diph.dwSize       = sizeof(range);
  range.diph.dwHeaderSize = sizeof(range.diph);
  range.diph.dwHow	  = DIPH_BYOFFSET;
  range.lMin		  = -1000;
  range.lMax		  = +1000;

  range.diph.dwObj = DIJOFS_X;
  success = device->SetProperty(DIPROP_RANGE, &range.diph);
  if (success != DI_OK) {
    // couldn't set x axis range, what to do now?
    DXError("Could not set X-axis range", success);
    device = NULL;
    return;
  }

  range.diph.dwObj = DIJOFS_Y;
  success = device->SetProperty(DIPROP_RANGE, &range.diph);
  if (success != DI_OK) {
    // couldn't set y axis range, what to do now?
    DXError("Could not set Y-axis range", success);
    device = NULL;
    return;
  }

  range.diph.dwObj = DIJOFS_Z;
  success = device->SetProperty(DIPROP_RANGE, &range.diph);
  if (success == DI_OK)
    axes["Z"] = true;

  range.diph.dwObj = DIJOFS_RX;
  success = device->SetProperty(DIPROP_RANGE, &range.diph);
  if (success == DI_OK)
    axes["Rx"] = true;

  range.diph.dwObj = DIJOFS_RY;
  success = device->SetProperty(DIPROP_RANGE, &range.diph);
  if (success == DI_OK)
    axes["Ry"] = true;

  range.diph.dwObj = DIJOFS_RZ;
  success = device->SetProperty(DIPROP_RANGE, &range.diph);
  if (success == DI_OK)
    axes["Rz"] = true;

  /*
   * Acquire the device so that we can get input from it.
   */

  reaquireDevice();
}

bool	      DXJoystick::joystick() const
{
  return (device != NULL);
}

void	      DXJoystick::getJoy(int& x, int& y)
{
  if (!device) return;

  DIJOYSTATE state = pollDevice();

  if (xAxis == "X")	 x = state.lX;
  else if (xAxis == "Y")  x = state.lY;
  else if (xAxis == "Z")  x = state.lZ;
  else if (xAxis == "Rx") x = state.lRx;
  else if (xAxis == "Ry") x = state.lRy;
  else if (xAxis == "Rz") x = state.lRz;

  if (yAxis == "X")	 y = state.lX;
  else if (yAxis == "Y")  y = state.lY;
  else if (yAxis == "Z")  y = state.lZ;
  else if (yAxis == "Rx") y = state.lRx;
  else if (yAxis == "Ry") y = state.lRy;
  else if (yAxis == "Rz") y = state.lRz;

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
    DXError("Acquisition succeeded, but could not get joystick status", success);
  }

  return state;
}

void	      DXJoystick::getJoyDevices(std::vector<std::string> &list) const
{
  for (unsigned int i = 0; i < devices.size(); i++) {
    list.push_back(devices[i].tszProductName);
  }
}

void	      DXJoystick::getJoyDeviceAxes(std::vector<std::string> &list) const
{
  list.clear();
  std::map<std::string,bool>::const_iterator itr = axes.begin();
  while (itr != axes.end()) {
    if (itr->second == true)
      list.push_back(itr->first);
    ++itr;
  }
}

void	      DXJoystick::setXAxis(std::string axis)
{
  if (axes[axis] == false) return;
  xAxis = axis;
}

void	      DXJoystick::setYAxis(std::string axis)
{
  if (axes[axis] == false) return;
  yAxis = axis;
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
    printError("Could not get joystick capabilities, assuming no force feedback");
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
  if (!device)
    return;

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
  DWORD axes[2] = {DIJOFS_X, DIJOFS_Y};
  LONG  dir[2] = {1, 1};

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
  // x and y axes
  effect.cAxes = 2;
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
    DXError("Could not create rumble effect", success);
    return;
  }

  // play the thing
  success = createdEffect->Start(count, 0);

  if (success != DI_OK) {
    // uh-oh, no worky
    DXError("Could not play rumble effect", success);
  }

  return;
}

void DXJoystick::enumerateDevices()
{
  if (!directInput)
    return;

  devices.clear();

  HRESULT success = directInput->EnumDevices(DIDEVTYPE_JOYSTICK,
					     &deviceEnumCallback, NULL,
					     DIEDFL_ATTACHEDONLY);

  if (success != DI_OK) {
    DXError("Could not enumerate DirectInput devices", success);
    return;
  }
}

void DXJoystick::reaquireDevice()
{
  if (!device)
    return;

  // try to reaquire the device
  HRESULT success = device->Acquire();

  if (success != DI_OK) {
    // couldn't acquire, what to do now?
    device = NULL;
    DXError("Could not acquire device", success);
  }
}

void DXJoystick::resetFF()
{
  if (!device)
    return;

  HRESULT success = device->SendForceFeedbackCommand(DISFFC_RESET);

  if (success != DI_OK) {
    // couldn't reset, what to do now?
    device = NULL;
    DXError("Could not reset force feedback device", success);
  }
}

/* error handling */

void DXJoystick::DXError(const char* situation, HRESULT problem)
{
  // uh-oh, no worky
  char buffer[40] = {0};

  // some stuff we can handle
  if (problem == (HRESULT)DIERR_UNPLUGGED) {
    device = NULL;
    printError("Joystick device in use has been unplugged.");
    enumerateDevices();
    return;
  }
  if (problem == (HRESULT)DIERR_INPUTLOST) {
    reaquireDevice();
    return;
  }
  if (problem == (HRESULT)DIERR_DEVICEFULL) {
    printError("DirectInput device is full.  Resetting FF state.");
    resetFF();
    return;
  }

  // print error messages
  if (problem == (HRESULT)DIERR_DEVICENOTREG)
    sprintf(buffer, "Device not registered");
  else if (problem == (HRESULT)DIERR_INVALIDPARAM)
    sprintf(buffer, "Invalid parameter");
  else if (problem == (HRESULT)DIERR_NOTINITIALIZED)
    sprintf(buffer, "Device not initialized");
  else if (problem == (HRESULT)DI_BUFFEROVERFLOW)
    sprintf(buffer, "Buffer overflow");
  else if (problem == (HRESULT)DIERR_BADDRIVERVER)
    sprintf(buffer, "Bad or incompatible device driver");
  else if (problem == (HRESULT)DIERR_EFFECTPLAYING)
    sprintf(buffer, "Effect already playing");
  else if (problem == (HRESULT)DIERR_INCOMPLETEEFFECT)
    sprintf(buffer, "Incomplete effect");
  else if (problem == (HRESULT)DIERR_MOREDATA)
    sprintf(buffer, "Return buffer not large enough");
  else if (problem == (HRESULT)DIERR_NOTACQUIRED)
    sprintf(buffer, "Device not acquired");
  else if (problem == (HRESULT)DIERR_NOTDOWNLOADED)
    sprintf(buffer, "Effect not downloaded");
  else if (problem == (HRESULT)DIERR_NOTINITIALIZED)
    sprintf(buffer, "Device not initialized");
  else if (problem == (HRESULT)DIERR_OUTOFMEMORY)
    sprintf(buffer, "Out of memory");
  else if (problem == (HRESULT)DIERR_UNSUPPORTED)
    sprintf(buffer, "Action not supported by driver");
  else
    sprintf(buffer, "Unknown error (%d)", (int)problem);
  printError(TextUtils::format("%s (%s).", situation, buffer));
}

/* Nasty callbacks 'cause DirectX sucks */

BOOL CALLBACK DXJoystick::deviceEnumCallback(LPCDIDEVICEINSTANCE device, void* /*pvRef*/)
{
  if (!device)
    return DIENUM_STOP;

  devices.push_back(*device);

  return DIENUM_CONTINUE;
}

#endif


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

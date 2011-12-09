/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Joystick.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Joystick Input class
*/

#include "MemDebug.h"
#include "Joystick.h"
#include "MachineInfo.h"
#include "Game.h"

#define DIRECTINPUT_VERSION 0x0700

#include <dinput.h>

#define JOY_POVUPRIGHT   4500
#define JOY_POVDNRIGHT  13500
#define JOY_POVDNLEFT   22500
#define JOY_POVUPLEFT   31500

// +--------------------------------------------------------------------+
// DIRECT INPUT SUPPORT

const  int                    MAX_DEVICES = 8;

static LPDIRECTINPUT7         pdi   = 0;
static LPDIRECTINPUTDEVICE7   pdev  = 0;
static DIDEVICEINSTANCE       devices[MAX_DEVICES];
static int                    ndev  = 0;
static int                    idev  = -1;
static int                    strikes = 3;

static Joystick*              joystick = 0;

void   DirectInputError(const char* msg, HRESULT err);
char*  DIErrStr(HRESULT hr);
void   ReleaseDirectInput();

// +--------------------------------------------------------------------+

Joystick::Joystick()
: x(0), y(0), z(0), p(0), r(0), w(0), t(0)
{
	if (!joystick)
	joystick = this;

	select      =   1;
	rudder      =   0;
	throttle    =   1;
	sensitivity =  25;
	dead_zone   = 100;

	for (int i = 0; i < MotionController::MaxActions; i++)
	action[i] = false;

	for (int i = 0; i < KEY_MAP_SIZE; i++)
	map[i] = 0;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			hat[i][j] = false;
		}
	}

	map_axis[0] = KEY_JOY_AXIS_X;
	map_axis[1] = KEY_JOY_AXIS_Y;
	map_axis[2] = KEY_JOY_AXIS_RZ;
	map_axis[3] = KEY_JOY_AXIS_S0;

	inv_axis[0] = false;
	inv_axis[1] = false;
	inv_axis[2] = false;
	inv_axis[3] = false;

	if (MachineInfo::GetDirectXVersion() < MachineInfo::DX_7) {
		Print("Joystick: DI7 not found, using multimedia library\n");
		pdi  = 0;
		pdev = 0;
	}

	else if (!pdi) {
		HRESULT hr = DirectInputCreateEx(Game::GetHINST(), 
		DIRECTINPUT_VERSION, 
		IID_IDirectInput7, 
		(void**)&pdi,
		NULL); 
		if FAILED(hr) { 
			DirectInputError("Failed to initialize DI7", hr);
			pdi  = 0;
			pdev = 0;
		}
		else {
			Print("Joystick: initialized DI7 pdi = %08x\n", (DWORD) pdi);
		}
	}
}

Joystick::~Joystick()
{
	ReleaseDirectInput();
	joystick = 0;
}

void ReleaseDirectInput()
{
	if (pdev) {
		pdev->Unacquire();
		pdev->Release();
		pdev = 0;
	}

	if (pdi) {
		pdi->Release();
		pdi = 0;
	}
}

Joystick* Joystick::GetInstance()
{
	return joystick;
}

// +--------------------------------------------------------------------+

void
Joystick::MapKeys(KeyMapEntry* mapping, int nkeys)
{
	ZeroMemory(map, sizeof(map));

	for (int i = 0; i < nkeys; i++) {
		KeyMapEntry k = mapping[i];

		if (k.act >= KEY_MAP_FIRST && k.act < KEY_MAP_LAST) {
			if (k.act == KEY_JOY_SENSE)
			sensitivity = k.key;

			else if (k.act == KEY_JOY_DEAD_ZONE)
			dead_zone = k.key;

			else if (k.act == KEY_JOY_SWAP)
			swapped = k.key;

			else if (k.act == KEY_JOY_RUDDER)
			rudder = k.key;

			else if (k.act == KEY_JOY_THROTTLE)
			throttle = k.key;

			else if (k.act == KEY_JOY_SELECT)
			select = k.key;


			else if (k.act == KEY_AXIS_YAW)
			map_axis[0] = k.key;

			else if (k.act == KEY_AXIS_PITCH)
			map_axis[1] = k.key;

			else if (k.act == KEY_AXIS_ROLL)
			map_axis[2] = k.key;

			else if (k.act == KEY_AXIS_THROTTLE)
			map_axis[3] = k.key;


			else if (k.act == KEY_AXIS_YAW_INVERT)
			inv_axis[0] = k.key ? true : false;

			else if (k.act == KEY_AXIS_PITCH_INVERT)
			inv_axis[1] = k.key ? true : false;

			else if (k.act == KEY_AXIS_ROLL_INVERT)
			inv_axis[2] = k.key ? true : false;

			else if (k.act == KEY_AXIS_THROTTLE_INVERT)
			inv_axis[3] = k.key ? true : false;

			else if (k.key >= KEY_JOY_1 && k.key <= KEY_JOY_32)
			map[k.act] = k.key;

			else if (k.alt >= KEY_JOY_1 && k.alt <= KEY_JOY_32)
			map[k.act] = k.alt;

			else if (k.joy >= KEY_JOY_1 && k.joy <= KEY_JOY_32)
			map[k.act] = k.joy;

			else if (k.key >= KEY_POV_0_UP && k.key <= KEY_POV_3_RIGHT)
			map[k.act] = k.key;

			else if (k.alt >= KEY_POV_0_UP && k.alt <= KEY_POV_3_RIGHT)
			map[k.act] = k.alt;

			else if (k.joy >= KEY_POV_0_UP && k.joy <= KEY_POV_3_RIGHT)
			map[k.act] = k.joy;
		}
	}
}

// +--------------------------------------------------------------------+

static inline double sqr(double a) { return a*a; }

BOOL FAR PASCAL EnumJoystick(LPCDIDEVICEINSTANCE pdinst, LPVOID pvSelect) 
{
	CopyMemory(&devices[ndev++], pdinst, pdinst->dwSize);

	::Print("EnumJoystick %d: '%s'\n", ndev, pdinst->tszInstanceName);
	::Print("   guid:    {%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x} \n",
	(DWORD)  pdinst->guidInstance.Data1,
	(WORD)   pdinst->guidInstance.Data2,
	(WORD)   pdinst->guidInstance.Data3,
	(BYTE)   pdinst->guidInstance.Data4[0],
	(BYTE)   pdinst->guidInstance.Data4[1],
	(BYTE)   pdinst->guidInstance.Data4[2],
	(BYTE)   pdinst->guidInstance.Data4[3],
	(BYTE)   pdinst->guidInstance.Data4[4],
	(BYTE)   pdinst->guidInstance.Data4[5]);

	if (ndev < MAX_DEVICES)
	return DIENUM_CONTINUE;

	return DIENUM_STOP;
}

bool CreateDevice(int select)
{
	if (!pdi || ndev < select)
	return false;

	LPCDIDEVICEINSTANCE pdinst = &devices[select-1];

	::Print("Joystick CreateDevice(%d)\n", select);
	::Print("   name:    %s\n\n",   pdinst->tszInstanceName);

	// release current device before trying to get another:
	if (pdev) {
		pdev->Unacquire();
		pdev->Release();
		pdev = 0;
	}

	HRESULT hr = DI_OK;
	// Create the DirectInput joystick device:
	hr = pdi->CreateDeviceEx(pdinst->guidInstance, 
	IID_IDirectInputDevice7, 
	(void**)&pdev, 
	NULL);

	if (hr != DI_OK || pdev == 0) { 
		DirectInputError("Create Device Ex failed", hr);
		return false; 
	}

	// Set the data format:
	hr = pdev->SetDataFormat(&c_dfDIJoystick);

	if (hr != DI_OK) {
		DirectInputError("Set Data Format failed", hr);
		pdev->Release();
		pdev = 0;
		return false;
	}

	// Set the coop level:
	hr = pdev->SetCooperativeLevel(Game::GetHWND(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

	if (hr != DI_OK) { 
		DirectInputError("Set Cooperative Level failed", hr); 
		pdev->Release(); 
		return false; 
	}

	// Set data ranges
	DIPROPRANGE diprg; 
	diprg.lMin              = -32768;
	diprg.lMax              = +32768;

	diprg.diph.dwSize       = sizeof(diprg);
	diprg.diph.dwHeaderSize = sizeof(diprg.diph);
	diprg.diph.dwObj        = DIJOFS_X;
	diprg.diph.dwHow        = DIPH_BYOFFSET;
	pdev->SetProperty(DIPROP_RANGE, &diprg.diph);

	diprg.diph.dwObj        = DIJOFS_Y;
	pdev->SetProperty(DIPROP_RANGE, &diprg.diph);

	diprg.diph.dwObj        = DIJOFS_Z;
	pdev->SetProperty(DIPROP_RANGE, &diprg.diph);

	diprg.diph.dwObj        = DIJOFS_RX;
	pdev->SetProperty(DIPROP_RANGE, &diprg.diph);

	diprg.diph.dwObj        = DIJOFS_RY;
	pdev->SetProperty(DIPROP_RANGE, &diprg.diph);

	diprg.diph.dwObj        = DIJOFS_RZ;
	pdev->SetProperty(DIPROP_RANGE, &diprg.diph);

	diprg.diph.dwObj        = DIJOFS_SLIDER(0);
	pdev->SetProperty(DIPROP_RANGE, &diprg.diph);

	diprg.diph.dwObj        = DIJOFS_SLIDER(1);
	pdev->SetProperty(DIPROP_RANGE, &diprg.diph);

	::Print("Created joystick %d (pdev = %08x)\n", select, (DWORD) pdev);
	idev = select;
	return true;
}

void
Joystick::EnumerateDevices()
{
	if (!pdi) {
		Print("Joystick: no DI7, unable to enumerate devices\n");
		ndev = 0;
	}

	else if (ndev < 1) {
		Print("Joystick: preparing to enumerate devices\n");

		ndev = 0;
		HRESULT hr = 
		pdi->EnumDevices(DIDEVTYPE_JOYSTICK,
		EnumJoystick,
		(LPVOID) 0, 
		DIEDFL_ATTACHEDONLY);

		if (FAILED(hr)) {
			DirectInputError("Failed to enumerate devices", hr);
			ReleaseDirectInput();
		}

		else if (ndev < 1) {
			Print("Joystick: no devices found\n");
			ReleaseDirectInput();
		}
	}
}

int
Joystick::NumDevices()
{
	return ndev;
}

const char*
Joystick::GetDeviceName(int i)
{
	if (i >= 0 && i < ndev)
	return devices[i].tszInstanceName;

	return 0;
}

// +--------------------------------------------------------------------+

static DIJOYSTATE joystate;
static JOYINFOEX  joyinfo;

int
Joystick::ReadRawAxis(int a)
{
	if (!joystick)
	return 0;

	int result = 0;

	if (pdev) {
		switch (a) {
		case KEY_JOY_AXIS_X:    result = joystate.lX;            break;
		case KEY_JOY_AXIS_Y:    result = joystate.lY;            break;
		case KEY_JOY_AXIS_Z:    result = joystate.lZ;            break;
		case KEY_JOY_AXIS_RX:   result = joystate.lRx;           break;
		case KEY_JOY_AXIS_RY:   result = joystate.lRy;           break;
		case KEY_JOY_AXIS_RZ:   result = joystate.lRz;           break;
		case KEY_JOY_AXIS_S0:   result = joystate.rglSlider[0];  break;
		case KEY_JOY_AXIS_S1:   result = joystate.rglSlider[1];  break;
		}
	}

	else {
		switch (a) {
		case KEY_JOY_AXIS_X:
			if (joyinfo.dwFlags & JOY_RETURNX)
			result = joyinfo.dwXpos;
			break;

		case KEY_JOY_AXIS_Y:
			if (joyinfo.dwFlags & JOY_RETURNY)
			result = joyinfo.dwYpos;
			break;

		case KEY_JOY_AXIS_Z:
			if (joyinfo.dwFlags & JOY_RETURNZ)
			result = joyinfo.dwZpos;
			break;

		case KEY_JOY_AXIS_RZ:
			if (joyinfo.dwFlags & JOY_RETURNR)
			result = joyinfo.dwRpos;
			break;
		}
	}

	return result;
}

double
Joystick::ReadAxisDI(int a)
{
	if (a < 0 || a > 3)
	return 0;

	double result = 0;

	switch (map_axis[a]) {
	case KEY_JOY_AXIS_X:    result = joystate.lX;            break;
	case KEY_JOY_AXIS_Y:    result = joystate.lY;            break;
	case KEY_JOY_AXIS_Z:    result = joystate.lZ;            break;
	case KEY_JOY_AXIS_RX:   result = joystate.lRx;           break;
	case KEY_JOY_AXIS_RY:   result = joystate.lRy;           break;
	case KEY_JOY_AXIS_RZ:   result = joystate.lRz;           break;
	case KEY_JOY_AXIS_S0:   result = joystate.rglSlider[0];  break;
	case KEY_JOY_AXIS_S1:   result = joystate.rglSlider[1];  break;
	}

	if (a < 3) {
		// ignore small movements:
		if (result > dead_zone)       result -= dead_zone;
		else if (result < -dead_zone) result += dead_zone;
		else                          result = 0;

		double scale = 1.0 / (32768.0-dead_zone);

		if (result >= 0)
		result = sqr(result * scale);
		else
		result = sqr(result * scale) * -1.0;

		if (inv_axis[a])
		result = -result;
	}
	else {
		result = (result+32768.0) / 65536.0;

		if (inv_axis[a])
		result = 1 - result;
	}


	return result;
}

double
Joystick::ReadAxisMM(int a)
{
	if (a < 0 || a > 3)
	return 0;

	double result = 0;

	switch (map_axis[a]) {
	case KEY_JOY_AXIS_X:
		if (joyinfo.dwFlags & JOY_RETURNX)
		result = joyinfo.dwXpos - 32768;
		break;

	case KEY_JOY_AXIS_Y:
		if (joyinfo.dwFlags & JOY_RETURNY)
		result = joyinfo.dwYpos - 32768;
		break;

	case KEY_JOY_AXIS_Z:
		if (joyinfo.dwFlags & JOY_RETURNZ)
		result = joyinfo.dwZpos - 32768;
		break;

	case KEY_JOY_AXIS_RZ:
		if (joyinfo.dwFlags & JOY_RETURNR)
		result = joyinfo.dwRpos - 32768;
		break;
	}

	if (a < 3) {
		// ignore small movements:
		if (result > dead_zone)       result -= dead_zone;
		else if (result < -dead_zone) result += dead_zone;
		else                          result = 0;

		double scale = 1.0 / (32768.0-dead_zone);

		if (result >= 0)
		result = sqr(result * scale);
		else
		result = sqr(result * scale) * -1.0;

		if (inv_axis[a])
		result = -result;
	}
	else {
		result = (result+32768.0) / 65536.0;

		if (inv_axis[a])
		result = 1 - result;
	}

	return result;
}

// +--------------------------------------------------------------------+

void
Joystick::Acquire()
{
	t = x = y = z = p = r = w = 0;
	for (int i = 0; i < MotionController::MaxActions; i++)
	action[i] = false;

	for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	hat[i][j] = false;

	if (select == 0)
	return;

	//============================================================
	// 
	// FIRST TRY DIRECT INPUT

	bool acquired = false;

	if (pdi) {
		if (idev != select) {
			if (ndev < 1)
			EnumerateDevices();

			if (CreateDevice(select))
			pdev->Acquire();
		}

		if (pdev) {
			HRESULT     hr = 0;

			hr = pdev->Poll();
			hr = pdev->GetDeviceState(sizeof(joystate), &joystate);

			if (hr == DIERR_INPUTLOST) {
				pdev->Acquire();

				hr = pdev->Poll();
				hr = pdev->GetDeviceState(sizeof(joystate), &joystate);

				if (FAILED(hr)) {
					strikes--;
					::Print("Joystick could not re-acquire joystick (%08x)\n", hr);

					// give up before you hurt yourself:
					if (strikes <= 0) {
						ReleaseDirectInput();
						ndev   = 0;
						select = 0;
					}

					return;
				}
			}

			for (int i = 0; i < 32; i++)
			action[i] = (joystate.rgbButtons[i] & 0x80) != 0;

			double joy_x = ReadAxisDI(0);
			double joy_y = ReadAxisDI(1);
			double joy_r = rudder   ? ReadAxisDI(2) : 0;
			double joy_t = throttle ? ReadAxisDI(3) : 0;

			int joy_p = joystate.rgdwPOV[0];

			ProcessAxes(joy_x, joy_y, joy_r, joy_t);

			for (int i = 0; i < 4; i++)
			ProcessHat(i, joystate.rgdwPOV[i]);

			acquired = true;
		}
	}

	//============================================================
	// 
	// THEN TRY WINDOWS MULTIMEDIA LIBRARY

	if (!acquired) {
		memset(&joyinfo, 0, sizeof(JOYINFOEX));
		joyinfo.dwSize  = sizeof(JOYINFOEX);
		joyinfo.dwFlags = JOY_RETURNALL;

		HRESULT hr = 0;

		if (select == 1)
		hr = joyGetPosEx(JOYSTICKID1, &joyinfo);

		else if (select == 2)
		hr = joyGetPosEx(JOYSTICKID2, &joyinfo);

		if (hr != 0) {
			Print("\nJoystick::Acquire() joyGetPosEx %d failed (err=%08x)\n\n", select, hr);
			select = 0;
		}

		action[0] = (joyinfo.dwButtons & JOY_BUTTON1) ? true : false;
		action[1] = (joyinfo.dwButtons & JOY_BUTTON2) ? true : false;
		action[2] = (joyinfo.dwButtons & JOY_BUTTON3) ? true : false;
		action[3] = (joyinfo.dwButtons & JOY_BUTTON4) ? true : false;

		double joy_x = ReadAxisMM(0);
		double joy_y = ReadAxisMM(1);
		double joy_r = rudder   ? ReadAxisMM(2) : 0;
		double joy_t = throttle ? ReadAxisMM(3) : 0;

		ProcessAxes(joy_x, joy_y, joy_r, joy_t);
		ProcessHat(0, joyinfo.dwPOV);
	}

	// lateral translations:
	if (KeyDownMap(KEY_PLUS_Y))            y =  1;
	else if (KeyDownMap(KEY_MINUS_Y))      y = -1;

	if (KeyDownMap(KEY_PLUS_Z))            z =  1;
	else if (KeyDownMap(KEY_MINUS_Z))      z = -1;

	if (KeyDownMap(KEY_MINUS_X))           x = -1;
	else if (KeyDownMap(KEY_PLUS_X))       x =  1;

	// button-based turns:
	const  double  steps=10;
	static double  p1=0, r1=0, w1=0;

	// if roll and yaw are swapped --------------------------
	if (swapped) {
		// yaw:
		if (KeyDownMap(KEY_ROLL_LEFT))         { if (w1<steps) w1+=1; w = -sqr(w1/steps); }
		else if (KeyDownMap(KEY_ROLL_RIGHT))   { if (w1<steps) w1+=1; w =  sqr(w1/steps); }

		// roll:
		if (KeyDownMap(KEY_YAW_LEFT))          { if (r1<steps) r1+=1; r =  sqr(r1/steps); }
		else if (KeyDownMap(KEY_YAW_RIGHT))    { if (r1<steps) r1+=1; r = -sqr(r1/steps); }
		else w1 = 0;
	}

	// else roll and yaw are NOT swapped ---------------------
	else {
		// roll:
		if (KeyDownMap(KEY_ROLL_LEFT))         { if (r1<steps) r1+=1; r =  sqr(r1/steps); }
		else if (KeyDownMap(KEY_ROLL_RIGHT))   { if (r1<steps) r1+=1; r = -sqr(r1/steps); }

		// yaw left-right
		if (KeyDownMap(KEY_YAW_LEFT))          { if (w1<steps) w1+=1; w = -sqr(w1/steps); }
		else if (KeyDownMap(KEY_YAW_RIGHT))    { if (w1<steps) w1+=1; w =  sqr(w1/steps); }
		else w1 = 0;
	}

	// pitch --------------------------------------------------
	if (KeyDownMap(KEY_PITCH_UP))             { if (p1<steps) p1+=1; p = -sqr(p1/steps); }
	else if (KeyDownMap(KEY_PITCH_DOWN))      { if (p1<steps) p1+=1; p =  sqr(p1/steps); }
	else p1 = 0;
}

// +--------------------------------------------------------------------+

void
Joystick::ProcessAxes(double joy_x, double joy_y, double joy_r, double joy_t)
{
	int roll_enable = 0;

	joy_y *= -1;
	joy_t  = 1 - joy_t;

	if (map[KEY_ROLL_ENABLE])
	roll_enable = action[map[KEY_ROLL_ENABLE] - KEY_JOY_1];

	// if roll and yaw are swapped --------------------------
	if (swapped) {
		if (roll_enable) {
			w =  joy_x;
			r =  joy_r;
		}
		else {
			w =  joy_r;
			r = -joy_x;
		}
	}

	// else roll and yaw are NOT swapped ---------------------
	else {
		if (roll_enable) {
			w =  joy_r;
			r =  joy_x;
		}
		else {
			w =  joy_x;
			r = -joy_r;
		}
	}

	p = joy_y;

	// read throttle:
	if (throttle) {
		static double init_throttle = -1;
		static bool   latch_throttle = false;

		if (init_throttle < 0)
		init_throttle = joy_t;
		else if (init_throttle != joy_t)
		latch_throttle = true;

		if (latch_throttle)
		t = joy_t;
		else
		t = 0;
	}
	else {
		t = 0;
	}
}

void
Joystick::ProcessHat(int i, DWORD joy_pov)
{
	if (i < 0 || i > 3) return;

	if (LOWORD(joy_pov) == 0xFFFF)
	return;

	switch (joy_pov) {
	case JOY_POVFORWARD:    hat[i][0] = true; break;
	case JOY_POVBACKWARD:   hat[i][1] = true; break;
	case JOY_POVLEFT:       hat[i][2] = true; break;
	case JOY_POVRIGHT:      hat[i][3] = true; break;

	case JOY_POVUPRIGHT:    hat[i][0] = true;
		hat[i][3] = true; break;

	case JOY_POVDNRIGHT:    hat[i][1] = true;
		hat[i][3] = true; break;

	case JOY_POVDNLEFT:     hat[i][1] = true;
		hat[i][2] = true; break;

	case JOY_POVUPLEFT:     hat[i][0] = true;
		hat[i][2] = true; break;

	default:                                  break;
	}
}

// +--------------------------------------------------------------------+

bool Joystick::KeyDown(int key)
{
	if (!joystick)
	return false;
	
	if (key >= KEY_JOY_1 && key <= KEY_JOY_32)
	return joystick->action[key - KEY_JOY_1];

	else if (key >= KEY_POV_0_UP && key <= KEY_POV_0_RIGHT)
	return joystick->hat[0][key - KEY_POV_0_UP];

	else if (key >= KEY_POV_1_UP && key <= KEY_POV_1_RIGHT)
	return joystick->hat[1][key - KEY_POV_1_UP];

	else if (key >= KEY_POV_2_UP && key <= KEY_POV_2_RIGHT)
	return joystick->hat[2][key - KEY_POV_2_UP];

	else if (key >= KEY_POV_3_UP && key <= KEY_POV_3_RIGHT)
	return joystick->hat[3][key - KEY_POV_3_UP];

	return false;
}

// +--------------------------------------------------------------------+

bool Joystick::KeyDownMap(int key)
{
	if (!joystick)
	return false;

	if (key >= KEY_MAP_FIRST && key <= KEY_MAP_LAST && joystick->map[key])
	return KeyDown(joystick->map[key]);

	return false;
}

// +--------------------------------------------------------------------+

int Joystick::GetAxisMap(int n)
{
	if (!joystick || n < 0 || n > 3)
	return -1;

	return joystick->map_axis[n];
}

int Joystick::GetAxisInv(int n)
{
	if (!joystick || n < 0 || n > 3)
	return -1;

	return joystick->inv_axis[n];
}

// +--------------------------------------------------------------------+

void
DirectInputError(const char* msg, HRESULT err)
{
	static int report = 50;
	if (report > 0)
	report--;
	else
	return;

	Print("   DirectInput7: %s. [%s]\n", msg, DIErrStr(err));
}

static char errstrbuf[128];

char* DIErrStr(HRESULT hr)
{
	switch (hr) {
	default:
		sprintf_s(errstrbuf, "Unrecognized error value = %08x.", hr);
		return errstrbuf;

	case DI_OK:
		return "No error.";

	case DI_BUFFEROVERFLOW:
		return "The device buffer overflowed and some input was lost. This value is equal to the S_FALSE standard COM return value.";
	case DI_DOWNLOADSKIPPED:
		return "The parameters of the effect were successfully updated, but the effect could not be downloaded because the associated device was not acquired in exclusive mode.";
	case DI_EFFECTRESTARTED: 
		return "The effect was stopped, the parameters were updated, and the effect was restarted.";
	case DI_POLLEDDEVICE:
		return "The device is a polled device. As a result, device buffering does not collect any data and event notifications is not signaled until the IDirectInputDevice7::Poll method is called.";
	case DI_TRUNCATED:
		return "The parameters of the effect were successfully updated, but some of them were beyond the capabilities of the device and were truncated to the nearest supported value.";
	case DI_TRUNCATEDANDRESTARTED:
		return "Equal to DI_EFFECTRESTARTED | DI_TRUNCATED";
	case DIERR_ACQUIRED:
		return "The operation cannot be performed while the device is acquired.";
	case DIERR_ALREADYINITIALIZED:
		return "This object is already initialized";
	case DIERR_BADDRIVERVER:
		return "The object could not be created due to an incompatible driver version or mismatched or incomplete driver components.";
	case DIERR_BETADIRECTINPUTVERSION:
		return "The application was written for an unsupported prerelease version of DirectInput.";
	case DIERR_DEVICEFULL:
		return "The device is full.";
	case DIERR_DEVICENOTREG:
		return "The device or device instance is not registered with DirectInput. This value is equal to the REGDB_E_CLASSNOTREG standard COM return value.";
	case DIERR_EFFECTPLAYING:
		return "The parameters were updated in memory but were not downloaded to the device because the device does not support updating an effect while it is still playing.";
	case DIERR_HASEFFECTS:
		return "The device cannot be reinitialized because there are still effects attached to it.";
	case DIERR_GENERIC:
		return "An undetermined error occurred inside the DirectInput subsystem. This value is equal to the E_FAIL standard COM return value.";
	case DIERR_HANDLEEXISTS:
		return "The device already has an event notification associated with it. This value is equal to the E_ACCESSDENIED standard COM return value.";
	case DIERR_INCOMPLETEEFFECT:
		return "The effect could not be downloaded because essential information is missing. For example, no axes have been associated with the effect, or no type-specific information has been supplied.";
	case DIERR_INPUTLOST:
		return "Access to the input device has been lost. It must be reacquired.";
	case DIERR_INVALIDPARAM:
		return "An invalid parameter was passed to the returning function, or the object was not in a state that permitted the function to be called. This value is equal to the E_INVALIDARG standard COM return value.";
	case DIERR_MOREDATA:
		return "Not all the requested information fit into the buffer.";
	case DIERR_NOAGGREGATION:
		return "This object does not support aggregation.";
	case DIERR_NOINTERFACE:
		return "The specified interface is not supported by the object. This value is equal to the E_NOINTERFACE standard COM return value.";
	case DIERR_NOTACQUIRED:
		return "The operation cannot be performed unless the device is acquired.";
	case DIERR_NOTBUFFERED:
		return "The device is not buffered. Set the DIPROP_BUFFERSIZE property to enable buffering.";
	case DIERR_NOTDOWNLOADED:
		return "The effect is not downloaded.";
	case DIERR_NOTEXCLUSIVEACQUIRED:
		return "The operation cannot be performed unless the device is acquired in DISCL_EXCLUSIVE mode.";
	case DIERR_NOTFOUND:
		return "The requested object does not exist.";
	case DIERR_NOTINITIALIZED:
		return "This object has not been initialized.";
	case DIERR_OLDDIRECTINPUTVERSION:
		return "The application requires a newer version of DirectInput.";
	case DIERR_OUTOFMEMORY:
		return "The DirectInput subsystem could not allocate sufficient memory to complete the call. This value is equal to the E_OUTOFMEMORY standard COM return value.";
	case DIERR_REPORTFULL:
		return "More information was requested to be sent than can be sent to the device.";
	case DIERR_UNPLUGGED:
		return "The operation could not be completed because the device is not plugged in.";
	case DIERR_UNSUPPORTED:
		return "The function called is not supported at this time. This value is equal to the E_NOTIMPL standard COM return value.";
	}
}


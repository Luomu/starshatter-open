/*  Project Starshatter 4.6
	Destroyer Studios LLC
	Copyright © 1997-2006. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         KeyMap.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Keyboard Mapping
*/

#ifndef KeyMap_h
#define KeyMap_h

#include "Types.h"
#include "Geometry.h"
#include "MotionController.h"

// +--------------------------------------------------------------------+

class KeyMap
{
public:
	KeyMap();
	virtual ~KeyMap();

	int DefaultKeyMap(int max_keys = 256);
	int LoadKeyMap(const char* filename, int max_keys = 256);
	int SaveKeyMap(const char* filename, int max_keys = 256);

	enum KEY_CATEGORY { KEY_FLIGHT, KEY_WEAPONS, KEY_VIEW, KEY_MISC };

	int                     GetCategory(int i);
	const char*             DescribeAction(int i);
	const char*             DescribeKey(int i);
	int                     FindMapIndex(int act);

	static const char*      DescribeKey(int vk, int shift, int joy);
	static int              GetMappableVKey(int n);

	int                     GetNumKeys()      { return nkeys;      }
	KeyMapEntry*            GetMapping()      { return map;        }
	KeyMapEntry*            GetKeyMap(int i)  { return &map[i];    }
	KeyMapEntry*            GetDefault(int i) { return &defmap[i]; }

	void                    Bind(int a, int k, int s);

	static int              GetKeyAction(const char* act_str);
	static int              GetKeyActionIndex(int act);
	static int              GetKeyKey(const char* key_str);
	static int              GetKeyKeyIndex(int key);

protected:
	int                     BuildDefaultKeyMap();

	KeyMapEntry             map[256];
	KeyMapEntry             defmap[256];
	int                     nkeys;
};

// +--------------------------------------------------------------------+


const int KEY_EXIT                  =   0 + KEY_USER_BASE;
const int KEY_PAUSE                 =   1 + KEY_USER_BASE;
const int KEY_NEXT_VIEW             =   2 + KEY_USER_BASE;
const int KEY_TARGET_PADLOCK        =   3 + KEY_USER_BASE;
const int KEY_THREAT_PADLOCK        =   4 + KEY_USER_BASE;
const int KEY_LOCK_TARGET           =   5 + KEY_USER_BASE;
const int KEY_LOCK_THREAT           =   6 + KEY_USER_BASE;
const int KEY_AUTO_NAV              =   7 + KEY_USER_BASE;
const int KEY_TIME_COMPRESS         =   8 + KEY_USER_BASE;
const int KEY_TIME_EXPAND           =   9 + KEY_USER_BASE;
const int KEY_TIME_SKIP             =  10 + KEY_USER_BASE;

const int KEY_SWAP_ROLL_YAW         =  11 + KEY_USER_BASE;

const int KEY_THROTTLE_UP           =  15 + KEY_USER_BASE;
const int KEY_THROTTLE_DOWN         =  16 + KEY_USER_BASE;
const int KEY_THROTTLE_ZERO         =  17 + KEY_USER_BASE;
const int KEY_THROTTLE_FULL         =  18 + KEY_USER_BASE;
const int KEY_CYCLE_PRIMARY         =  19 + KEY_USER_BASE;
const int KEY_CYCLE_SECONDARY       =  20 + KEY_USER_BASE;
const int KEY_FLCS_MODE_AUTO        =  21 + KEY_USER_BASE;
const int KEY_DROP_ORBIT            =  22 + KEY_USER_BASE;

const int KEY_HUD_INST              =  23 + KEY_USER_BASE;
const int KEY_CAM_BRIDGE            =  24 + KEY_USER_BASE;
const int KEY_CAM_CHASE             =  25 + KEY_USER_BASE;
const int KEY_CAM_EXTERN            =  26 + KEY_USER_BASE;
const int KEY_HUD_MODE              =  27 + KEY_USER_BASE;
const int KEY_HUD_COLOR             =  28 + KEY_USER_BASE;
const int KEY_HUD_WARN              =  29 + KEY_USER_BASE;
const int KEY_NAV_DLG               =  30 + KEY_USER_BASE;
const int KEY_WEP_DLG               =  31 + KEY_USER_BASE;
const int KEY_FLT_DLG               =  32 + KEY_USER_BASE;
const int KEY_ENG_DLG               =  33 + KEY_USER_BASE;

const int KEY_ZOOM_WIDE             =  34 + KEY_USER_BASE;
const int KEY_ZOOM_IN               =  35 + KEY_USER_BASE;
const int KEY_ZOOM_OUT              =  36 + KEY_USER_BASE;
const int KEY_CAM_CYCLE_OBJECT      =  37 + KEY_USER_BASE;
const int KEY_CAM_EXT_PLUS_AZ       =  38 + KEY_USER_BASE;
const int KEY_CAM_EXT_MINUS_AZ      =  39 + KEY_USER_BASE;
const int KEY_CAM_EXT_PLUS_EL       =  40 + KEY_USER_BASE;
const int KEY_CAM_EXT_MINUS_EL      =  41 + KEY_USER_BASE;
const int KEY_CAM_EXT_PLUS_RANGE    =  42 + KEY_USER_BASE;
const int KEY_CAM_EXT_MINUS_RANGE   =  43 + KEY_USER_BASE;
const int KEY_CAM_VIEW_SELECTION    =  44 + KEY_USER_BASE;
const int KEY_CAM_DROP              =  45 + KEY_USER_BASE;

const int KEY_TARGET_SELECTION      =  50 + KEY_USER_BASE;
const int KEY_RADIO_MENU            =  51 + KEY_USER_BASE;
const int KEY_QUANTUM_MENU          =  52 + KEY_USER_BASE;
const int KEY_MFD1                  =  53 + KEY_USER_BASE;
const int KEY_MFD2                  =  54 + KEY_USER_BASE;
const int KEY_MFD3                  =  55 + KEY_USER_BASE;
const int KEY_MFD4                  =  56 + KEY_USER_BASE;

const int KEY_SENSOR_MODE           =  60 + KEY_USER_BASE;
const int KEY_SENSOR_GROUND_MODE    =  61 + KEY_USER_BASE;
const int KEY_SENSOR_BEAM           =  62 + KEY_USER_BASE;
const int KEY_SENSOR_RANGE_PLUS     =  63 + KEY_USER_BASE;
const int KEY_SENSOR_RANGE_MINUS    =  64 + KEY_USER_BASE;
const int KEY_EMCON_PLUS            =  65 + KEY_USER_BASE;
const int KEY_EMCON_MINUS           =  66 + KEY_USER_BASE;

const int KEY_SHIELDS_UP            =  67 + KEY_USER_BASE;
const int KEY_SHIELDS_DOWN          =  68 + KEY_USER_BASE;
const int KEY_SHIELDS_FULL          =  69 + KEY_USER_BASE;
const int KEY_SHIELDS_ZERO          =  70 + KEY_USER_BASE;
const int KEY_DECOY                 =  71 + KEY_USER_BASE;
const int KEY_ECM_TOGGLE            =  72 + KEY_USER_BASE;
const int KEY_LAUNCH_PROBE          =  73 + KEY_USER_BASE;
const int KEY_GEAR_TOGGLE           =  74 + KEY_USER_BASE;

const int KEY_LOCK_CLOSEST_SHIP     =  75 + KEY_USER_BASE;
const int KEY_LOCK_CLOSEST_THREAT   =  76 + KEY_USER_BASE;
const int KEY_LOCK_HOSTILE_SHIP     =  77 + KEY_USER_BASE;
const int KEY_LOCK_HOSTILE_THREAT   =  78 + KEY_USER_BASE;
const int KEY_CYCLE_SUBTARGET       =  79 + KEY_USER_BASE;
const int KEY_PREV_SUBTARGET        =  80 + KEY_USER_BASE;

const int KEY_AUGMENTER             =  81 + KEY_USER_BASE;
const int KEY_NAVLIGHT_TOGGLE       =  82 + KEY_USER_BASE;

const int KEY_CAM_VIRT              =  85 + KEY_USER_BASE;
const int KEY_CAM_VIRT_PLUS_AZ      =  86 + KEY_USER_BASE;
const int KEY_CAM_VIRT_MINUS_AZ     =  87 + KEY_USER_BASE;
const int KEY_CAM_VIRT_PLUS_EL      =  88 + KEY_USER_BASE;
const int KEY_CAM_VIRT_MINUS_EL     =  89 + KEY_USER_BASE;

const int KEY_COMM_ATTACK_TGT       =  90 + KEY_USER_BASE;
const int KEY_COMM_ESCORT_TGT       =  91 + KEY_USER_BASE;
const int KEY_COMM_WEP_FREE         =  92 + KEY_USER_BASE;
const int KEY_COMM_WEP_HOLD         =  93 + KEY_USER_BASE;
const int KEY_COMM_COVER_ME         =  94 + KEY_USER_BASE;
const int KEY_COMM_SKIP_NAV         =  95 + KEY_USER_BASE;
const int KEY_COMM_RETURN_TO_BASE   =  96 + KEY_USER_BASE;
const int KEY_COMM_CALL_INBOUND     =  97 + KEY_USER_BASE;
const int KEY_COMM_REQUEST_PICTURE  =  98 + KEY_USER_BASE;
const int KEY_COMM_REQUEST_SUPPORT  =  99 + KEY_USER_BASE;

const int KEY_CHAT_BROADCAST        = 100 + KEY_USER_BASE;
const int KEY_CHAT_TEAM             = 101 + KEY_USER_BASE;
const int KEY_CHAT_WING             = 102 + KEY_USER_BASE;
const int KEY_CHAT_UNIT             = 103 + KEY_USER_BASE;

const int KEY_COMMAND_MODE          = 104 + KEY_USER_BASE;
const int KEY_SELF_DESTRUCT         = 105 + KEY_USER_BASE;

/*** For Debug Convenience Only: ***/
const int KEY_INC_STARDATE          = 120 + KEY_USER_BASE;
const int KEY_DEC_STARDATE          = 121 + KEY_USER_BASE;
/***/

#endif KeyMap_h


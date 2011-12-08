/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         KeyMap.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Weapon class
*/

#include "MemDebug.h"
#include "Keyboard.h"
#include "KeyMap.h"
#include "Game.h"
#include "Ship.h"

// +----------------------------------------------------------------------+

struct KeyName
{
   int         category;
   int         key;
   const char* name;
   const char* desc;
};

static KeyName key_action_table[] = {
   { KeyMap::KEY_FLIGHT,  KEY_PLUS_X,          "KEY_PLUS_X",        "Translate Right"       },
   { KeyMap::KEY_FLIGHT,  KEY_MINUS_X,         "KEY_MINUS_X",       "Translate Left"        },
   { KeyMap::KEY_FLIGHT,  KEY_PLUS_Y,          "KEY_PLUS_Y",        "Translate Forward"     },
   { KeyMap::KEY_FLIGHT,  KEY_MINUS_Y,         "KEY_MINUS_Y",       "Translate Aft"         },
   { KeyMap::KEY_FLIGHT,  KEY_PLUS_Z,          "KEY_PLUS_Z",        "Translate Up"          },
   { KeyMap::KEY_FLIGHT,  KEY_MINUS_Z,         "KEY_MINUS_Z",       "Translate Down"        },

   { KeyMap::KEY_FLIGHT,  KEY_PITCH_UP,        "KEY_PITCH_UP",      "Pitch Up"              },
   { KeyMap::KEY_FLIGHT,  KEY_PITCH_DOWN,      "KEY_PITCH_DOWN",    "Pitch Down"            },
   { KeyMap::KEY_FLIGHT,  KEY_YAW_LEFT,        "KEY_YAW_LEFT",      "Yaw Left"              },
   { KeyMap::KEY_FLIGHT,  KEY_YAW_RIGHT,       "KEY_YAW_RIGHT",     "Yaw Right"             },
   { KeyMap::KEY_FLIGHT,  KEY_ROLL_LEFT,       "KEY_ROLL_LEFT",     "Roll Left"             },
   { KeyMap::KEY_FLIGHT,  KEY_ROLL_RIGHT,      "KEY_ROLL_RIGHT",    "Roll Right"            },
   { KeyMap::KEY_FLIGHT,  KEY_CENTER,          "KEY_CENTER",        "Center"                },
   { KeyMap::KEY_FLIGHT,  KEY_ROLL_ENABLE,     "KEY_ROLL_ENABLE",   "Roll Enable"           }, 
   { KeyMap::KEY_FLIGHT,  KEY_SWAP_ROLL_YAW,   "KEY_SWAP_ROLL_YAW", "Swap Ctrl Axes"        }, 

   { KeyMap::KEY_WEAPONS, KEY_ACTION_0,        "KEY_ACTION_0",      "Fire Primary"   },
   { KeyMap::KEY_WEAPONS, KEY_ACTION_1,        "KEY_ACTION_1",      "Fire Secondary" },
   { KeyMap::KEY_WEAPONS, KEY_ACTION_2,        "KEY_ACTION_2",      "Action 2"       },
   { KeyMap::KEY_WEAPONS, KEY_ACTION_3,        "KEY_ACTION_3",      "Action 3"       },

   { KeyMap::KEY_FLIGHT,  KEY_CONTROL_MODEL,   "KEY_CONTROL_MODEL"  },

   { KeyMap::KEY_FLIGHT,  KEY_MOUSE_SELECT,    "KEY_MOUSE_SELECT"   },
   { KeyMap::KEY_FLIGHT,  KEY_MOUSE_SENSE,     "KEY_MOUSE_SENSE"    },
   { KeyMap::KEY_FLIGHT,  KEY_MOUSE_SWAP,      "KEY_MOUSE_SWAP"     },
   { KeyMap::KEY_FLIGHT,  KEY_MOUSE_INVERT,    "KEY_MOUSE_INVERT"   },
   { KeyMap::KEY_FLIGHT,  KEY_MOUSE_ACTIVE,    "KEY_MOUSE_ACTIVE",  "Toggle Mouse Ctrl"     },

   { KeyMap::KEY_FLIGHT,  KEY_JOY_SELECT,      "KEY_JOY_SELECT"     },
   { KeyMap::KEY_FLIGHT,  KEY_JOY_RUDDER,      "KEY_JOY_RUDDER"     },
   { KeyMap::KEY_FLIGHT,  KEY_JOY_THROTTLE,    "KEY_JOY_THROTTLE"   },
   { KeyMap::KEY_FLIGHT,  KEY_JOY_SENSE,       "KEY_JOY_SENSE"      },
   { KeyMap::KEY_FLIGHT,  KEY_JOY_DEAD_ZONE,   "KEY_JOY_DEAD_ZONE"  },
   { KeyMap::KEY_FLIGHT,  KEY_JOY_SWAP,        "KEY_JOY_SWAP"       },

   { KeyMap::KEY_FLIGHT,  KEY_AXIS_YAW,               "KEY_AXIS_YAW"             },
   { KeyMap::KEY_FLIGHT,  KEY_AXIS_PITCH,             "KEY_AXIS_PITCH"           },
   { KeyMap::KEY_FLIGHT,  KEY_AXIS_ROLL,              "KEY_AXIS_ROLL"            },
   { KeyMap::KEY_FLIGHT,  KEY_AXIS_THROTTLE,          "KEY_AXIS_THROTTLE"        },
   { KeyMap::KEY_FLIGHT,  KEY_AXIS_YAW_INVERT,        "KEY_AXIS_YAW_INVERT"      },
   { KeyMap::KEY_FLIGHT,  KEY_AXIS_PITCH_INVERT,      "KEY_AXIS_PITCH_INVERT"    },
   { KeyMap::KEY_FLIGHT,  KEY_AXIS_ROLL_INVERT,       "KEY_AXIS_ROLL_INVERT"     },
   { KeyMap::KEY_FLIGHT,  KEY_AXIS_THROTTLE_INVERT,   "KEY_AXIS_THROTTLE_INVERT" },

   { KeyMap::KEY_MISC,    KEY_EXIT,            "KEY_EXIT",          "Exit"           },
   { KeyMap::KEY_MISC,    KEY_PAUSE,           "KEY_PAUSE",         "Pause"          },
// { KeyMap::KEY_VIEW,    KEY_NEXT_VIEW,       "KEY_NEXT_VIEW",     "Next View"      },
   { KeyMap::KEY_VIEW,    KEY_TARGET_PADLOCK,  "KEY_TARGET_PADLOCK","Padlock Target" },
   { KeyMap::KEY_VIEW,    KEY_THREAT_PADLOCK,  "KEY_THREAT_PADLOCK","Padlock Threat" },
   { KeyMap::KEY_WEAPONS, KEY_LOCK_TARGET,     "KEY_LOCK_TARGET",   "Lock Target"    },
   { KeyMap::KEY_WEAPONS, KEY_LOCK_THREAT,     "KEY_LOCK_THREAT",   "Lock Threat"    },
   { KeyMap::KEY_WEAPONS, KEY_LOCK_CLOSEST_SHIP,   "KEY_LOCK_CLOSEST_SHIP",     "Lock Closest Ship"    },
   { KeyMap::KEY_WEAPONS, KEY_LOCK_CLOSEST_THREAT, "KEY_LOCK_CLOSEST_THREAT",   "Lock Closest Threat"  },
   { KeyMap::KEY_WEAPONS, KEY_LOCK_HOSTILE_SHIP,   "KEY_LOCK_HOSTILE_SHIP",     "Lock Hostile Ship"    },
   { KeyMap::KEY_WEAPONS, KEY_LOCK_HOSTILE_THREAT, "KEY_LOCK_HOSTILE_THREAT",   "Lock Hostile Threat"  },
   { KeyMap::KEY_WEAPONS, KEY_CYCLE_SUBTARGET, "KEY_CYCLE_SUBTARGET",   "Target Subsystem"  },
   { KeyMap::KEY_WEAPONS, KEY_PREV_SUBTARGET,  "KEY_PREV_SUBTARGET",    "Previous Subsystem"  },
   { KeyMap::KEY_FLIGHT,  KEY_AUTO_NAV,        "KEY_AUTO_NAV",      "Autonav"        },
   { KeyMap::KEY_MISC,    KEY_TIME_COMPRESS,   "KEY_TIME_COMPRESS", "Compress Time"  },
   { KeyMap::KEY_MISC,    KEY_TIME_EXPAND,     "KEY_TIME_EXPAND",   "Expand Time"    },
   { KeyMap::KEY_MISC,    KEY_TIME_SKIP,       "KEY_TIME_SKIP",     "Skip to Next Event" },

   { KeyMap::KEY_FLIGHT,  KEY_THROTTLE_UP,     "KEY_THROTTLE_UP",   "Throttle Up"    },
   { KeyMap::KEY_FLIGHT,  KEY_THROTTLE_DOWN,   "KEY_THROTTLE_DOWN", "Throttle Down"  },
   { KeyMap::KEY_FLIGHT,  KEY_THROTTLE_ZERO,   "KEY_THROTTLE_ZERO", "All Stop"       },
   { KeyMap::KEY_FLIGHT,  KEY_THROTTLE_FULL,   "KEY_THROTTLE_FULL", "Full Throttle"  },
   { KeyMap::KEY_FLIGHT,  KEY_AUGMENTER,       "KEY_AUGMENTER",     "Augmenter"      },
   { KeyMap::KEY_FLIGHT,  KEY_FLCS_MODE_AUTO,  "KEY_FLCS_MODE_AUTO","FLCS Mode Toggle"  },
   { KeyMap::KEY_FLIGHT,  KEY_COMMAND_MODE,    "KEY_COMMAND_MODE",  "CMD Mode Toggle"   },
   { KeyMap::KEY_FLIGHT,  KEY_DROP_ORBIT,      "KEY_DROP_ORBIT",    "Break Orbit"    },
   { KeyMap::KEY_FLIGHT,  KEY_GEAR_TOGGLE,     "KEY_GEAR_TOGGLE",   "Landing Gear"   },
   { KeyMap::KEY_FLIGHT,  KEY_NAVLIGHT_TOGGLE, "KEY_NAVLIGHT_TOGGLE","Nav Lights"    },

   { KeyMap::KEY_WEAPONS, KEY_CYCLE_PRIMARY,   "KEY_CYCLE_PRIMARY", "Cycle Primary"  },
   { KeyMap::KEY_WEAPONS, KEY_CYCLE_SECONDARY, "KEY_CYCLE_SECONDARY", "Cycle Secondary" },

   { KeyMap::KEY_VIEW,    KEY_HUD_INST,        "KEY_HUD_INST",      "Instr. Display" },
   { KeyMap::KEY_VIEW,    KEY_CAM_BRIDGE,      "KEY_CAM_BRIDGE",    "Bridge Cam"     },
   { KeyMap::KEY_VIEW,    KEY_CAM_VIRT,        "KEY_CAM_VIRT",      "Virtual Cockpit" },
   { KeyMap::KEY_VIEW,    KEY_CAM_CHASE,       "KEY_CAM_CHASE",     "Chase Cam"      },
   { KeyMap::KEY_VIEW,    KEY_CAM_DROP,        "KEY_CAM_DROP",      "Drop Cam"       },
   { KeyMap::KEY_VIEW,    KEY_CAM_EXTERN,      "KEY_CAM_EXTERN",    "Orbit Cam"      },
   { KeyMap::KEY_MISC,    KEY_HUD_MODE,        "KEY_HUD_MODE",      "HUD Mode"       },
   { KeyMap::KEY_MISC,    KEY_HUD_COLOR,       "KEY_HUD_COLOR",     "HUD Color"      },
   { KeyMap::KEY_MISC,    KEY_HUD_WARN,        "KEY_HUD_WARN",      "Master Caution" },
   { KeyMap::KEY_MISC,    KEY_NAV_DLG,         "KEY_NAV_DLG",       "NAV Window"     },
   { KeyMap::KEY_MISC,    KEY_WEP_DLG,         "KEY_WEP_DLG",       "TAC Overlay"    },
   { KeyMap::KEY_MISC,    KEY_FLT_DLG,         "KEY_FLT_DLG",       "FLT Window"     },
   { KeyMap::KEY_MISC,    KEY_ENG_DLG,         "KEY_ENG_DLG",       "ENG Window"     },

   { KeyMap::KEY_VIEW,    KEY_ZOOM_WIDE,       "KEY_ZOOM_WIDE",     "Toggle Wide Angle"      },
   { KeyMap::KEY_VIEW,    KEY_ZOOM_IN,         "KEY_ZOOM_IN",       "Zoom In"        },
   { KeyMap::KEY_VIEW,    KEY_ZOOM_OUT,        "KEY_ZOOM_OUT",      "Zoom Out"       },

   { KeyMap::KEY_VIEW,    KEY_CAM_VIRT_PLUS_AZ,   "KEY_CAM_VIRT_PLUS_AZ",       "Look Left"        },
   { KeyMap::KEY_VIEW,    KEY_CAM_VIRT_MINUS_AZ,  "KEY_CAM_VIRT_MINUS_AZ",      "Look Right"       },
   { KeyMap::KEY_VIEW,    KEY_CAM_VIRT_PLUS_EL,   "KEY_CAM_VIRT_PLUS_EL",       "Look Up"          },
   { KeyMap::KEY_VIEW,    KEY_CAM_VIRT_MINUS_EL,  "KEY_CAM_VIRT_MINUS_EL",      "Look Down"        },
   { KeyMap::KEY_VIEW,    KEY_CAM_CYCLE_OBJECT,   "KEY_CAM_CYCLE_OBJECT",       "View Next Object" },
   { KeyMap::KEY_VIEW,    KEY_CAM_EXT_PLUS_AZ,    "KEY_CAM_EXT_PLUS_AZ",        "View Spin Left"   },
   { KeyMap::KEY_VIEW,    KEY_CAM_EXT_MINUS_AZ,   "KEY_CAM_EXT_MINUS_AZ",       "View Spin Right"  },
   { KeyMap::KEY_VIEW,    KEY_CAM_EXT_PLUS_EL,    "KEY_CAM_EXT_PLUS_EL",        "View Raise"       },
   { KeyMap::KEY_VIEW,    KEY_CAM_EXT_MINUS_EL,   "KEY_CAM_EXT_MINUS_EL",       "View Lower"       },
   { KeyMap::KEY_VIEW,    KEY_CAM_EXT_PLUS_RANGE, "KEY_CAM_EXT_PLUS_RANGE",     "View Farther"     },
   { KeyMap::KEY_VIEW,    KEY_CAM_EXT_MINUS_RANGE,"KEY_CAM_EXT_MINUS_RANGE",    "View Closer"      },
   { KeyMap::KEY_VIEW,    KEY_CAM_VIEW_SELECTION, "KEY_CAM_VIEW_SELECTION",     "View Selection"   },

   { KeyMap::KEY_WEAPONS, KEY_TARGET_SELECTION,   "KEY_TARGET_SELECTION",       "Target Selection" },
   { KeyMap::KEY_MISC,    KEY_RADIO_MENU,         "KEY_RADIO_MENU",             "Radio Call"       },
   { KeyMap::KEY_MISC,    KEY_QUANTUM_MENU,       "KEY_QUANTUM_MENU",           "Quantum Drive"    },

   { KeyMap::KEY_MISC,    KEY_MFD1,               "KEY_MFD1", "MFD 1"                 },
   { KeyMap::KEY_MISC,    KEY_MFD2,               "KEY_MFD2", "MFD 2"                 },
   { KeyMap::KEY_MISC,    KEY_MFD3,               "KEY_MFD3", "MFD 3"                 },
   { KeyMap::KEY_MISC,    KEY_MFD4,               "KEY_MFD4", "MFD 4"                 },
   { KeyMap::KEY_MISC,    KEY_SELF_DESTRUCT,      "KEY_SELF_DESTRUCT",       "Self Destruct"                 },

   { KeyMap::KEY_MISC,    KEY_COMM_ATTACK_TGT,    "KEY_COMM_ATTACK_TGT",     "'Attack Tgt'"       },
   { KeyMap::KEY_MISC,    KEY_COMM_ESCORT_TGT,    "KEY_COMM_ESCORT_TGT",     "'Escort Tgt'"       },
   { KeyMap::KEY_MISC,    KEY_COMM_WEP_FREE,      "KEY_COMM_WEP_FREE",       "'Break & Attack'"   },
   { KeyMap::KEY_MISC,    KEY_COMM_WEP_HOLD,      "KEY_COMM_WEP_HOLD",       "'Form Up'"          },
   { KeyMap::KEY_MISC,    KEY_COMM_COVER_ME,      "KEY_COMM_COVER_ME",       "'Help Me Out!'"     },
   { KeyMap::KEY_MISC,    KEY_COMM_SKIP_NAV,      "KEY_COMM_SKIP_NAV",       "'Skip Navpoint'"    },
   { KeyMap::KEY_MISC,    KEY_COMM_RETURN_TO_BASE,"KEY_COMM_RETURN_TO_BASE", "'Return to Base'"   },
   { KeyMap::KEY_MISC,    KEY_COMM_CALL_INBOUND,  "KEY_COMM_CALL_INBOUND",   "'Call Inbound'"     },
   { KeyMap::KEY_MISC,    KEY_COMM_REQUEST_PICTURE, "KEY_COMM_REQUEST_PICTURE", "'Request Picture'"     },
   { KeyMap::KEY_MISC,    KEY_COMM_REQUEST_SUPPORT, "KEY_COMM_REQUEST_SUPPORT", "'Request Support'"     },
   { KeyMap::KEY_MISC,    KEY_CHAT_BROADCAST,     "KEY_CHAT_BROADCAST",      "Chat Broadcast" },
   { KeyMap::KEY_MISC,    KEY_CHAT_TEAM,          "KEY_CHAT_TEAM",           "Chat Team" },
   { KeyMap::KEY_MISC,    KEY_CHAT_WING,          "KEY_CHAT_WING",           "Chat Wingman" },
   { KeyMap::KEY_MISC,    KEY_CHAT_UNIT,          "KEY_CHAT_UNIT",           "Chat Unit" },

   { KeyMap::KEY_WEAPONS, KEY_SHIELDS_UP,         "KEY_SHIELDS_UP",          "Raise Shields" },
   { KeyMap::KEY_WEAPONS, KEY_SHIELDS_DOWN,       "KEY_SHIELDS_DOWN",        "Lower Shields" },
   { KeyMap::KEY_WEAPONS, KEY_SHIELDS_FULL,       "KEY_SHIELDS_FULL",        "Full Shields" },
   { KeyMap::KEY_WEAPONS, KEY_SHIELDS_ZERO,       "KEY_SHIELDS_ZERO",        "Zero Shields" },

   { KeyMap::KEY_WEAPONS, KEY_SENSOR_MODE,        "KEY_SENSOR_MODE",         "Sensor Mode"  },
   { KeyMap::KEY_WEAPONS, KEY_SENSOR_GROUND_MODE, "KEY_SENSOR_GROUND_MODE",  "Sensor AGM"   },
   { KeyMap::KEY_WEAPONS, KEY_SENSOR_BEAM,        "KEY_SENSOR_BEAM",         "Sensor Sweep Angle" },
   { KeyMap::KEY_WEAPONS, KEY_SENSOR_RANGE_PLUS,  "KEY_SENSOR_RANGE_PLUS",   "Inc Sensor Range" },
   { KeyMap::KEY_WEAPONS, KEY_SENSOR_RANGE_MINUS, "KEY_SENSOR_RANGE_MINUS",  "Dec Sensor Range" },
   { KeyMap::KEY_WEAPONS, KEY_EMCON_PLUS,         "KEY_EMCON_PLUS",          "Inc EMCON Level" },
   { KeyMap::KEY_WEAPONS, KEY_EMCON_MINUS,        "KEY_EMCON_MINUS",         "Dec EMCON Level" },

   { KeyMap::KEY_WEAPONS, KEY_DECOY,              "KEY_DECOY",               "Launch Decoy" },
   { KeyMap::KEY_WEAPONS, KEY_LAUNCH_PROBE,       "KEY_LAUNCH_PROBE",        "Launch Probe" },

};

static KeyName key_key_table[] = {
   { 0, VK_ESCAPE,            "VK_ESCAPE",      "Escape"       },
   { 0, VK_UP,                "VK_UP",          "Up"           },
   { 0, VK_DOWN,              "VK_DOWN",        "Down"         },
   { 0, VK_LEFT,              "VK_LEFT",        "Left"         },
   { 0, VK_RIGHT,             "VK_RIGHT",       "Right"        },
   { 0, VK_NEXT,              "VK_NEXT",        "Pg Down"      },
   { 0, VK_PRIOR,             "VK_PRIOR",       "Pg Up"        },
   { 0, VK_ADD,               "VK_ADD",         "+"            },
   { 0, VK_SUBTRACT,          "VK_SUBTRACT",    "-"            },
   { 0, VK_MULTIPLY,          "VK_MULTIPLY",    "*"            },
   { 0, VK_DIVIDE,            "VK_DIVIDE",      "/"            },
   { 0, VK_SPACE,             "VK_SPACE",       "Space"        },
   { 0, VK_TAB,               "VK_TAB",         "Tab"          },
   { 0, VK_RETURN,            "VK_RETURN",      "Enter"        },
   { 0, VK_HOME,              "VK_HOME",        "Home"         },
   { 0, VK_END,               "VK_END",         "End"          },
   { 0, VK_SHIFT,             "VK_SHIFT",       "Shift"        },
   { 0, VK_CONTROL,           "VK_CONTROL",     "Ctrl"         },
   { 0, VK_MENU,              "VK_MENU",        "Alt"          },

   { 0, VK_DECIMAL,           "VK_DECIMAL",     "."            },
   { 0, VK_SEPARATOR,         "VK_SEPARATOR",   "Separator"    },
   { 0, VK_PAUSE,             "VK_PAUSE",       "Pause"        },
   { 0, VK_BACK,              "VK_BACK",        "Backspace"    },
   { 0, VK_INSERT,            "VK_INSERT",      "Insert"       },
   { 0, VK_DELETE,            "VK_DELETE",      "Delete"       },
   { 0, 20,                   "CAP",            "CapsLock"     },
   { 0, 144,                  "NUM",            "NumLock"      },
   { 0, 145,                  "SCROLL",         "Scroll"       },
   { 0, 44,                   "PRINT",          "PrintScr"     },
         
   { 0, VK_NUMPAD0,           "VK_NUMPAD0",     "Num 0"        },
   { 0, VK_NUMPAD1,           "VK_NUMPAD1",     "Num 1"        },
   { 0, VK_NUMPAD2,           "VK_NUMPAD2",     "Num 2"        },
   { 0, VK_NUMPAD3,           "VK_NUMPAD3",     "Num 3"        },
   { 0, VK_NUMPAD4,           "VK_NUMPAD4",     "Num 4"        },
   { 0, VK_NUMPAD5,           "VK_NUMPAD5",     "Num 5"        },
   { 0, VK_NUMPAD6,           "VK_NUMPAD6",     "Num 6"        },
   { 0, VK_NUMPAD7,           "VK_NUMPAD7",     "Num 7"        },
   { 0, VK_NUMPAD8,           "VK_NUMPAD8",     "Num 8"        },
   { 0, VK_NUMPAD9,           "VK_NUMPAD9",     "Num 9"        },

   { 0, VK_F1,                "VK_F1",          "F1"           },
   { 0, VK_F2,                "VK_F2",          "F2"           },
   { 0, VK_F3,                "VK_F3",          "F3"           },
   { 0, VK_F4,                "VK_F4",          "F4"           },
   { 0, VK_F5,                "VK_F5",          "F5"           },
   { 0, VK_F6,                "VK_F6",          "F6"           },
   { 0, VK_F7,                "VK_F7",          "F7"           },
   { 0, VK_F8,                "VK_F8",          "F8"           },
   { 0, VK_F9,                "VK_F9",          "F9"           },
   { 0, VK_F10,               "VK_F10",         "F10"          },
   { 0, VK_F11,               "VK_F11",         "F11"          },
   { 0, VK_F12,               "VK_F12",         "F12"          },

   { 0, KEY_JOY_1,            "KEY_JOY_1",      "Joy 1"        },
   { 0, KEY_JOY_2,            "KEY_JOY_2",      "Joy 2"        },
   { 0, KEY_JOY_3,            "KEY_JOY_3",      "Joy 3"        },
   { 0, KEY_JOY_4,            "KEY_JOY_4",      "Joy 4"        },
   { 0, KEY_JOY_5,            "KEY_JOY_5",      "Joy 5"        },
   { 0, KEY_JOY_6,            "KEY_JOY_6",      "Joy 6"        },
   { 0, KEY_JOY_7,            "KEY_JOY_7",      "Joy 7"        },
   { 0, KEY_JOY_8,            "KEY_JOY_8",      "Joy 8"        },
   { 0, KEY_JOY_9,            "KEY_JOY_9",      "Joy 9"        },
   { 0, KEY_JOY_10,           "KEY_JOY_10",     "Joy 10"       },
   { 0, KEY_JOY_11,           "KEY_JOY_11",     "Joy 11"       },
   { 0, KEY_JOY_12,           "KEY_JOY_12",     "Joy 12"       },
   { 0, KEY_JOY_13,           "KEY_JOY_13",     "Joy 13"       },
   { 0, KEY_JOY_14,           "KEY_JOY_14",     "Joy 14"       },
   { 0, KEY_JOY_15,           "KEY_JOY_15",     "Joy 15"       },
   { 0, KEY_JOY_16,           "KEY_JOY_16",     "Joy 16"       },

   { 0, KEY_POV_0_UP,         "KEY_POV_0_UP",      "Hat 1 Up"     },
   { 0, KEY_POV_0_DOWN,       "KEY_POV_0_DOWN",    "Hat 1 Down"   },
   { 0, KEY_POV_0_LEFT,       "KEY_POV_0_LEFT",    "Hat 1 Left"   },
   { 0, KEY_POV_0_RIGHT,      "KEY_POV_0_RIGHT",   "Hat 1 Right"  },
   { 0, KEY_POV_1_UP,         "KEY_POV_1_UP",      "Hat 2 Up"     },
   { 0, KEY_POV_1_DOWN,       "KEY_POV_1_DOWN",    "Hat 2 Down"   },
   { 0, KEY_POV_1_LEFT,       "KEY_POV_1_LEFT",    "Hat 2 Left"   },
   { 0, KEY_POV_1_RIGHT,      "KEY_POV_1_RIGHT",   "Hat 2 Right"  },
   { 0, KEY_POV_2_UP,         "KEY_POV_2_UP",      "Hat 3 Up"     },
   { 0, KEY_POV_2_DOWN,       "KEY_POV_2_DOWN",    "Hat 3 Down"   },
   { 0, KEY_POV_2_LEFT,       "KEY_POV_2_LEFT",    "Hat 3 Left"   },
   { 0, KEY_POV_2_RIGHT,      "KEY_POV_2_RIGHT",   "Hat 3 Right"  },
   { 0, KEY_POV_3_UP,         "KEY_POV_3_UP",      "Hat 4 Up"     },
   { 0, KEY_POV_3_DOWN,       "KEY_POV_3_DOWN",    "Hat 4 Down"   },
   { 0, KEY_POV_3_LEFT,       "KEY_POV_3_LEFT",    "Hat 4 Left"   },
   { 0, KEY_POV_3_RIGHT,      "KEY_POV_3_RIGHT",   "Hat 4 Right"  },

   { 0, 186,                  ";",   ";" },
   { 0, 188,                  ",",   "<" },
   { 0, 190,                  ".",   ">" },
   { 0, 191,                  "/",   "/" },
   { 0, 192,                  "~",   "~" },
   { 0, 219,                  "[",   "[" },
   { 0, 220,                  "\\",  "\\" },
   { 0, 221,                  "]",   "]" },
   { 0, 222,                  "'",   "'" },

   { 0, '0',                  "0",   "0" },
   { 0, '1',                  "1",   "1" },
   { 0, '2',                  "2",   "2" },
   { 0, '3',                  "3",   "3" },
   { 0, '4',                  "4",   "4" },
   { 0, '5',                  "5",   "5" },
   { 0, '6',                  "6",   "6" },
   { 0, '7',                  "7",   "7" },
   { 0, '8',                  "8",   "8" },
   { 0, '9',                  "9",   "9" },

   { 0, 'A',                  "A",   "A" },
   { 0, 'B',                  "B",   "B" },
   { 0, 'C',                  "C",   "C" },
   { 0, 'D',                  "D",   "D" },
   { 0, 'E',                  "E",   "E" },
   { 0, 'F',                  "F",   "F" },
   { 0, 'G',                  "G",   "G" },
   { 0, 'H',                  "H",   "H" },
   { 0, 'I',                  "I",   "I" },
   { 0, 'J',                  "J",   "J" },
   { 0, 'K',                  "K",   "K" },
   { 0, 'L',                  "L",   "L" },
   { 0, 'M',                  "M",   "M" },
   { 0, 'N',                  "N",   "N" },
   { 0, 'O',                  "O",   "O" },
   { 0, 'P',                  "P",   "P" },
   { 0, 'Q',                  "Q",   "Q" },
   { 0, 'R',                  "R",   "R" },
   { 0, 'S',                  "S",   "S" },
   { 0, 'T',                  "T",   "T" },
   { 0, 'U',                  "U",   "U" },
   { 0, 'V',                  "V",   "V" },
   { 0, 'W',                  "W",   "W" },
   { 0, 'X',                  "X",   "X" },
   { 0, 'Y',                  "Y",   "Y" },
   { 0, 'Z',                  "Z",   "Z" },

};

// +----------------------------------------------------------------------+

int KeyMap::GetKeyAction(const char* act_str)
{
   if (!act_str) return -1;

   int nactions = sizeof(key_action_table) / sizeof(KeyName);

   for (int i = 0; i < nactions; i++)
      if (!stricmp(act_str, key_action_table[i].name))
         return key_action_table[i].key;

   return -1;
}

// +----------------------------------------------------------------------+

int KeyMap::GetKeyActionIndex(int act)
{
   int nactions = sizeof(key_action_table) / sizeof(KeyName);

   for (int i = 0; i < nactions; i++)
      if (key_action_table[i].key == act)
         return i;

   return -1;
}

// +----------------------------------------------------------------------+

int KeyMap::GetKeyKey(const char* key_str)
{
   if (!key_str) return 0;

   if (*key_str == '=') {
      int value = 0;
      sscanf(key_str, "=%d", &value);
      return value;
   }

   int nkeys = sizeof(key_key_table) / sizeof(KeyName);

   for (int i = 0; i < nkeys; i++)
      if (!stricmp(key_str, key_key_table[i].name))
         return key_key_table[i].key;

   return 0;
}

// +----------------------------------------------------------------------+

int KeyMap::GetKeyKeyIndex(int key)
{
   int nkeys = sizeof(key_key_table) / sizeof(KeyName);

   for (int i = 0; i < nkeys; i++)
      if (key_key_table[i].key == key)
         return i;

   return -1;
}

// +----------------------------------------------------------------------+


KeyMap::KeyMap()
 : nkeys(0)
{
   int n = BuildDefaultKeyMap();
   DefaultKeyMap(n);
}

KeyMap::~KeyMap()
{ }

// +----------------------------------------------------------------------+

int
KeyMap::BuildDefaultKeyMap()
{
   int i = 0;

   defmap[i++] = KeyMapEntry(KEY_PITCH_UP,            VK_DOWN);
   defmap[i++] = KeyMapEntry(KEY_PITCH_DOWN,          VK_UP);
   defmap[i++] = KeyMapEntry(KEY_YAW_LEFT,            VK_LEFT);
   defmap[i++] = KeyMapEntry(KEY_YAW_RIGHT,           VK_RIGHT);
   defmap[i++] = KeyMapEntry(KEY_ROLL_LEFT,           VK_NUMPAD7);
   defmap[i++] = KeyMapEntry(KEY_ROLL_RIGHT,          VK_NUMPAD9);

   defmap[i++] = KeyMapEntry(KEY_PLUS_X,              190,           0,         KEY_POV_0_RIGHT);    // .
   defmap[i++] = KeyMapEntry(KEY_MINUS_X,             188,           0,         KEY_POV_0_LEFT);     // ,
   defmap[i++] = KeyMapEntry(KEY_PLUS_Y,              VK_HOME);
   defmap[i++] = KeyMapEntry(KEY_MINUS_Y,             VK_END);
   defmap[i++] = KeyMapEntry(KEY_PLUS_Z,              VK_PRIOR,      0,         KEY_POV_0_UP);
   defmap[i++] = KeyMapEntry(KEY_MINUS_Z,             VK_NEXT,       0,         KEY_POV_0_DOWN);

   defmap[i++] = KeyMapEntry(KEY_ACTION_0,            VK_CONTROL,    0,         KEY_JOY_1);
   defmap[i++] = KeyMapEntry(KEY_ACTION_1,            VK_SPACE,      0,         KEY_JOY_2);

   
   defmap[i++] = KeyMapEntry(KEY_THROTTLE_UP,         'A');
   defmap[i++] = KeyMapEntry(KEY_THROTTLE_DOWN,       'Z');
   defmap[i++] = KeyMapEntry(KEY_THROTTLE_FULL,       'A',           VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_THROTTLE_ZERO,       'Z',           VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_AUGMENTER,           VK_TAB);
   defmap[i++] = KeyMapEntry(KEY_FLCS_MODE_AUTO,      'M');
   defmap[i++] = KeyMapEntry(KEY_COMMAND_MODE,        'M',           VK_SHIFT);

   defmap[i++] = KeyMapEntry(KEY_CYCLE_PRIMARY,       VK_BACK,       VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_CYCLE_SECONDARY,     VK_BACK);
   defmap[i++] = KeyMapEntry(KEY_LOCK_TARGET,         'T',           0,         KEY_JOY_3);
   defmap[i++] = KeyMapEntry(KEY_LOCK_THREAT,         'T',           VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_LOCK_CLOSEST_SHIP,   'U');
   defmap[i++] = KeyMapEntry(KEY_LOCK_CLOSEST_THREAT, 'U',           VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_LOCK_HOSTILE_SHIP,   'Y');
   defmap[i++] = KeyMapEntry(KEY_LOCK_HOSTILE_THREAT, 'Y',           VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_CYCLE_SUBTARGET,     186);
   defmap[i++] = KeyMapEntry(KEY_PREV_SUBTARGET,      186,           VK_SHIFT);

   defmap[i++] = KeyMapEntry(KEY_DECOY,               'D',           0,         KEY_JOY_4);
   defmap[i++] = KeyMapEntry(KEY_GEAR_TOGGLE,         'G');
   defmap[i++] = KeyMapEntry(KEY_NAVLIGHT_TOGGLE,     'L');

   defmap[i++] = KeyMapEntry(KEY_AUTO_NAV,            'N',           VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_DROP_ORBIT,          'O');

   defmap[i++] = KeyMapEntry(KEY_SHIELDS_UP,          'S');
   defmap[i++] = KeyMapEntry(KEY_SHIELDS_DOWN,        'X');
   defmap[i++] = KeyMapEntry(KEY_SHIELDS_FULL,        'S',           VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_SHIELDS_ZERO,        'X',           VK_SHIFT);

   defmap[i++] = KeyMapEntry(KEY_SENSOR_MODE,         VK_F5);
   defmap[i++] = KeyMapEntry(KEY_SENSOR_GROUND_MODE,  VK_F5,         VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_LAUNCH_PROBE,        VK_F6);
   defmap[i++] = KeyMapEntry(KEY_SENSOR_RANGE_MINUS,  VK_F7);
   defmap[i++] = KeyMapEntry(KEY_SENSOR_RANGE_PLUS,   VK_F8);
   defmap[i++] = KeyMapEntry(KEY_EMCON_MINUS,         VK_F9);
   defmap[i++] = KeyMapEntry(KEY_EMCON_PLUS,          VK_F10);

   defmap[i++] = KeyMapEntry(KEY_EXIT,                VK_ESCAPE);
   defmap[i++] = KeyMapEntry(KEY_PAUSE,               VK_PAUSE);
// defmap[i++] = KeyMapEntry(KEY_NEXT_VIEW,           VK_TAB);
   defmap[i++] = KeyMapEntry(KEY_TIME_EXPAND,         VK_DELETE);
   defmap[i++] = KeyMapEntry(KEY_TIME_COMPRESS,       VK_INSERT);
   defmap[i++] = KeyMapEntry(KEY_TIME_SKIP,           VK_INSERT,     VK_SHIFT);

   defmap[i++] = KeyMapEntry(KEY_CAM_BRIDGE,          VK_F1);
   defmap[i++] = KeyMapEntry(KEY_CAM_VIRT,            VK_F1,         VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_CAM_CHASE,           VK_F2);
   defmap[i++] = KeyMapEntry(KEY_CAM_DROP,            VK_F2,         VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_CAM_EXTERN,          VK_F3);
   defmap[i++] = KeyMapEntry(KEY_TARGET_PADLOCK,      VK_F4);

   defmap[i++] = KeyMapEntry(KEY_ZOOM_WIDE,           'K');
   defmap[i++] = KeyMapEntry(KEY_HUD_MODE,            'H');
   defmap[i++] = KeyMapEntry(KEY_HUD_COLOR,           'H',           VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_HUD_WARN,            'C');
   defmap[i++] = KeyMapEntry(KEY_HUD_INST,            'I');
   defmap[i++] = KeyMapEntry(KEY_NAV_DLG,             'N');
   defmap[i++] = KeyMapEntry(KEY_WEP_DLG,             'W');
   defmap[i++] = KeyMapEntry(KEY_ENG_DLG,             'E');
   defmap[i++] = KeyMapEntry(KEY_FLT_DLG,             'F');
   defmap[i++] = KeyMapEntry(KEY_RADIO_MENU,          'R');
   defmap[i++] = KeyMapEntry(KEY_QUANTUM_MENU,        'Q');

   defmap[i++] = KeyMapEntry(KEY_MFD1,                219);             // [
   defmap[i++] = KeyMapEntry(KEY_MFD2,                221);             // ]
   defmap[i++] = KeyMapEntry(KEY_SELF_DESTRUCT,       VK_ESCAPE,     VK_SHIFT);
   
   defmap[i++] = KeyMapEntry(KEY_CAM_CYCLE_OBJECT,    VK_TAB,        VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_CAM_EXT_PLUS_AZ,     VK_LEFT,       VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_CAM_EXT_MINUS_AZ,    VK_RIGHT,      VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_CAM_EXT_PLUS_EL,     VK_UP,         VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_CAM_EXT_MINUS_EL,    VK_DOWN,       VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_CAM_EXT_PLUS_RANGE,  VK_SUBTRACT);
   defmap[i++] = KeyMapEntry(KEY_CAM_EXT_MINUS_RANGE, VK_ADD);
   defmap[i++] = KeyMapEntry(KEY_CAM_VIEW_SELECTION,  'V');
   defmap[i++] = KeyMapEntry(KEY_CAM_VIRT_PLUS_AZ,    VK_LEFT,       VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_CAM_VIRT_MINUS_AZ,   VK_RIGHT,      VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_CAM_VIRT_PLUS_EL,    VK_DOWN,       VK_SHIFT);
   defmap[i++] = KeyMapEntry(KEY_CAM_VIRT_MINUS_EL,   VK_UP,         VK_SHIFT);

   defmap[i++] = KeyMapEntry(KEY_SWAP_ROLL_YAW,       'J');

   defmap[i++] = KeyMapEntry(KEY_COMM_ATTACK_TGT,     'A',           VK_MENU);
   defmap[i++] = KeyMapEntry(KEY_COMM_ESCORT_TGT,     'E',           VK_MENU);
   defmap[i++] = KeyMapEntry(KEY_COMM_WEP_FREE,       'B',           VK_MENU);
   defmap[i++] = KeyMapEntry(KEY_COMM_WEP_HOLD,       'F',           VK_MENU);
   defmap[i++] = KeyMapEntry(KEY_COMM_COVER_ME,       'H',           VK_MENU);
   defmap[i++] = KeyMapEntry(KEY_COMM_SKIP_NAV,       'N',           VK_MENU);
   defmap[i++] = KeyMapEntry(KEY_COMM_RETURN_TO_BASE, 'R',           VK_MENU);
   defmap[i++] = KeyMapEntry(KEY_COMM_CALL_INBOUND,   'I',           VK_MENU);
   defmap[i++] = KeyMapEntry(KEY_COMM_REQUEST_PICTURE,'P',           VK_MENU);
   defmap[i++] = KeyMapEntry(KEY_COMM_REQUEST_SUPPORT,'S',           VK_MENU);

   defmap[i++] = KeyMapEntry(KEY_CHAT_BROADCAST,      '1',           VK_MENU);
   defmap[i++] = KeyMapEntry(KEY_CHAT_TEAM,           '2',           VK_MENU);
   defmap[i++] = KeyMapEntry(KEY_CHAT_WING,           '3',           VK_MENU);
   defmap[i++] = KeyMapEntry(KEY_CHAT_UNIT,           '4',           VK_MENU);

   defmap[i++] = KeyMapEntry(KEY_CONTROL_MODEL,       0);

   defmap[i++] = KeyMapEntry(KEY_JOY_SELECT,          1);
   defmap[i++] = KeyMapEntry(KEY_JOY_RUDDER,          0);
   defmap[i++] = KeyMapEntry(KEY_JOY_THROTTLE,        1);
   defmap[i++] = KeyMapEntry(KEY_JOY_SENSE,           1);
   defmap[i++] = KeyMapEntry(KEY_JOY_DEAD_ZONE,       500);

   defmap[i++] = KeyMapEntry(KEY_MOUSE_SELECT,        1);
   defmap[i++] = KeyMapEntry(KEY_MOUSE_SENSE,         10);
   defmap[i++] = KeyMapEntry(KEY_MOUSE_ACTIVE,        192);          // ~ key

/*** For Debug Convenience Only: ***/
   defmap[i++] = KeyMapEntry(KEY_INC_STARDATE,        VK_F11);
   defmap[i++] = KeyMapEntry(KEY_DEC_STARDATE,        VK_F11,        VK_SHIFT);
/***/

   return i;
}

// +----------------------------------------------------------------------+

int
KeyMap::DefaultKeyMap(int max_keys)
{
   for (int i = 0; i < max_keys; i++)
      map[i] = defmap[i];

   nkeys = max_keys;
   return nkeys;
}

void
KeyMap::Bind(int a, int k, int s)
{
   if (!a) return;

   for (int i = 0; i < nkeys; i++) {
      if (map[i].act == a) {
         map[i].key = k;
         map[i].alt = s;

         return;
      }
   }

   map[nkeys++] = KeyMapEntry(a, k, s);
}

// +----------------------------------------------------------------------+

int 
KeyMap::LoadKeyMap(const char* filename, int max_keys)
{
   FILE* f = fopen(filename, "r");
   if (!f) return nkeys;
   
   char  line[256];

   while (fgets(line, sizeof(line), f)) {
      int   act = -1, key = -1, alt = 0, joy = -1;
      char  act_str[128], key_str[128], alt_str[128], joy_str[128];

      ZeroMemory(act_str, sizeof(act_str));
      ZeroMemory(key_str, sizeof(key_str));
      ZeroMemory(alt_str, sizeof(alt_str));
      ZeroMemory(joy_str, sizeof(joy_str));

      sscanf(line, "%s %s %s %s", act_str, key_str, alt_str, joy_str);

      act = GetKeyAction(act_str);
      key = GetKeyKey(key_str);
      alt = GetKeyKey(alt_str);
      joy = GetKeyKey(joy_str);

      if (act != -1 && key != -1) {
         if (act == KEY_CONTROL_MODEL)
            Ship::SetControlModel(key);

         int mapped = false;

         for (int i = 0; i < max_keys && !mapped; i++) {
            if (map[i].act == act) {
               Print("  Remapping: '%s' => %s(%d) %s(%d) %s(%d)\n",
                  act_str, key_str, key, alt_str, alt, joy_str, joy);

               map[i] = KeyMapEntry(act, key, alt, joy);
               mapped = true;
            }
         }

         if (!mapped) {
            Print("  Mapping: '%s' => %s(%d) %s(%d) %s(%d)\n",
               act_str, key_str, key, alt_str, alt, joy_str, joy);
            map[nkeys++] = KeyMapEntry(act, key, alt, joy);
         }
      }

      if (nkeys >= max_keys-1) {
         Print("   Too many keys in configuration...\n");
         break;
      }
   }
   
   fclose(f);
   return nkeys;
}

// +----------------------------------------------------------------------+

int 
KeyMap::SaveKeyMap(const char* filename, int max_keys)
{
   FILE* f = fopen(filename, "w");
   if (!f) return 0;
   
   for (int i = 0; i < nkeys; i++) {
      if (map[i].act >= KEY_CONTROL_MODEL &&
          map[i].act <= KEY_AXIS_THROTTLE_INVERT) {

         int a = GetKeyActionIndex(map[i].act);

         fprintf(f, "%-24s =%d\n", key_action_table[a].name, map[i].key);
      }

      else if (map[i] != defmap[i]) {
         int a = GetKeyActionIndex(map[i].act);
         int k = GetKeyKeyIndex(map[i].key);
         int s = GetKeyKeyIndex(map[i].alt);
         int j = GetKeyKeyIndex(map[i].joy);

         if (a > -1) {
            if (j > -1) {
               if (s > -1) {
                  fprintf(f, "%-24s %-16s %-16s %-16s\n", 
                     key_action_table[a].name,
                     key_key_table[   k].name,
                     key_key_table[   s].name,
                     key_key_table[   j].name);
               }
               else if (k > -1) {
                  fprintf(f, "%-24s %-16s %-16s %-16s\n", 
                     key_action_table[a].name,
                     key_key_table[   k].name,
                     "null",
                     key_key_table[   j].name);
               }
               else {
                  fprintf(f, "%-24s %-16s %-16s %-16s\n", 
                     key_action_table[a].name,
                     "null",
                     "null",
                     key_key_table[   j].name);
               }
            }
            else if (s > -1) {
               fprintf(f, "%-24s %-16s %-16s\n", 
                  key_action_table[a].name, 
                  key_key_table[   k].name, 
                  key_key_table[   s].name);
            }
            else if (k > -1) {
               fprintf(f, "%-24s %-16s\n", 
                  key_action_table[a].name, 
                  key_key_table[   k].name);
            }
         }
      }
   }

   fclose(f);
   return nkeys;
}

// +----------------------------------------------------------------------+

int 
KeyMap::GetCategory(int n)
{
   int nactions = sizeof(key_action_table) / sizeof(KeyName);

   for (int i = 0; i < nactions; i++) {
      if (map[n].act == key_action_table[i].key) {
         return key_action_table[i].category;
      }
   }

   return KEY_MISC;
}

// +----------------------------------------------------------------------+

int
KeyMap::FindMapIndex(int act)
{
   for (int n = 0; n < nkeys; n++)
      if (map[n].act == act)
         return n;
   return -1;
}

// +----------------------------------------------------------------------+

const char* 
KeyMap::DescribeAction(int n)
{
   int nactions = sizeof(key_action_table) / sizeof(KeyName);

   for (int i = 0; i < nactions; i++) {
      if (map[n].act == key_action_table[i].key) {
         if (key_action_table[i].desc)
            return key_action_table[i].desc;

         return key_action_table[i].name;
      }
   }

   return 0;
}

// +----------------------------------------------------------------------+

static char key_desc[32];

const char*
KeyMap::DescribeKey(int n)
{
   if (n >= 0 && n < 256)
      return DescribeKey(map[n].key, map[n].alt, map[n].joy);

   return 0;
}

const char*
KeyMap::DescribeKey(int vk, int shift, int j)
{
   const char* key = 0;
   const char* alt = 0;
   const char* joy = 0;

   int         nkeys = sizeof(key_key_table) / sizeof(KeyName);

   for (int i = 0; i < nkeys; i++) {
      if (vk > 0 && vk == key_key_table[i].key) {
         if (key_key_table[i].desc)
            key = key_key_table[i].desc;
         else
            key = key_key_table[i].name;
      }

      if (shift > 0 && shift == key_key_table[i].key) {
         if (key_key_table[i].desc)
            alt = key_key_table[i].desc;
         else
            alt = key_key_table[i].name;
      }

      if (j > 0 && j == key_key_table[i].key) {
         if (key_key_table[i].desc)
            joy = key_key_table[i].desc;
         else
            joy = key_key_table[i].name;
      }
   }

   if (key) {
      if (alt) {
         sprintf(key_desc, "%s+%s", alt, key);
      }
      else {
         strcpy(key_desc, key);
      }

      if (joy) {
         strcat(key_desc, ", ");
         strcat(key_desc, joy);
      }
   }

   else if (joy) {
      strcpy(key_desc, joy);
   }

   else {
      sprintf(key_desc, "%d", vk);
   }

   return key_desc;
}

int
KeyMap::GetMappableVKey(int n)
{
   int nkeys = sizeof(key_key_table) / sizeof(KeyName);

   if (n >= 0 && n < nkeys) {
      return key_key_table[n].key;
   }

   return 0;
}
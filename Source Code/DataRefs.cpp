#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPLMScenery.h"
#include "XPLMInstance.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMCamera.h"
#include "XPLMPlanes.h"


//Overrides
static XPLMDataRef override_planepath = XPLMFindDataRef("sim/operation/override/override_planepath");
static XPLMDataRef override_ai = XPLMFindDataRef("sim/operation/override/override_plane_ai_autopilot");
static XPLMDataRef override_joystick = XPLMFindDataRef("sim/operation/override/override_joystick");

static XPLMDataRef is_paused = XPLMFindDataRef("sim/time/paused");

//Location datarefs
static XPLMDataRef loc_x = XPLMFindDataRef("sim/flightmodel/position/local_x");
static XPLMDataRef loc_y = XPLMFindDataRef("sim/flightmodel/position/local_y");
static XPLMDataRef loc_z = XPLMFindDataRef("sim/flightmodel/position/local_z");
static XPLMDataRef heading = XPLMFindDataRef("sim/flightmodel/position/psi");
static XPLMDataRef pitch = XPLMFindDataRef("sim/flightmodel/position/theta");
static XPLMDataRef roll = XPLMFindDataRef("sim/flightmodel/position/phi");

static XPLMDataRef vel_x = XPLMFindDataRef("sim/flightmodel/position/local_vx");
static XPLMDataRef vel_y = XPLMFindDataRef("sim/flightmodel/position/local_vy");
static XPLMDataRef vel_z = XPLMFindDataRef("sim/flightmodel/position/local_vz");

//AI control
static XPLMDataRef ai_control = XPLMFindDataRef("sim/operation/prefs/ai_flies_aircraft");
static XPLMDataRef autopilot_enable = XPLMFindDataRef("sim/cockpit/autopilot/autopilot_mode");
static XPLMDataRef autopilot_altitude = XPLMFindDataRef("sim/cockpit/autopilot/altitude");
static XPLMDataRef autopilot_control = XPLMFindDataRef("sim/cockpit/autopilot/autopilot_state");

//Joystick datarefs
static XPLMDataRef joystick_values = XPLMFindDataRef("sim/joystick/joy_mapped_axis_value");
static XPLMDataRef joystick_roll = XPLMFindDataRef("sim/joystick/yoke_roll_ratio");
static XPLMDataRef joystick_pitch = XPLMFindDataRef("sim/joystick/yoke_pitch_ratio");
static XPLMDataRef joystick_raw = XPLMFindDataRef("sim/joystick/joystick_axis_values");
static XPLMDataRef trim_ail = XPLMFindDataRef("sim/flightmodel/controls/ail_trim");
static XPLMDataRef trim_elv = XPLMFindDataRef("sim/flightmodel/controls/elv_trim");

static XPLMDataRef fuel_amt = XPLMFindDataRef("sim/aircraft/weight/acf_m_fuel_tot");

static XPLMDataRef landing_gear = XPLMFindDataRef("sim/aircraft/parts/acf_gear_deploy");
static XPLMDataRef sim_time = XPLMFindDataRef("sim/time/zulu_time_sec");

static XPLMDataRef sim_volume = XPLMFindDataRef("sim/operation/sound/master_volume_ratio");
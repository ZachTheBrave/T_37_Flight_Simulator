
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include "DataRefs.cpp"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GL/gl.h>



DCB dcbSerialParams = { 0 };




// Flight loop setup
static float	MyFlightLoopCallback(
	float                inElapsedSinceLastCall,
	float                inElapsedTimeSinceLastFlightLoop,
	int                  inCounter,
	void* inRefcon);

//Objects
XPLMObjectRef sphere_object = XPLMLoadObject("Resources/plugins/T37_Resources/Sphere.obj");
XPLMObjectRef sphere_sm = XPLMLoadObject("Resources/plugins/T37_Resources/Sphere_small.obj");
XPLMObjectRef sphere_lg = XPLMLoadObject("Resources/plugins/T37_Resources/Sphere_large.obj");
const char* drefs[] = { NULL };
XPLMDrawInfo_t		dr;
struct sphere_data {
	XPLMInstanceRef instance;
	float x;
	float y;
	float z;
	bool active;
	int size;
};
sphere_data spheres[20];
//sphere_data spheres_small[4];

//Struct used by object handler
struct location {
	float x;
	float y;
	float z;
};



//Camera control
static int cameraControl(
	XPLMCameraPosition_t* outCameraPosition,
	int losingControl,
	void* refcon);

static int cameraControl2(
	XPLMCameraPosition_t* outCameraPosition,
	int losingControl,
	void* refcon);

//Window setup
//Don't use all of these but they need to be defined
static XPLMWindowID	g_window;
static XPLMWindowID g_window2;
void				draw(XPLMWindowID in_window_id, void* in_refcon);
int					dummy_mouse_handler(XPLMWindowID in_window_id, int x, int y, int is_down, void* in_refcon) { return 0; }
XPLMCursorStatus	dummy_cursor_status_handler(XPLMWindowID in_window_id, int x, int y, void* in_refcon) { return xplm_CursorDefault; }
int					dummy_wheel_handler(XPLMWindowID in_window_id, int x, int y, int wheel, int clicks, void* in_refcon) { return 0; }
void				dummy_key_handler(XPLMWindowID in_window_id, char key, XPLMKeyFlags flags, char virtual_key, void* in_refcon, int losing_focus) {}

//Leaderboard file
char* filePath = "C:/X-Plane 12/Resources/plugins/T37_Resources/Leaderboard.txt";

//Control variables
int setdata;
int game_state = -1;
float tracked_time = 0;
float current_time = 0;
float auto_x = 0;
bool crashed = false;
float START_ALT = 6000;
float START_VEL = 130;
int MAX_SPHERES = 20;
float set_volume = 0;

//Score variables
int score = 0;
struct total_scores {
	int total;
	int year;
	int month;
	int day;
};
total_scores leaderboard[10];
int daily_leaderboard[10];

//Drawing variables
int image_to_draw;
int num_to_draw;
bool draw_top_scores;
int TITLE_SCREEN_IMG;
int NUMBER_IMG;
int MONTH_IMG;
int SCORE_IMG;
int LEADERBOARD_IMG;
int TUTORIAL_IMG[4];
int CRASH_IMG;
int HUD_IMG;
int FIFTY_IMG;
int HUNDRED_IMG;
int TWO_IMG;
int draw_fifty = 0;
int draw_hundred = 0;
int draw_two = 0;
int buffer = 0;

//Light variables
HANDLE hSerial;
const char* light = "r";
DWORD bytesWritten;
bool light_changed = true;

static void setPosition();
static void startFlying(void);
bool checkUserInput(void);
bool checkGameEnd(void);
static void createObjects(void);
static void handleObjects(void);
static void destroyObjects(void);
static void AutoFly(int x);
static void updateLeaderboard(int score);
static void loadImages();
static location lookAhead(float t);
static void drawNums(int left, int bot, int num, int size);
static void drawDigits(int left, int bot, int num, int size);
static void drawMonth(int left, int bot, int month);




PLUGIN_API int XPluginStart(
	char* outName,
	char* outSig,
	char* outDesc)
{
	strcpy(outName, "T37_Plugin");
	strcpy(outSig, "TU.plugin");
	strcpy(outDesc, "The plugin for our simulator");

	
	//Flight loop
	XPLMRegisterFlightLoopCallback(
		MyFlightLoopCallback,	
		1.0,
		NULL);

	//Initialize window and load images
	loadImages();
	
	//Read leaderboard file
	std::ifstream inFile(filePath);
	for (int i = 0; i < 10; i++) {
		inFile >> leaderboard[i].total;
		inFile >> leaderboard[i].day;
		inFile >> leaderboard[i].month;
		inFile >> leaderboard[i].year;
	}
	inFile.close();

	
	//Light output
	hSerial = CreateFileA(
		"COM8",
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		0
	);

	if (hSerial == INVALID_HANDLE_VALUE) {
		image_to_draw = NUMBER_IMG;
		return 1;
	}

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;

	if (!GetCommState(hSerial, &dcbSerialParams)) {
		CloseHandle(hSerial);
		image_to_draw = MONTH_IMG;
		return 1;
	}

	

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{

	//Close serial
	light = "x";
	WriteFile(hSerial, light, strlen(light), &bytesWritten, NULL);
	CloseHandle(hSerial);

	//Write scores to file
	std::ofstream outFile(filePath);
	for (int i = 0; i < 10; i++) {
		outFile << leaderboard[i].total << std::endl;
		outFile << leaderboard[i].day << std::endl;
		outFile << leaderboard[i].month << std::endl;
		outFile << leaderboard[i].year << std::endl;
	}
	outFile.close();


	//End flight loop callback
	XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, NULL);

	//Unload Objects
	if (game_state == 2)
		destroyObjects;

	num_to_draw = -1;

	//Destroy window
	XPLMDestroyWindow(g_window);
	g_window = NULL;

	XPLMDestroyWindow(g_window2);
	g_window2 = NULL;

}

PLUGIN_API void XPluginDisable(void) {}
PLUGIN_API int  XPluginEnable(void) { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void* inParam) {}


//Flight loop callback controls the majority of the program, called every time the physics updates
float	MyFlightLoopCallback(
	float                inElapsedSinceLastCall,
	float                inElapsedTimeSinceLastFlightLoop,
	int                  inCounter,
	void* inRefcon)
{
	//Set landing gear
	//For some reason it kept opening
	float gear = 0;
	XPLMSetDatavf(landing_gear, &gear, 0, 1);
	XPLMSetDatavf(landing_gear, &gear, 1, 1);
	XPLMSetDatavf(landing_gear, &gear, 2, 1);

	//Get time
	current_time = XPLMGetElapsedTime();

	
	//Control lights
	if (light_changed == true) {
			WriteFile(hSerial, light, strlen(light), &bytesWritten, NULL);
		light_changed = false;
	}
	

	switch (game_state) {
	case -1:
		//Setup game
			startFlying();

			//With 2 monitors maybe use this
			//XPLMSetDataf(XPLMFindDataRef("sim/aircraft/view/acf_peX"), 0);
			//XPLMSetDataf(XPLMFindDataRef("sim/aircraft/view/acf_peY"), 0.95);
			//XPLMSetDataf(XPLMFindDataRef("sim/aircraft/view/acf_peZ"), -1);

			//XPLMSetDataf(XPLMFindDataRef("sim/aircraft/view/acf_peX"), -0.2);
			//XPLMSetDataf(XPLMFindDataRef("sim/aircraft/view/acf_peY"), 0.88);
			//XPLMSetDataf(XPLMFindDataRef("sim/aircraft/view/acf_peZ"), -0.85);

			XPLMSetDataf(XPLMFindDataRef("sim/aircraft/view/acf_peX"), 0.2);
			XPLMSetDataf(XPLMFindDataRef("sim/aircraft/view/acf_peY"), 0.88);
			XPLMSetDataf(XPLMFindDataRef("sim/aircraft/view/acf_peZ"), -0.9);

			//Daily leaderboard
			for (int i = 0; i < 10; i++) {
				daily_leaderboard[i] = 0;
			}

			XPLMSetWindowIsVisible(g_window, 1);
			XPLMSetWindowIsVisible(g_window2, 1);
			tracked_time = current_time - 400;
			game_state = 0;

		break;

	case 0:
		//Title Screen
		//Set plane to specified position, repeats every couple minutes
		if ((current_time - tracked_time) > 300) {
			setPosition();
			XPLMControlCamera(xplm_ControlCameraUntilViewChanges, cameraControl, NULL);
			XPLMSetDataf(fuel_amt, 4000);
			XPLMSetDataf(sim_time, 63200);
			XPLMSetDataf(sim_volume, 0);
			tracked_time = current_time;
			auto_x = 0;
			//Don't technically need to set these variables, but want to ensure that they have the correct value
			image_to_draw = TITLE_SCREEN_IMG;
			num_to_draw = -1;
			score = 0;
			draw_top_scores = false;
			crashed = false;
			light = "r";
			light_changed = true;
		}
		//Automatically move plane and camera
		AutoFly(auto_x);
		auto_x = auto_x + 1;

		//Check is user is moving the joystick
		if (checkUserInput()) {
			//Change to tutorial screen
			image_to_draw = 0;
			num_to_draw = -1;
			XPLMDontControlCamera();
			setPosition();
			createObjects();
			tracked_time = current_time;
			game_state = 1;
			light = "b";
			light_changed = true;
			set_volume = 0;
		}
		break;

	case 1:
		//Instructions
		// Ramp up volume
		if (set_volume < 200) {
			set_volume = set_volume + 1;
			XPLMSetDataf(sim_volume, set_volume / 200);
		}

		//Display Instructions and countdown
		if ((current_time - tracked_time) < 4) {
			image_to_draw = TUTORIAL_IMG[3];
		}
		else if ((current_time - tracked_time) < 8) {
			float whole, fraction;
			fraction = modf((current_time - tracked_time), &whole);
			int n = fraction * 3;
			image_to_draw = TUTORIAL_IMG[n];
		}
		else if ((current_time - tracked_time) < 9) {
			image_to_draw = 0;
			num_to_draw = 3;
		}
		else if ((current_time - tracked_time) < 10) {
			num_to_draw = 2;
		}
		else if ((current_time - tracked_time) < 11) {
			num_to_draw = 1;
		}
		else{
			//Start game
			num_to_draw = -1;
			image_to_draw = HUD_IMG;
			game_state = 2;
			setdata = 0;
			XPLMSetDatavi(override_planepath, &setdata, 0, 1);
			tracked_time = current_time;
		}
		break;

	case 2:
		//Player control
		handleObjects();

		//Track if the player is shown scoring icons
		if (draw_fifty > 0) {
			draw_fifty = draw_fifty - 1;
		}
		if (draw_hundred > 0) {
			draw_hundred = draw_hundred - 1;
		}
		if (draw_two > 0) {
			draw_two = draw_two - 1;
		}
		//Change light to blue if no spheres have been hit recently
		if (draw_fifty == 0 && draw_hundred == 0 && draw_two == 0 && light != "b") {
			light = "b";
			light_changed = true;
		}
		if (buffer > 0) {
			buffer = buffer - 1;
		}

		//Draw countdown
		if ((current_time - tracked_time) > 80) {
			num_to_draw = 90 - (current_time - tracked_time);
		}

		//Check if the 90 seconds are done
		if (checkGameEnd()) {
			game_state = 3;
			tracked_time = current_time;
			
			setdata = 1;
			XPLMSetDatavi(override_planepath, &setdata, 0, 1);
			destroyObjects();
			draw_fifty = 0;
			draw_hundred = 0;
			draw_two = 0;
			num_to_draw = -1;
			buffer = 0;
			
			updateLeaderboard(score);
			
			light = "r";
			light_changed = true;

			XPLMSetDataf(sim_volume, 0.6);
		}

		//Might not need want with the actual joysticks
		//Small flight help
		if (XPLMGetDataf(loc_y) < 2000) {
			XPLMSetDataf(trim_elv, 0.05);
		}
		else if (XPLMGetDataf(loc_y) > 6000) {
			XPLMSetDataf(trim_elv, -0.1);
		}
		else {
			XPLMSetDataf(trim_elv, -0.05);
		}

		break;

	case 3:
		//End of game
		//Set plane position
		if (((current_time - tracked_time) > 2 && setdata == 1) || (crashed && setdata==1)) {
			setPosition();
			setdata = 0;
			XPLMSetDatavi(override_planepath, &setdata, 0, 1);
		}

		//Show crash screne if crashed
		if (crashed) {
			buffer = 10;
			XPLMControlCamera(xplm_ControlCameraUntilViewChanges, cameraControl2, NULL);
			image_to_draw = CRASH_IMG;
			num_to_draw = -1;
			if ((current_time - tracked_time) > 5) {
				game_state = 0;
				tracked_time = current_time - 400;
			}
		}
		else if (buffer < 10) {
			image_to_draw = SCORE_IMG;
			buffer = buffer + 1;
		}
		//Otherwise show score screens
		else {
			if (num_to_draw == -1) {
				image_to_draw = SCORE_IMG;
				num_to_draw = 0;
			}
			else if ((current_time - tracked_time) < 7) {
				if (num_to_draw < score)
					num_to_draw = num_to_draw + 10;
			}
			else if ((current_time - tracked_time) < 14) {
				draw_top_scores = true;
				image_to_draw = LEADERBOARD_IMG;
				num_to_draw = score;
			}
			else {
				//Reset back to title screen
				draw_top_scores = false;
				game_state = 0;
				tracked_time = current_time - 400;
				break;
			}
		}
		
	}





	//Next flight loop callback happens the next frame
	return -1;
}

//Updates the daily and overall leaderboards with new score
void updateLeaderboard(int score) {
	for (int i = 0; i < 10; i++) {
		if (score >= daily_leaderboard[i]) {
			for (int j = 9; j > i; j--) {
				daily_leaderboard[j] = daily_leaderboard[j - 1];
			}
			daily_leaderboard[i] = score;
			break;
		}
	}

	for (int i = 0; i < 10; i++) {
		if (score >= leaderboard[i].total) {
			//Shift scores
			for (int j = 9; j > i; j--) {
					leaderboard[j] = leaderboard[j - 1];
					//add dates
				
			}
			leaderboard[i].total = score;
			//Add date
			time_t now = time(0);
			tm* localTime = localtime(&now);
			int dy = localTime->tm_mday;
			int mn = localTime->tm_mon; //If we change to number dates, add 1
			int yr = localTime->tm_year + 1900;

			leaderboard[i].day = dy;
			leaderboard[i].month = mn;
			leaderboard[i].year = yr;

			//rewrite to file
			return;
		}
	}
}

//Moves the plane on the title screen
void AutoFly(int x) {
	double lat = 36;
	double lon = -95;
	double alt = START_ALT;

	double local_x, local_y, local_z;
	XPLMWorldToLocal(lat, lon, alt, &local_x, &local_y, &local_z);
	XPLMSetDataf(loc_x, local_x + x);
	XPLMSetDataf(loc_y, local_y);
}

//Checks if the game should end
bool checkGameEnd(void) {
	
	if ((current_time - tracked_time) > 90) {
		crashed = false;
		return true;
	}

	if (XPLMGetDataf(loc_y) < 800) {
		crashed = true;
		return true;
	}

	return false;

}


//Checks if the game should start
bool checkUserInput(void) {

	float roll;
	float pitch;
	//Other way of joystick stuff, might need it
	/*
	XPLMGetDatavf(joystick_raw, &roll, 1, 1);
	XPLMGetDatavf(joystick_raw, &pitch, 0, 1);

if (roll < 0 || pitch < 0) {
	return false;
	}


	if (abs(roll-0.5) > 0.2 || abs(pitch-0.5) > 0.2) {
		return true;
	}
	else {
		return false;
	}
	*/
	XPLMGetDatavf(joystick_values, &roll, 2, 1);
	XPLMGetDatavf(joystick_values, &pitch, 1, 1);

	if (abs(roll) > 0.2 || abs(pitch) > 0.2) {
		return true;
	}
	else {
		return false;
	}
	
}

//Flight initializations
void startFlying(void) {
//Things we want to do at the start
	double lat = 36;
	double lon = -95;
	double alt = START_ALT;
	float vel = START_VEL;

	XPLMPlaceUserAtLocation(lat, lon, alt, 90, vel);

	float throttle = 0.8;
	XPLMSetDatavf(XPLMFindDataRef("sim/flightmodel/engine/ENGN_thro"), &throttle, 0, 1);
	XPLMSetDatavf(XPLMFindDataRef("sim/flightmodel/engine/ENGN_thro"), &throttle, 1, 1);

	XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_on"));
	XPLMCommandOnce(XPLMFindCommand("sim/autopilot/fdir_on"));
}




//Sphere methods
//Create the spheres
void createObjects(void) {
	//Create Spheres
	float x_offset = 800;
	float y_offset = 0;
	float z_offset = 0;

	srand(time(0));


	for (int i = 0; i < MAX_SPHERES; i++) {
		spheres[i].x = x_offset + (rand() % 800) + XPLMGetDataf(loc_x);
		//spheres[i].x = x_offset + XPLMGetDataf(loc_x) + i * 90;
		spheres[i].y = y_offset + (rand() % 200) - 100 + XPLMGetDataf(loc_y);
		spheres[i].z = z_offset + (rand() % 200) - 100 + XPLMGetDataf(loc_z);
		spheres[i].active = true;

		dr.structSize = sizeof(dr);
		dr.x = spheres[i].x;
		dr.y = spheres[i].y;
		dr.z = spheres[i].z;
		dr.pitch = XPLMGetDataf(pitch) * 0;
		dr.heading = XPLMGetDataf(heading);
		dr.roll = XPLMGetDataf(roll);

		//Set sphere size
		//7 medium spheres, 7 large, 6 small
		if (i < 7) {
			spheres[i].instance = XPLMCreateInstance(sphere_object, drefs);
			spheres[i].size = 2;
		}
		else if (i < 13) {
			spheres[i].instance = XPLMCreateInstance(sphere_sm, drefs);
			spheres[i].size = 1;
		}
		else if (i < 20) {
			spheres[i].instance = XPLMCreateInstance(sphere_lg, drefs);
			spheres[i].size = 3;
		}

		if (spheres[i].instance) {
			XPLMInstanceSetPosition(spheres[i].instance, &dr, NULL);
		}
	}

	
}

//Hand the sphere collection and spawning
void handleObjects() {
	//Go through all the spheres
	//Check if they meet conditions to be unspawned
	//Spawn new spheres

	for (int i = 0; i < MAX_SPHERES; i++) {

		if (spheres[i].active == true) {
			//Calculate sphere to plane distances
			float dx = spheres[i].x - XPLMGetDataf(loc_x);
			float dy = spheres[i].y - XPLMGetDataf(loc_y);
			float dz = spheres[i].z - XPLMGetDataf(loc_z);
			float d = pow(pow(dx, 2) + pow(dy, 2) + pow(dz, 2), 0.5);
			//Check is plane is near sphere
			if (d < (15 * spheres[i].size)) {
				//Unload Object
				spheres[i].active = false;
				//Get Points
				//Buttkicker and lights
				if (spheres[i].size == 1) {
					score = score + 200;
					draw_two = 300;
					light = "p";
					light_changed = true;
				}
				else if (spheres[i].size == 2) {
					score = score + 100;
					draw_hundred = 300;
					light = "g";
					light_changed = true;
				}
				else if (spheres[i].size == 3) {
					score = score + 50;
					draw_fifty = 300;
					light = "y";
					light_changed = true;
				}
			}

			//Check if object is behind plane
			float h = XPLMGetDataf(heading);
			h = (h + 90) * 3.14159 / 180;
			//Heading vector
			float hx = -sin(h);
			float hz = cos(h);
			//Cross product
			if (((hx * dz) - (hz * dx)) < 0) {
				spheres[i].active = false;
			}
		}

		float abs_pitch = abs(XPLMGetDataf(pitch));

		if (spheres[i].active == false && buffer <5 && abs_pitch<60) {
			location look_ahead = lookAhead(10);

			spheres[i].x = look_ahead.x + (rand() % 220) - 110;
			spheres[i].y = look_ahead.y + (rand() % 220) - 110;
			spheres[i].z = look_ahead.z + (rand() % 220) - 110;
			spheres[i].active = true;

			if (XPLMGetDataf(loc_y) < 3000) {
				spheres[i].y = spheres[i].y + 100;
			}
			else if (XPLMGetDataf(loc_y) > 6000) {
				spheres[i].y = spheres[i].y - 100;
			}

			dr.structSize = sizeof(dr);
			dr.x = spheres[i].x;
			dr.y = spheres[i].y;
			dr.z = spheres[i].z;
			dr.pitch = XPLMGetDataf(pitch) * 0;
			dr.heading = XPLMGetDataf(heading);
			dr.roll = XPLMGetDataf(roll);

			XPLMInstanceSetPosition(spheres[i].instance, &dr, NULL);

			buffer = 40;
		
		}
	}

	
}


void destroyObjects(void) {
	for (int i = 0; i < MAX_SPHERES; i++) {
		XPLMDestroyInstance(spheres[i].instance);
	}
}

location lookAhead(float t) {
	//Return approximately where the plane will be in t seconds
	//Used to ensure spheres spawn where the user is able to reach
	location look_ahead{};
	look_ahead.x = XPLMGetDataf(loc_x) + t * XPLMGetDataf(vel_x);
	look_ahead.y = XPLMGetDataf(loc_y) + t * XPLMGetDataf(vel_y);
	look_ahead.z = XPLMGetDataf(loc_z) + t * XPLMGetDataf(vel_z);

	return look_ahead;
}

//Camera spins around plane
int cameraControl(
	XPLMCameraPosition_t * outCameraPosition,
	int losingControl,
	void * refcon
)
{ 
	if (outCameraPosition && !losingControl) {

		//auto_x increases as plane auto flies
		outCameraPosition->x = XPLMGetDataf(loc_x) + 25 * -sin(auto_x/1200);
		outCameraPosition->y = XPLMGetDataf(loc_y) + 10;
		outCameraPosition->z = XPLMGetDataf(loc_z) + 25 * cos(auto_x/1200);
		outCameraPosition->pitch = -20;
		outCameraPosition->heading = auto_x / 1200 * (180/3.14159);
		outCameraPosition->roll = 0;
	}
	return 1;
}

//Camera faces sky
int cameraControl2(XPLMCameraPosition_t* outCameraPosition,
	int losingControl,
	void* refcon
) {
	if (outCameraPosition && !losingControl) {
		outCameraPosition->x = XPLMGetDataf(loc_x);
		outCameraPosition->y = XPLMGetDataf(loc_y) + 20;
		outCameraPosition->z = XPLMGetDataf(loc_z);
		outCameraPosition->pitch = 90;
		outCameraPosition->heading = 0;
		outCameraPosition->roll = 0;
	}
	return 1;
}




//Sets the plane to the start position
void setPosition() {

	setdata = 1;
	XPLMSetDatavi(override_planepath, &setdata, 0, 1);

	double lat = 36;
	double lon = -95;
	double alt = START_ALT;
	double psi = 90 * 3.14159 / 360;
	double theta = 0;
	double phi = 0;
	float q[4];
	float vel = START_VEL;


	q[0] = cos(psi) * cos(theta) * cos(phi) + sin(psi) * sin(theta) * sin(phi);
	q[1] = cos(psi) * cos(theta) * sin(phi) - sin(psi) * sin(theta) * cos(phi);
	q[2] = cos(psi) * sin(theta) * cos(phi) + sin(psi) * cos(theta) * sin(phi);
	q[3] = -cos(psi) * sin(theta) * sin(phi) + sin(psi) * cos(theta) * cos(phi);


	double local_x, local_y, local_z;
	XPLMWorldToLocal(lat, lon, alt, &local_x, &local_y, &local_z);

	XPLMSetDatavf(XPLMFindDataRef("sim/flightmodel/position/q"), q, 0, 4);

	XPLMSetDataf(loc_x, local_x);
	XPLMSetDataf(loc_y, local_y);
	XPLMSetDataf(loc_z, local_z);
	XPLMSetDataf(roll, 0); //bank angle
	XPLMSetDataf(heading, 90); //heading
	XPLMSetDataf(pitch, 0); //pitch
	XPLMSetDataf(XPLMFindDataRef("sim/flightmodel/position/P"), 0); //Roll rotation rate
	XPLMSetDataf(XPLMFindDataRef("sim/flightmodel/position/Q"), 0); //Pitch rotation rate
	XPLMSetDataf(XPLMFindDataRef("sim/flightmodel/position/R"), 0); //yaw rotation rate

	XPLMSetDataf(vel_x, vel); //velocity
	XPLMSetDataf(vel_y, 0); //velocity
	XPLMSetDataf(vel_z, 0); //velocity



	setdata = 0;
	//XPLMSetDatavi(override_planepath, &setdata, 0, 1);
}


//Draw window
void	draw(XPLMWindowID in_window_id, void* in_refcon)
{
	XPLMSetGraphicsState(
		0 /* fog */,
		1 /* texture units */,
		0 /* lighting */,
		0 /* alpha testing */,
		1 /* alpha blend */,
		1 /* depth testing */,
		0 /* depth writing */
	);

	if (image_to_draw != 0) {
		int b[4];
		XPLMGetWindowGeometry(in_window_id, &b[0], &b[3], &b[2], &b[1]);

		XPLMBindTexture2d(image_to_draw, 0);
		
		//Slides the score screen down from top
		if (game_state == 3 && buffer < 10) {
			float ratio = (float)buffer / 10;
			glBegin(GL_QUADS);
			glTexCoord2f(0, 1);
			glVertex2i(b[0], b[3] + (b[1]-b[3])*ratio);

			glTexCoord2f(0, (1-ratio));
			glVertex2i(b[0], b[3]);

			glTexCoord2f(1, (1-ratio));
			glVertex2i(b[2], b[3]);

			glTexCoord2f(1, 1);
			glVertex2i(b[2], b[3] + (b[1]-b[3])*ratio);
			glEnd();
		}
		else {
		
			glBegin(GL_QUADS);
			glTexCoord2f(0, 1);
			glVertex2i(b[0], b[1]);

			glTexCoord2f(0, 0);
			glVertex2i(b[0], b[3]);

			glTexCoord2f(1, 0);
			glVertex2i(b[2], b[3]);

			glTexCoord2f(1, 1);
			glVertex2i(b[2], b[1]);
			glEnd();
		}
	}
	

	//Draw numbers
	XPLMBindTexture2d(NUMBER_IMG, 0);

	//Draw the score and time icons
	if (game_state == 2) {
		drawNums(1670, 875, score, 2);
		int time_left = 90 - (current_time - tracked_time);
		drawNums(1670, 740, time_left, 2);
	}


	if (draw_top_scores) {
		//Draw daily scores
		for (int i = 0; i < 10; i++) {
			if(daily_leaderboard[i] > 0)
				drawNums(550, 700 - (i * 50), daily_leaderboard[i], 3);
		}
		//Draw top scores
		for (int i = 0; i < 10; i++) {
			drawNums(900, 700-(i*50), leaderboard[i].total, 3);
			drawMonth(1100, 700-(i*50), leaderboard[i].month);
			drawNums(1210, 700-(i*50), leaderboard[i].day, 3);
			drawNums(1300, 700-(i*50), leaderboard[i].year, 3);
		}
		drawNums(750, 50, score, 1);
	}
	else{
		if (num_to_draw >= 0) {
			if (num_to_draw < 10)
				drawNums(850, 486, num_to_draw, 1);
			else
				drawNums(750, 486, num_to_draw, 1);
		}
	}
	

	//Draw score icons
	if (draw_two > 0) {
		XPLMBindTexture2d(TWO_IMG, 0);

		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2i(760, 140);

		glTexCoord2f(0, 0);
		glVertex2i(760, 540);

		glTexCoord2f(1, 0);
		glVertex2i(1160, 540);

		glTexCoord2f(1, 1);
		glVertex2i(1160, 140);
		glEnd();

		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2i(760+1920, 140);

		glTexCoord2f(0, 0);
		glVertex2i(760+1920, 540);

		glTexCoord2f(1, 0);
		glVertex2i(1160+1920, 540);

		glTexCoord2f(1, 1);
		glVertex2i(1160+1920, 140);
		glEnd();
	}
	else if (draw_hundred > 0) {

		XPLMBindTexture2d(HUNDRED_IMG, 0);

		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2i(760, 140);

		glTexCoord2f(0, 0);
		glVertex2i(760, 540);

		glTexCoord2f(1, 0);
		glVertex2i(1160, 540);

		glTexCoord2f(1, 1);
		glVertex2i(1160, 140);
		glEnd();

		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2i(760+1920, 140);

		glTexCoord2f(0, 0);
		glVertex2i(760+1920, 540);

		glTexCoord2f(1, 0);
		glVertex2i(1160+1920, 540);

		glTexCoord2f(1, 1);
		glVertex2i(1160+1920, 140);
		glEnd();
	} 
	else if (draw_fifty > 0) {

		XPLMBindTexture2d(FIFTY_IMG, 0);

		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2i(760, 140);

		glTexCoord2f(0, 0);
		glVertex2i(760, 540);

		glTexCoord2f(1, 0);
		glVertex2i(1160, 540);

		glTexCoord2f(1, 1);
		glVertex2i(1160, 140);
		glEnd();

		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);
		glVertex2i(760+1920, 140);

		glTexCoord2f(0, 0);
		glVertex2i(760+1920, 540);

		glTexCoord2f(1, 0);
		glVertex2i(1160+1920, 540);

		glTexCoord2f(1, 1);
		glVertex2i(1160+1920, 140);
		glEnd();
	}

}

void drawNums(int left, int bot, int num, int size) {
	//Space between digits
	int width = 116 / size;
	
	int spot = 0;

	
	for (int i = 6; i > 0; i--) {
		int digit = (num % (int)pow(10,i)) / pow(10,i-1);
		if (digit > 0 || spot > 0) {
			drawDigits(left + (spot * width), bot, digit, size);
			spot = spot + 1;
		}
	}

	if(spot == 0)
		drawDigits(left + (spot * width), bot, 0, size);
}


void drawDigits(int left, int bot, int num, int size) {
	float number = (float)num / 10;
	//Width and height of text
	int height = 160 / size;
	int width = 96 / size;

	glBegin(GL_QUADS);
	glTexCoord2f(number, 1);
	glVertex2i(left, bot);

	glTexCoord2f(number, 0);
	glVertex2i(left, bot + height);

	glTexCoord2f(number+0.1, 0);
	glVertex2i(left+width, bot + height);

	glTexCoord2f(number+0.1, 1);
	glVertex2i(left+width, bot);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(number, 1);
	glVertex2i(left+1920, bot);

	glTexCoord2f(number, 0);
	glVertex2i(left+1920, bot + height);

	glTexCoord2f(number + 0.1, 0);
	glVertex2i(left + width+1920, bot + height);

	glTexCoord2f(number + 0.1, 1);
	glVertex2i(left + width+1920, bot);
	glEnd();
}


void drawMonth(int left, int bot, int mon) {
	float month = (float)mon / 12;

	float scale = 1.8;
	int height = 100.0 / scale;
	int width = 160.0 / scale;
	
	XPLMBindTexture2d(MONTH_IMG, 0);

	glBegin(GL_QUADS);
	glTexCoord2f(month, 1);
	glVertex2i(left, bot);

	glTexCoord2f(month, 0);
	glVertex2i(left, bot + height);

	glTexCoord2f(month + 1.0 / 12, 0);
	glVertex2i(left + width, bot + height);

	glTexCoord2f(month + 1.0 / 12, 1);
	glVertex2i(left + width, bot);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(month, 1);
	glVertex2i(left+1920, bot);

	glTexCoord2f(month, 0);
	glVertex2i(left+1920, bot + height);

	glTexCoord2f(month + 1.0 / 12, 0);
	glVertex2i(left + width+1920, bot + height);

	glTexCoord2f(month + 1.0 / 12, 1);
	glVertex2i(left + width+1920, bot);
	glEnd();


	XPLMBindTexture2d(NUMBER_IMG, 0);
}


void loadImages() {
	//Initialize window
	int desktop_bounds[4]; // left, bottom, right, top
	XPLMGetScreenBoundsGlobal(&desktop_bounds[0], &desktop_bounds[3], &desktop_bounds[2], &desktop_bounds[1]);

	XPLMCreateWindow_t params;
	params.structSize = sizeof(params);
	//Window parameters
	//params.left = desktop_bounds[0];
	params.left = 0;
	params.bottom = desktop_bounds[1];
	//params.right = desktop_bounds[2];
	params.right = 1920;
	params.top = desktop_bounds[3];
	params.visible = 1;
	params.drawWindowFunc = draw;
	params.handleMouseClickFunc = dummy_mouse_handler;
	params.handleRightClickFunc = dummy_mouse_handler;
	params.handleMouseWheelFunc = dummy_wheel_handler;
	params.handleKeyFunc = dummy_key_handler;
	params.handleCursorFunc = dummy_cursor_status_handler;
	params.refcon = NULL;
	params.layer = xplm_WindowLayerFloatingWindows;
	params.decorateAsFloatingWindow = 0;

	g_window = XPLMCreateWindowEx(&params);

	XPLMSetWindowPositioningMode(g_window, xplm_WindowPositionFree, -1);
	XPLMSetWindowGravity(g_window, 0, 1, 1, 1);

	XPLMCreateWindow_t params2 = params;
	params2.left = params.left + 1920;
	params2.right = params.right + 1920;
	g_window2 = XPLMCreateWindowEx(&params2);
	XPLMSetWindowPositioningMode(g_window2, xplm_WindowPositionFree, -1);
	XPLMSetWindowGravity(g_window2, 0, 1, 1, 1);


	//Load images
	char* imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/Title_text.png";
	int width, height, n;
	unsigned char* data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&TITLE_SCREEN_IMG, 1);
	XPLMBindTexture2d(TITLE_SCREEN_IMG, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);


	imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/Numbers.png";
	data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&NUMBER_IMG, 1);
	XPLMBindTexture2d(NUMBER_IMG, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);


	imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/Score.png";
	data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&SCORE_IMG, 1);
	XPLMBindTexture2d(SCORE_IMG, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	
	imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/Leaderboard.png";
	data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&LEADERBOARD_IMG, 1);
	XPLMBindTexture2d(LEADERBOARD_IMG, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/Tutorial.png";
	data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&TUTORIAL_IMG[3], 1);
	XPLMBindTexture2d(TUTORIAL_IMG[3], 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/Tutorial1.png";
	data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&TUTORIAL_IMG[0], 1);
	XPLMBindTexture2d(TUTORIAL_IMG[0], 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/Tutorial2.png";
	data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&TUTORIAL_IMG[1], 1);
	XPLMBindTexture2d(TUTORIAL_IMG[1], 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/Tutorial3.png";
	data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&TUTORIAL_IMG[2], 1);
	XPLMBindTexture2d(TUTORIAL_IMG[2], 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/Crash.png";
	data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&CRASH_IMG, 1);
	XPLMBindTexture2d(CRASH_IMG, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/HUD.png";
	data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&HUD_IMG, 1);
	XPLMBindTexture2d(HUD_IMG, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/100_score.png";
	data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&HUNDRED_IMG, 1);
	XPLMBindTexture2d(HUNDRED_IMG, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/50_score.png";
	data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&FIFTY_IMG, 1);
	XPLMBindTexture2d(FIFTY_IMG, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/200_score.png";
	data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&TWO_IMG, 1);
	XPLMBindTexture2d(TWO_IMG, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	imgPath = "C:/X-Plane 12/Resources/plugins/T37_Resources/Months.png";
	data = stbi_load(imgPath, &width, &height, &n, 0);
	XPLMGenerateTextureNumbers(&MONTH_IMG, 1);
	XPLMBindTexture2d(MONTH_IMG, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
}
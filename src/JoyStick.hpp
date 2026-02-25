#ifndef JOYSTICK_HPP
#define JOYSTICK_HPP

#include <string>
#include <iostream>

#define MAX_JS (5)
#define MAX_JS_ENTRY (18)
#define MAX_JS_AXIS (3)
#define MAX_JS_POV (0)
#define MAX_JS_BUTTON (MAX_JS_ENTRY - MAX_JS_AXIS - MAX_JS_POV)

enum eJsEntryChannel
{
    AXIS_PORT = 0x00,
    AXIS_STBD,
    AXIS_RUDDER,
    BUTTON_HORN,
    BUTTON_CHANGE_VIEW,
    BUTTON_CHANGE_LOCK_VIEW,
    BUTTON_STEP_LEFT,
    BUTTON_STEP_RIGHT,
    BUTTON_BEARING_ON,
    BUTTON_BEARING_OFF,
    BUTTON_ZOOM_ON,
    BUTTON_ZOOM_OFF,
    BUTTON_LOOK_LEFT,
    BUTTON_LOOK_RIGHT,
    BUTTON_LOOK_UP,
    BUTTON_LOOK_DOWN,
    BUTTON_ALARM,
    BUTTON_ACK_ALARM
};

enum eJsEntryType
{
    AXIS = 0x01,
    BUTTON,
    POV
};

struct sJsEntry
{
    eJsEntryType type;
    unsigned char channel;
    unsigned char jsNumber;
};

struct sJsMapping
{
    sJsEntry entry[MAX_JS_ENTRY];
};

class JoyStick
{

public:

  JoyStick();
  JoyStick(sJsMapping aJsMapping);
  ~JoyStick();

  bool Init(void *aModel, void *aGuiMain);
  static void Process(int aNumJoysticks, sJsMapping& aJsMapping, void *aModel, void* aGuiMain);

private:

	int mNumJoysticks;
    sJsMapping mJsMapping;

};


#endif

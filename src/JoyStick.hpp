#ifndef JOYSTICK_HPP
#define JOYSTICK_HPP

#include <string>
#include <iostream>

#define MAX_JS (5)
#define MAX_JS_ENTRY (18)
#define MAX_JS_AXIS (3)
#define MAX_JS_POV (0)
#define MAX_JS_BUTTON (MAX_JS_ENTRY - MAX_JS_AXIS - MAX_JS_POV)

typedef enum eJsEntryType
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

#ifndef __GUIMAIN_HPP_INCLUDED__
#define __GUIMAIN_HPP_INCLUDED__

#include "irrlicht.h"

#include <string>

class GUIMain //Create, build and update GUI
{
public:
    GUIMain(irr::IrrlichtDevice* device);

    enum GUI_ELEMENTS// Define some values that we'll use to identify individual GUI controls.
    {
        GUI_ID_HEADING_SCROLL_BAR = 101,
        GUI_ID_SPEED_SCROLL_BAR,
        GUI_ID_START_BUTTON
    };

    void updateGuiData(irr::f32 hdg, irr::f32 spd, irr::f32 depth, std::string currentTime, bool paused);

    void drawGUI();


private:

    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* guienv;


    irr::gui::IGUIScrollBar* hdgScrollbar;
    irr::gui::IGUIScrollBar* spdScrollbar;
    irr::gui::IGUIStaticText* dataDisplay;
    irr::gui::IGUIButton* pausedButton;

    irr::f32 guiHeading;
    irr::f32 guiSpeed;
    irr::f32 guiDepth;
    std::string guiTime;
    bool guiPaused;

};

#endif

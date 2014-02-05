#ifndef __GUIMAIN_HPP_INCLUDED__
#define __GUIMAIN_HPP_INCLUDED__

#include "irrlicht.h"

using namespace irr;

class GUIMain //Create, build and update GUI
{
public:
    GUIMain(IrrlichtDevice* dev);

    void updateGuiData(f32 hdg, f32 spd);

    void drawGUI();

private:
    IrrlichtDevice* device;
    gui::IGUIEnvironment* guienv;

    gui::IGUIScrollBar* spdScrollbar;
    gui::IGUIScrollBar* hdgScrollbar;
    gui::IGUIStaticText* dataDisplay;

    f32 guiHeading;
    f32 guiSpeed;

};

#endif

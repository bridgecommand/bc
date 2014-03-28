#ifndef __GUIMAIN_HPP_INCLUDED__
#define __GUIMAIN_HPP_INCLUDED__

#include "irrlicht.h"

class GUIMain //Create, build and update GUI
{
public:
    GUIMain(irr::IrrlichtDevice* dev);

    void updateGuiData(irr::f32 hdg, irr::f32 spd);

    void drawGUI();

    enum // Define some values that we'll use to identify individual GUI controls.
    {
        GUI_ID_HEADING_SCROLL_BAR = 101,
        GUI_ID_SPEED_SCROLL_BAR
    };

    //fixme: Temporarily moved to public
    irr::gui::IGUIScrollBar* spdScrollbar;

private:
    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* guienv;


    irr::gui::IGUIScrollBar* hdgScrollbar;
    irr::gui::IGUIStaticText* dataDisplay;

    irr::f32 guiHeading;
    irr::f32 guiSpeed;

};

#endif

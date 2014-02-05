#include "irrlicht.h"

#include "GUIMain.hpp"

using namespace irr;

GUIMain::GUIMain(IrrlichtDevice* dev)
    {
        device = dev;
        guienv = device->getGUIEnvironment();

        //gui - add scroll bars
        hdgScrollbar = guienv->addScrollBar(false,core::rect<s32>(10, 240, 30, 470), 0, GUI_ID_HEADING_SCROLL_BAR);
        hdgScrollbar->setMax(360);

        spdScrollbar = guienv->addScrollBar(false,core::rect<s32>(40, 240, 60, 470), 0, GUI_ID_SPEED_SCROLL_BAR);
        spdScrollbar->setMax(20.f*1852.f/3600.f); //20 knots in m/s

        //add heading indicator:
        dataDisplay = guienv->addStaticText(L"", core::rect<s32>(20,20,120,40), true); //Actual text set later
        guiHeading = 0;
        guiSpeed = 0;
    }

    void GUIMain::updateGuiData(f32 hdg, f32 spd)
    {
        guiHeading = hdg; //Heading in degrees
        guiSpeed = spd; //Speed in knots
    }

    void GUIMain::drawGUI()
    {
        //update heading display element
        core::stringw displayText = L"Heading: "; //Do we need to destroy this when done?
        displayText.append(core::stringw(guiHeading));
        displayText.append(L"\nSpeed: ");
        displayText.append(core::stringw(guiSpeed));
        dataDisplay->setText(displayText.c_str());

        guienv->drawAll();
    }

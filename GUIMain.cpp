/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2014 James Packer

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License version 2 as
     published by the Free Software Foundation

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY Or FITNESS For A PARTICULAR PURPOSE.  See the
     GNU General Public License For more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include "GUIMain.hpp"

#include "Constants.hpp"

using namespace irr;

GUIMain::GUIMain(IrrlichtDevice* device, Lang* language)
    {
        this->device = device;
        guienv = device->getGUIEnvironment();

        video::IVideoDriver* driver = device->getVideoDriver();
        u32 su = driver->getScreenSize().Width;
        u32 sh = driver->getScreenSize().Height;

        this->language = language;

        //gui - add scroll bars for speed and heading control directly
        hdgScrollbar = guienv->addScrollBar(false,core::rect<s32>(0.01*su, 0.61*sh, 0.04*su, 0.99*sh), 0, GUI_ID_HEADING_SCROLL_BAR);
        hdgScrollbar->setMax(360);
        spdScrollbar = guienv->addScrollBar(false,core::rect<s32>(0.05*su, 0.61*sh, 0.08*su, 0.99*sh), 0, GUI_ID_SPEED_SCROLL_BAR);
        spdScrollbar->setMax(20.f*1852.f/3600.f); //20 knots in m/s
        //Hide speed/heading bars normally
        hdgScrollbar->setVisible(false);
        spdScrollbar->setVisible(false);

        //Add engine and rudder bars
        portScrollbar = guienv->addScrollBar(false,core::rect<s32>(0.01*su, 0.61*sh, 0.04*su, 0.99*sh), 0, GUI_ID_PORT_SCROLL_BAR);
        portScrollbar->setMax(100);
        portScrollbar->setMin(-100);
        portScrollbar->setPos(0);
        stbdScrollbar = guienv->addScrollBar(false,core::rect<s32>(0.05*su, 0.61*sh, 0.08*su, 0.99*sh), 0, GUI_ID_STBD_SCROLL_BAR);
        stbdScrollbar->setMax(100);
        stbdScrollbar->setMin(-100);
        stbdScrollbar->setPos(0);
        rudderScrollbar = guienv->addScrollBar(true,core::rect<s32>(0.09*su, 0.96*sh, 0.57*su, 0.99*sh), 0, GUI_ID_RUDDER_SCROLL_BAR);
        rudderScrollbar->setMax(30);
        rudderScrollbar->setMin(-30);
        rudderScrollbar->setPos(0);

        //add data display:
        dataDisplay = guienv->addStaticText(L"", core::rect<s32>(0.09*su,0.61*sh,0.57*su,0.95*sh), true, false, 0, -1, true); //Actual text set later
        guiHeading = 0;
        guiSpeed = 0;

        //add radar buttons
        increaseRangeButton = guienv->addButton(core::rect<s32>(0.58*su,0.84*sh,0.63*su,0.91*sh),0,GUI_ID_RADAR_INCREASE_BUTTON,L"/\\");//i18n
        decreaseRangeButton = guienv->addButton(core::rect<s32>(0.58*su,0.92*sh,0.63*su,0.99*sh),0,GUI_ID_RADAR_DECREASE_BUTTON,L"\\/");//i18n

        //Add paused button
        pausedButton = guienv->addButton(core::rect<s32>(0.3*su,0.27*sh,0.7*su,0.73*sh),0,GUI_ID_START_BUTTON,language->translate("pausedbutton").c_str());
    }

    void GUIMain::updateGuiData(f32 hdg, f32 spd, f32 portEng, f32 stbdEng, f32 rudder, f32 depth, f32 radarRangeNm, std::string currentTime, bool paused)
    {
        //Update scroll bars
        hdgScrollbar->setPos(hdg);
        spdScrollbar->setPos(spd);
        portScrollbar->setPos(portEng * -100);//Engine units are +- 1, scale to -+100, inverted as astern is at bottom of scroll bar
        stbdScrollbar->setPos(stbdEng * -100);
        rudderScrollbar->setPos(rudder);
        //Update text display data
        guiHeading = hdg; //Heading in degrees
        guiSpeed = spd*MPS_TO_KTS; //Speed in knots
        guiDepth = depth;
        guiRadarRangeNm = radarRangeNm;
        guiTime = currentTime;
        guiPaused = paused;
    }

    void GUIMain::drawGUI()
    {
        //update heading display element
        core::stringw displayText = language->translate("hdg");
        displayText.append(core::stringw(guiHeading));
        displayText.append(L"\n");

        displayText.append(language->translate("spd"));
        displayText.append(core::stringw(guiSpeed));
        displayText.append(L"\n");

        displayText.append(language->translate("depth"));
        displayText.append(core::stringw(guiDepth));
        displayText.append(L"\n");

        displayText.append(language->translate("radar"));
        displayText.append(core::stringw(guiRadarRangeNm));
        displayText.append(language->translate("nm"));
        displayText.append(L"\n");

        displayText.append(core::stringw(guiTime.c_str()));
        displayText.append(L"\n");

        displayText.append(language->translate("fps"));
        displayText.append(core::stringw(device->getVideoDriver()->getFPS()).c_str());
        if (guiPaused) {
            displayText.append(language->translate("paused"));
        }
        dataDisplay->setText(displayText.c_str());

        //Remove big paused button when the simulation is started.
        if (pausedButton) {
            if (!guiPaused) {
                pausedButton->remove();
                pausedButton = 0;
            }
        }

        guienv->drawAll();
    }

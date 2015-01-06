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
        rudderScrollbar = guienv->addScrollBar(true,core::rect<s32>(0.09*su, 0.96*sh, 0.45*su, 0.99*sh), 0, GUI_ID_RUDDER_SCROLL_BAR);
        rudderScrollbar->setMax(30);
        rudderScrollbar->setMin(-30);
        rudderScrollbar->setPos(0);

        //add data display:
        dataDisplay = guienv->addStaticText(L"", core::rect<s32>(0.09*su,0.61*sh,0.45*su,0.95*sh), true, false, 0, -1, true); //Actual text set later
        guiHeading = 0;
        guiSpeed = 0;

        //Add weather scroll bar
        weatherScrollbar = guienv->addScrollBar(false,core::rect<s32>(0.417*su, 0.79*sh, 0.440*su, 0.94*sh), 0, GUI_ID_WEATHER_SCROLL_BAR);
        weatherScrollbar->setMax(120); //Divide by 10 to get weather
        weatherScrollbar->setMin(0);
        weatherScrollbar->setSmallStep(5);
        weatherScrollbar->setToolTipText(language->translate("weather").c_str());

        //Add rain scroll bar
        rainScrollbar = guienv->addScrollBar(false,core::rect<s32>(0.389*su, 0.79*sh, 0.412*su, 0.94*sh), 0, GUI_ID_RAIN_SCROLL_BAR);
        rainScrollbar->setMax(100);
        rainScrollbar->setMin(0);
        rainScrollbar->setLargeStep(5);
        rainScrollbar->setSmallStep(5);
        rainScrollbar->setToolTipText(language->translate("rain").c_str());

        //add radar buttons
        //add tab control for radar
        radarTabControl = guienv->addTabControl(core::rect<s32>(0.455*su,0.61*sh,0.697*su,0.99*sh));
        irr::gui::IGUITab* mainRadarTab = radarTabControl->addTab(language->translate("radarMainTab").c_str(),0);
        irr::gui::IGUITab* radarPITab = radarTabControl->addTab(language->translate("radarPITab").c_str(),0);
        irr::gui::IGUITab* radarGZoneTab = radarTabControl->addTab(language->translate("radarGuardZoneTab").c_str(),0);
        irr::gui::IGUITab* radarARPATab = radarTabControl->addTab(language->translate("radarARPATab").c_str(),0);
        irr::gui::IGUITab* radarTrackTab = radarTabControl->addTab(language->translate("radarTrackTab").c_str(),0);
        irr::gui::IGUITab* radarARPAVectorTab = radarTabControl->addTab(language->translate("radarARPAVectorTab").c_str(),0);
        irr::gui::IGUITab* radarARPAAlarmTab = radarTabControl->addTab(language->translate("radarARPAAlarmTab").c_str(),0);
        irr::gui::IGUITab* radarARPATrialTab = radarTabControl->addTab(language->translate("radarARPATrialTab").c_str(),0);

        increaseRangeButton = guienv->addButton(core::rect<s32>(0.005*su,0.110*sh,0.055*su,0.210*sh),mainRadarTab,GUI_ID_RADAR_INCREASE_BUTTON,language->translate("increaserange").c_str());
        decreaseRangeButton = guienv->addButton(core::rect<s32>(0.005*su,0.220*sh,0.055*su,0.320*sh),mainRadarTab,GUI_ID_RADAR_DECREASE_BUTTON,language->translate("decreaserange").c_str());
        radarGainScrollbar = guienv->addScrollBar(false,    core::rect<s32>(0.060*su,0.110*sh,0.085*su,0.32*sh),mainRadarTab,GUI_ID_RADAR_GAIN_SCROLL_BAR);
        radarClutterScrollbar = guienv->addScrollBar(false, core::rect<s32>(0.085*su,0.110*sh,0.110*su,0.32*sh),mainRadarTab,GUI_ID_RADAR_CLUTTER_SCROLL_BAR);
        radarRainScrollbar = guienv->addScrollBar(false,    core::rect<s32>(0.110*su,0.110*sh,0.135*su,0.32*sh),mainRadarTab,GUI_ID_RADAR_RAIN_SCROLL_BAR);
        radarGainScrollbar->setSmallStep(2);
        radarClutterScrollbar->setSmallStep(2);
        radarRainScrollbar->setSmallStep(2);
        radarGainScrollbar->setToolTipText(language->translate("gain").c_str());
        radarClutterScrollbar->setToolTipText(language->translate("clutter").c_str());
        radarRainScrollbar->setToolTipText(language->translate("rain").c_str());

        //Add paused button
        pausedButton = guienv->addButton(core::rect<s32>(0.3*su,0.27*sh,0.7*su,0.73*sh),0,GUI_ID_START_BUTTON,language->translate("pausedbutton").c_str());

        //show/hide interface
        showInterface = true; //If we start with the 2d interface shown
        showInterfaceButton = guienv->addButton(core::rect<s32>(0.09*su,0.92*sh,0.14*su,0.95*sh),0,GUI_ID_SHOW_INTERFACE_BUTTON,language->translate("showinterface").c_str());
        hideInterfaceButton = guienv->addButton(core::rect<s32>(0.09*su,0.92*sh,0.14*su,0.95*sh),0,GUI_ID_HIDE_INTERFACE_BUTTON,language->translate("hideinterface").c_str());
        showInterfaceButton->setVisible(false);

    }

    bool GUIMain::getShowInterface() const
    {
        return showInterface;
    }

    void GUIMain::toggleShow2dInterface()
    {
        showInterface = !showInterface;
        updateVisibility();
    }

    void GUIMain::show2dInterface()
    {
        showInterface = true;
        updateVisibility();
    }

    void GUIMain::hide2dInterface()
    {
        showInterface = false;
        updateVisibility();
    }

    void GUIMain::updateVisibility()
    {
        //Items to show if we're showing interface
        radarTabControl->setVisible(showInterface);
        dataDisplay->setVisible(showInterface);
        hideInterfaceButton->setVisible(showInterface);
        weatherScrollbar->setVisible(showInterface);
        rainScrollbar->setVisible(showInterface);

        //Items to show if we're not
        showInterfaceButton->setVisible(!showInterface);
    }

    void GUIMain::updateGuiData(f32 hdg, f32 viewAngle, f32 spd, f32 portEng, f32 stbdEng, f32 rudder, f32 depth, f32 weather, f32 rain, f32 radarRangeNm, irr::f32 radarGain, irr::f32 radarClutter, irr::f32 radarRain, std::string currentTime, bool paused)
    {
        //Update scroll bars
        hdgScrollbar->setPos(hdg);
        spdScrollbar->setPos(spd);
        portScrollbar->setPos(portEng * -100);//Engine units are +- 1, scale to -+100, inverted as astern is at bottom of scroll bar
        stbdScrollbar->setPos(stbdEng * -100);
        rudderScrollbar->setPos(rudder);
        radarGainScrollbar->setPos(radarGain);
        radarClutterScrollbar->setPos(radarClutter);
        radarRainScrollbar->setPos(radarRain);
        weatherScrollbar->setPos(weather*10.0); //(Weather scroll bar is 0-120, weather is 0-12)
        rainScrollbar->setPos(rain*10.0); //(Rain scroll bar is 0-100, rain is 0-10)
        //Update text display data
        guiHeading = hdg; //Heading in degrees
        viewHdg = viewAngle+hdg;
        while (viewHdg>=360) {viewHdg-=360;}
        while (viewHdg<0) {viewHdg+=360;}
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

        displayText.append(language->translate("viewAng"));
        displayText.append(core::stringw(viewHdg));
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

        //draw the heading line on the radar
        if (showInterface) {
            u32 su = device->getVideoDriver()->getScreenSize().Width;
            u32 sh = device->getVideoDriver()->getScreenSize().Height;
            s32 centreX = su-0.2*sh;
            s32 centreY = 0.8*sh;
            s32 deltaX = 0.2*sh*sin(core::DEGTORAD*guiHeading);
            s32 deltaY = -0.2*sh*cos(core::DEGTORAD*guiHeading);
            core::position2d<s32> radarCentre (centreX,centreY);
            core::position2d<s32> radarHeading (centreX+deltaX,centreY+deltaY);
            device->getVideoDriver()->draw2DLine(radarCentre,radarHeading,video::SColor(255, 255, 255, 255));

            //draw a look direction line
            s32 deltaXView = 0.2*sh*sin(core::DEGTORAD*viewHdg);
            s32 deltaYView = -0.2*sh*cos(core::DEGTORAD*viewHdg);
            core::position2d<s32> lookInner (centreX + 0.9*deltaXView,centreY + 0.9*deltaYView);
            core::position2d<s32> lookOuter (centreX + deltaXView,centreY + deltaYView);
            device->getVideoDriver()->draw2DLine(lookInner,lookOuter,video::SColor(255, 255, 0, 0));
        }
    }

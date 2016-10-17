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
#include "Utilities.hpp"
#include "OutlineScrollBar.h"
#include "ScrollDial.h"

#include <iostream> //for debugging

using namespace irr;

GUIMain::GUIMain(IrrlichtDevice* device, Lang* language, std::vector<std::string>* logMessages)
    {
        this->device = device;
        guienv = device->getGUIEnvironment();

        video::IVideoDriver* driver = device->getVideoDriver();
        su = driver->getScreenSize().Width;
        sh = driver->getScreenSize().Height;

        this->language = language;
        this->logMessages = logMessages;

        //default to double engine in gui
        singleEngine = false;

        //Default to small radar display
        radarLarge = false;
        //Find available 4:3 rectangle to fit in area for large radar display
        s32 availableWidth  = (0.99-0.09)*su;
        s32 availableHeight = (0.95-0.01)*sh;
        if (availableWidth/(float)availableHeight > 4.0/3.0) {
            s32 activeWidth = availableHeight * 4.0/3.0;
            s32 activeHeight = availableHeight;
            radarLargeRect = core::rect<s32>(0.09*su + (availableWidth-activeWidth)/2, 0.01*sh, 0.09*su + activeWidth + (availableWidth-activeWidth)/2, 0.01+activeHeight);
        } else {
            s32 activeWidth = availableWidth;
            s32 activeHeight = availableWidth * 3.0/4.0;
            radarLargeRect = core::rect<s32>(0.09*su, 0.01*sh+(availableHeight-activeHeight)/2, 0.09*su + activeWidth, 0.01+activeHeight+(availableHeight-activeHeight)/2);
        }
        //For brevity, store large radar window width and top left corner.
        s32 radarSu = radarLargeRect.getWidth();
        core::vector2d<s32> radarTL = radarLargeRect.UpperLeftCorner;
        //Find radar screen centre X, Y and radius
        largeRadarScreenRadius = (radarLargeRect.LowerRightCorner.Y-radarTL.Y)/2;
        largeRadarScreenCentreX = radarTL.X + largeRadarScreenRadius;
        largeRadarScreenCentreY = (radarLargeRect.LowerRightCorner.Y+radarTL.Y)/2;
        largeRadarScreenRadius*=0.95; //Make display slightly smaller, keeping the centre in the same place

        smallRadarScreenCentreX = su-0.2*sh;
        smallRadarScreenCentreY = 0.8*sh;
        smallRadarScreenRadius=0.2*sh;

        //gui - add scroll bars for speed and heading control directly
        hdgScrollbar = new gui::OutlineScrollBar(false,guienv,guienv->getRootGUIElement(),GUI_ID_HEADING_SCROLL_BAR,core::rect<s32>(0.01*su, 0.61*sh, 0.04*su, 0.99*sh));
        hdgScrollbar->setMax(360);
        spdScrollbar = new gui::OutlineScrollBar(false,guienv,guienv->getRootGUIElement(),GUI_ID_SPEED_SCROLL_BAR,core::rect<s32>(0.05*su, 0.61*sh, 0.08*su, 0.99*sh));
        spdScrollbar->setMax(20.f*1852.f/3600.f); //20 knots in m/s
        //Hide speed/heading bars normally
        hdgScrollbar->setVisible(false);
        spdScrollbar->setVisible(false);

        //Add engine and rudder bars
        core::array<s32> rudderTics; rudderTics.push_back(-25);rudderTics.push_back(-20);rudderTics.push_back(-15);rudderTics.push_back(-10);rudderTics.push_back(-5);
        rudderTics.push_back(5);rudderTics.push_back(10);rudderTics.push_back(15);rudderTics.push_back(20);rudderTics.push_back(25);

        core::array<s32> engineTics; engineTics.push_back(-80);engineTics.push_back(-60);engineTics.push_back(-40);engineTics.push_back(-20);
        engineTics.push_back(20);engineTics.push_back(40);engineTics.push_back(60);engineTics.push_back(80);

        core::array<s32> centreTic; centreTic.push_back(0);

        portText = guienv->addStaticText(language->translate("portEngine").c_str(),core::rect<s32>(0.005*su, 0.61*sh, 0.045*su, 0.67*sh));
        portText->setTextAlignment(gui::EGUIA_CENTER,gui::EGUIA_CENTER);
        portText->setOverrideColor(video::SColor(255,128,0,0));
        portScrollbar = new gui::OutlineScrollBar(false,guienv,guienv->getRootGUIElement(),GUI_ID_PORT_SCROLL_BAR,core::rect<s32>(0.01*su, 0.675*sh, 0.04*su, 0.99*sh),engineTics,centreTic);
        portScrollbar->setMax(100);
        portScrollbar->setMin(-100);
        portScrollbar->setPos(0);
        stbdText = guienv->addStaticText(language->translate("stbdEngine").c_str(),core::rect<s32>(0.045*su, 0.61*sh, 0.085*su, 0.67*sh));
        stbdText->setTextAlignment(gui::EGUIA_CENTER,gui::EGUIA_CENTER);
        stbdText->setOverrideColor(video::SColor(255,0,128,0));
        stbdScrollbar = new gui::OutlineScrollBar(false,guienv,guienv->getRootGUIElement(),GUI_ID_STBD_SCROLL_BAR,core::rect<s32>(0.05*su, 0.675*sh, 0.08*su, 0.99*sh),engineTics,centreTic);
        stbdScrollbar->setMax(100);
        stbdScrollbar->setMin(-100);
        stbdScrollbar->setPos(0);
        rudderScrollbar = new gui::OutlineScrollBar(true,guienv,guienv->getRootGUIElement(),GUI_ID_RUDDER_SCROLL_BAR,core::rect<s32>(0.09*su, 0.96*sh, 0.45*su, 0.99*sh),rudderTics,centreTic);
        rudderScrollbar->setMax(30);
        rudderScrollbar->setMin(-30);
        rudderScrollbar->setPos(0);

        //add data display:
        dataDisplay = guienv->addStaticText(L"", core::rect<s32>(0.09*su,0.71*sh,0.45*su,0.95*sh), true, false, 0, -1, true); //Actual text set later
        guiHeading = 0;
        guiSpeed = 0;

        //Add heading indicator
        stdHdgIndicatorPos = core::rect<s32>(0.09*su,0.630*sh,0.45*su,0.680*sh);
        altHdgIndicatorPos = core::rect<s32>(0.09*su,0.900*sh,0.45*su,0.950*sh);
        headingIndicator = new gui::HeadingIndicator(guienv,guienv->getRootGUIElement(),stdHdgIndicatorPos);

        //Add weather scroll bar
        //weatherScrollbar = guienv->addScrollBar(false,core::rect<s32>(0.417*su, 0.79*sh, 0.440*su, 0.94*sh), 0, GUI_ID_WEATHER_SCROLL_BAR);
        weatherScrollbar = new gui::ScrollDial(core::vector2d<s32>(0.290*su,0.90*sh),0.02*su,guienv,guienv->getRootGUIElement(),GUI_ID_WEATHER_SCROLL_BAR);
        weatherScrollbar->setMax(120); //Divide by 10 to get weather
        weatherScrollbar->setMin(0);
        weatherScrollbar->setSmallStep(5);
        weatherScrollbar->setToolTipText(language->translate("weather").c_str());

        //Add rain scroll bar
        //rainScrollbar = guienv->addScrollBar(false,core::rect<s32>(0.389*su, 0.79*sh, 0.412*su, 0.94*sh), 0, GUI_ID_RAIN_SCROLL_BAR);
        rainScrollbar = new gui::ScrollDial(core::vector2d<s32>(0.340*su,0.90*sh),0.02*su,guienv,guienv->getRootGUIElement(),GUI_ID_RAIN_SCROLL_BAR);
        rainScrollbar->setMax(100);
        rainScrollbar->setMin(0);
        rainScrollbar->setLargeStep(5);
        rainScrollbar->setSmallStep(5);
        rainScrollbar->setToolTipText(language->translate("rain").c_str());

        //Add visibility scroll bar: Will be divided by 10 to get visibility in Nm
        //visibilityScrollbar = guienv->addScrollBar(false,core::rect<s32>(0.361*su, 0.79*sh, 0.384*su, 0.94*sh),0,GUI_ID_VISIBILITY_SCROLL_BAR);
        visibilityScrollbar = new gui::ScrollDial(core::vector2d<s32>(0.390*su,0.90*sh),0.02*su,guienv,guienv->getRootGUIElement(),GUI_ID_VISIBILITY_SCROLL_BAR);
        visibilityScrollbar->setMax(101);
        visibilityScrollbar->setMin(1);
        visibilityScrollbar->setLargeStep(5);
        visibilityScrollbar->setSmallStep(1);
        visibilityScrollbar->setToolTipText(language->translate("visibility").c_str());

        //add radar buttons
        //add tab control for radar
        radarTabControl = guienv->addTabControl(core::rect<s32>(0.455*su,0.695*sh,0.697*su,0.990*sh),0,true);
        irr::gui::IGUITab* mainRadarTab = radarTabControl->addTab(language->translate("radarMainTab").c_str(),0);
        //irr::gui::IGUITab* radarEBLTab = radarTabControl->addTab(language->translate("radarEBLVRMTab").c_str(),0);
        //irr::gui::IGUITab* radarPITab = radarTabControl->addTab(language->translate("radarPITab").c_str(),0);
        //irr::gui::IGUITab* radarGZoneTab = radarTabControl->addTab(language->translate("radarGuardZoneTab").c_str(),0);
        irr::gui::IGUITab* radarARPATab = radarTabControl->addTab(language->translate("radarARPATab").c_str(),0);
        //irr::gui::IGUITab* radarTrackTab = radarTabControl->addTab(language->translate("radarTrackTab").c_str(),0);
        //irr::gui::IGUITab* radarARPAVectorTab = radarTabControl->addTab(language->translate("radarARPAVectorTab").c_str(),0);
        //irr::gui::IGUITab* radarARPAAlarmTab = radarTabControl->addTab(language->translate("radarARPAAlarmTab").c_str(),0);
        //irr::gui::IGUITab* radarARPATrialTab = radarTabControl->addTab(language->translate("radarARPATrialTab").c_str(),0);

        radarText = guienv->addStaticText(L"",core::rect<s32>(0.460*su,0.610*sh,0.690*su,0.690*sh),true,true,0,-1,true);

        //Buttons for full or small radar
        bigRadarButton = guienv->addButton(core::rect<s32>(0.700*su,0.610*sh,0.720*su,0.640*sh),0,GUI_ID_BIG_RADAR_BUTTON,language->translate("bigRadar").c_str());
        s32 smallRadarButtonLeft = radarTL.X + 0.01*su;
        s32 smallRadarButtonTop = radarTL.Y + 0.01*sh;
        smallRadarButton = guienv->addButton(core::rect<s32>(smallRadarButtonLeft,smallRadarButtonTop,smallRadarButtonLeft+0.020*su,smallRadarButtonTop+0.030*sh),0,GUI_ID_SMALL_RADAR_BUTTON,language->translate("smallRadar").c_str());
        bigRadarButton->setToolTipText(language->translate("fullScreenRadar").c_str());
        smallRadarButton->setToolTipText(language->translate("minimiseRadar").c_str());

        guienv->addButton(core::rect<s32>(0.005*su,0.010*sh,0.055*su,0.070*sh),mainRadarTab,GUI_ID_RADAR_INCREASE_BUTTON,language->translate("increaserange").c_str());
        guienv->addButton(core::rect<s32>(0.005*su,0.080*sh,0.055*su,0.140*sh),mainRadarTab,GUI_ID_RADAR_DECREASE_BUTTON,language->translate("decreaserange").c_str());

        guienv->addButton(core::rect<s32>(0.005*su,0.150*sh,0.055*su,0.180*sh),mainRadarTab,GUI_ID_RADAR_NORTH_BUTTON,language->translate("northUp").c_str());
        guienv->addButton(core::rect<s32>(0.005*su,0.180*sh,0.055*su,0.210*sh),mainRadarTab,GUI_ID_RADAR_COURSE_BUTTON,language->translate("courseUp").c_str());
        guienv->addButton(core::rect<s32>(0.005*su,0.210*sh,0.055*su,0.240*sh),mainRadarTab,GUI_ID_RADAR_HEAD_BUTTON,language->translate("headUp").c_str());

        //Controls for small radar window
        radarGainScrollbar    = new gui::ScrollDial(core::vector2d<s32>(0.0850*su,0.040*sh),0.02*su,guienv,mainRadarTab,GUI_ID_RADAR_GAIN_SCROLL_BAR);
        radarClutterScrollbar = new gui::ScrollDial(core::vector2d<s32>(0.1425*su,0.040*sh),0.02*su,guienv,mainRadarTab,GUI_ID_RADAR_CLUTTER_SCROLL_BAR);
        radarRainScrollbar    = new gui::ScrollDial(core::vector2d<s32>(0.2000*su,0.040*sh),0.02*su,guienv,mainRadarTab,GUI_ID_RADAR_RAIN_SCROLL_BAR);
        (guienv->addStaticText(language->translate("gain").c_str(),core::rect<s32>(0.0600*su,0.070*sh,0.1100*su,0.100*sh),false,true,mainRadarTab))->setTextAlignment(gui::EGUIA_CENTER,gui::EGUIA_CENTER);
        (guienv->addStaticText(language->translate("clutter").c_str(),core::rect<s32>(0.1165*su,0.070*sh,0.1675*su,0.100*sh),false,true,mainRadarTab))->setTextAlignment(gui::EGUIA_CENTER,gui::EGUIA_CENTER);
        (guienv->addStaticText(language->translate("rain").c_str(),core::rect<s32>(0.1750*su,0.070*sh,0.2250*su,0.100*sh),false,true,mainRadarTab))->setTextAlignment(gui::EGUIA_CENTER,gui::EGUIA_CENTER);
        radarGainScrollbar->setSmallStep(2);
        radarClutterScrollbar->setSmallStep(2);
        radarRainScrollbar->setSmallStep(2);

        eblLeftButton = guienv->addButton(core::rect<s32>(0.060*su,0.160*sh,0.115*su,0.190*sh),mainRadarTab,GUI_ID_RADAR_EBL_LEFT_BUTTON,language->translate("eblLeft").c_str());
        eblRightButton = guienv->addButton(core::rect<s32>(0.170*su,0.160*sh,0.225*su,0.190*sh),mainRadarTab,GUI_ID_RADAR_EBL_RIGHT_BUTTON,language->translate("eblRight").c_str());
        eblUpButton = guienv->addButton(core::rect<s32>(0.115*su,0.130*sh,0.170*su,0.160*sh),mainRadarTab,GUI_ID_RADAR_EBL_UP_BUTTON,language->translate("eblUp").c_str());
        eblDownButton = guienv->addButton(core::rect<s32>(0.115*su,0.190*sh,0.170*su,0.220*sh),mainRadarTab,GUI_ID_RADAR_EBL_DOWN_BUTTON,language->translate("eblDown").c_str());

        //Controls for large radar window
        largeRadarControls = new gui::IGUIRectangle(guienv,guienv->getRootGUIElement(),core::rect<s32>(radarTL.X+0.770*radarSu,radarTL.Y+0.020*radarSu,radarTL.X+0.980*radarSu,radarTL.Y+0.730*radarSu));
        radarGainScrollbar2    = new gui::ScrollDial(core::vector2d<s32>(0.040*radarSu,0.040*radarSu),0.03*radarSu,guienv,largeRadarControls,GUI_ID_RADAR_GAIN_SCROLL_BAR);
        radarClutterScrollbar2 = new gui::ScrollDial(core::vector2d<s32>(0.105*radarSu,0.040*radarSu),0.03*radarSu,guienv,largeRadarControls,GUI_ID_RADAR_CLUTTER_SCROLL_BAR);
        radarRainScrollbar2    = new gui::ScrollDial(core::vector2d<s32>(0.170*radarSu,0.040*radarSu),0.03*radarSu,guienv,largeRadarControls,GUI_ID_RADAR_RAIN_SCROLL_BAR);

        radarGainScrollbar2->setSmallStep(2);
        radarClutterScrollbar2->setSmallStep(2);
        radarRainScrollbar2->setSmallStep(2);

        (guienv->addStaticText(language->translate("gain").c_str(),core::rect<s32>(0.010*radarSu,0.070*radarSu,0.070*radarSu,0.100*radarSu),false,true,largeRadarControls))->setTextAlignment(gui::EGUIA_CENTER,gui::EGUIA_CENTER);
        (guienv->addStaticText(language->translate("clutter").c_str(),core::rect<s32>(0.075*radarSu,0.070*radarSu,0.135*radarSu,0.100*radarSu),false,true,largeRadarControls))->setTextAlignment(gui::EGUIA_CENTER,gui::EGUIA_CENTER);
        (guienv->addStaticText(language->translate("rain").c_str(),core::rect<s32>(0.140*radarSu,0.070*radarSu,0.200*radarSu,0.100*radarSu),false,true,largeRadarControls))->setTextAlignment(gui::EGUIA_CENTER,gui::EGUIA_CENTER);

        guienv->addButton(core::rect<s32>(0.025*radarSu,0.110*radarSu,0.085*radarSu,0.160*radarSu),largeRadarControls,GUI_ID_RADAR_INCREASE_BUTTON,language->translate("increaserange").c_str());
        guienv->addButton(core::rect<s32>(0.025*radarSu,0.165*radarSu,0.085*radarSu,0.210*radarSu),largeRadarControls,GUI_ID_RADAR_DECREASE_BUTTON,language->translate("decreaserange").c_str());

        guienv->addButton(core::rect<s32>(0.125*radarSu,0.110*radarSu,0.190*radarSu,0.140*radarSu),largeRadarControls,GUI_ID_RADAR_NORTH_BUTTON,language->translate("northUp").c_str());
        guienv->addButton(core::rect<s32>(0.125*radarSu,0.145*radarSu,0.190*radarSu,0.175*radarSu),largeRadarControls,GUI_ID_RADAR_COURSE_BUTTON,language->translate("courseUp").c_str());
        guienv->addButton(core::rect<s32>(0.125*radarSu,0.180*radarSu,0.190*radarSu,0.210*radarSu),largeRadarControls,GUI_ID_RADAR_HEAD_BUTTON,language->translate("headUp").c_str());

        eblLeftButton2 = guienv->addButton(core::rect<s32>(0.025*radarSu,0.245*radarSu,0.080*radarSu,0.275*radarSu),largeRadarControls,GUI_ID_RADAR_EBL_LEFT_BUTTON,language->translate("eblLeft").c_str());
        eblRightButton2 = guienv->addButton(core::rect<s32>(0.135*radarSu,0.245*radarSu,0.190*radarSu,0.275*radarSu),largeRadarControls,GUI_ID_RADAR_EBL_RIGHT_BUTTON,language->translate("eblRight").c_str());
        eblUpButton2 = guienv->addButton(core::rect<s32>(0.080*radarSu,0.215*radarSu,0.135*radarSu,0.245*radarSu),largeRadarControls,GUI_ID_RADAR_EBL_UP_BUTTON,language->translate("eblUp").c_str());
        eblDownButton2 = guienv->addButton(core::rect<s32>(0.080*radarSu,0.275*radarSu,0.135*radarSu,0.305*radarSu),largeRadarControls,GUI_ID_RADAR_EBL_DOWN_BUTTON,language->translate("eblDown").c_str());

        radarText2 = guienv->addStaticText(L"",core::rect<s32>(0.010*radarSu,0.310*radarSu,0.200*radarSu,0.400*radarSu),true,true,largeRadarControls,-1,true);

        //Radar ARPA tab
        guienv->addCheckBox(false,core::rect<s32>(0.005*su,0.010*sh,0.025*su,0.030*sh),radarARPATab,GUI_ID_ARPA_ON_BOX);
        (guienv->addStaticText(language->translate("ARPAon").c_str(),core::rect<s32>(0.030*su,0.010*sh,0.140*su,0.030*sh),false,true,radarARPATab))->setTextAlignment(gui::EGUIA_CENTER,gui::EGUIA_CENTER);
        irr::gui::IGUIComboBox* arpaVectorMode = guienv->addComboBox(core::rect<s32>(0.005*su,0.040*sh,0.150*su,0.080*sh),radarARPATab,GUI_ID_ARPA_TRUE_REL_BOX);
        arpaVectorMode->addItem(language->translate("trueArpa").c_str());
        arpaVectorMode->addItem(language->translate("relArpa").c_str());
        guienv->addEditBox(L"6",core::rect<s32>(0.155*su,0.040*sh,0.195*su,0.080*sh),true,radarARPATab,GUI_ID_ARPA_VECTOR_TIME_BOX);
        (guienv->addStaticText(language->translate("minsARPA").c_str(),core::rect<s32>(0.200*su,0.040*sh,0.237*su,0.080*sh),false,true,radarARPATab))->setTextAlignment(gui::EGUIA_CENTER,gui::EGUIA_CENTER);

        //Radar ARPA on big radar screen
        guienv->addCheckBox(false,core::rect<s32>(0.010*radarSu,0.410*radarSu,0.030*radarSu,0.430*radarSu),largeRadarControls,GUI_ID_BIG_ARPA_ON_BOX);
        (guienv->addStaticText(language->translate("ARPAon").c_str(),core::rect<s32>(0.040*radarSu,0.410*radarSu,0.160*radarSu,0.430*radarSu),false,true,largeRadarControls))->setTextAlignment(gui::EGUIA_CENTER,gui::EGUIA_CENTER);
        arpaVectorMode = guienv->addComboBox(core::rect<s32>(0.010*radarSu,0.440*radarSu,0.200*radarSu,0.470*radarSu),largeRadarControls,GUI_ID_BIG_ARPA_TRUE_REL_BOX);
        arpaVectorMode->addItem(language->translate("trueArpa").c_str());
        arpaVectorMode->addItem(language->translate("relArpa").c_str());
        guienv->addEditBox(L"6",core::rect<s32>(0.010*radarSu,0.480*radarSu,0.050*radarSu,0.510*radarSu),true,largeRadarControls,GUI_ID_BIG_ARPA_VECTOR_TIME_BOX);
        (guienv->addStaticText(language->translate("minsARPA").c_str(),core::rect<s32>(0.060*radarSu,0.480*radarSu,0.105*radarSu,0.510*radarSu),false,true,largeRadarControls))->setTextAlignment(gui::EGUIA_CENTER,gui::EGUIA_CENTER);

        //Add paused button
        pausedButton = guienv->addButton(core::rect<s32>(0.3*su,0.27*sh,0.7*su,0.73*sh),0,GUI_ID_START_BUTTON,language->translate("pausedbutton").c_str());

        //show/hide interface
        showInterface = true; //If we start with the 2d interface shown
        showInterfaceButton = guienv->addButton(core::rect<s32>(0.09*su,0.92*sh,0.14*su,0.95*sh),0,GUI_ID_SHOW_INTERFACE_BUTTON,language->translate("showinterface").c_str());
        hideInterfaceButton = guienv->addButton(core::rect<s32>(0.09*su,0.92*sh,0.14*su,0.95*sh),0,GUI_ID_HIDE_INTERFACE_BUTTON,language->translate("hideinterface").c_str());
        showInterfaceButton->setVisible(false);

        //binoculars button
        binosButton = guienv->addButton(core::rect<s32>(0.14*su,0.92*sh,0.19*su,0.95*sh),0,GUI_ID_BINOS_INTERFACE_BUTTON,language->translate("zoom").c_str());
        binosButton->setIsPushButton(true);

        //Take bearing button
        bearingButton = guienv->addButton(core::rect<s32>(0.19*su,0.92*sh,0.24*su,0.95*sh),0,GUI_ID_BEARING_INTERFACE_BUTTON,language->translate("bearing").c_str());
        bearingButton->setIsPushButton(true);

        //Show internal log window button
        pcLogButton = guienv->addButton(core::rect<s32>(0.24*su,0.92*sh,0.26*su,0.95*sh),0,GUI_ID_SHOW_LOG_BUTTON,language->translate("log").c_str());

        //Set initial visibility
        updateVisibility();

    }

    GUIMain::~GUIMain()
    {
        //Drop scroll bars created with 'new'
        portScrollbar->drop();
        stbdScrollbar->drop();
        rudderScrollbar->drop();

        weatherScrollbar->drop();
        visibilityScrollbar->drop();
        rainScrollbar->drop();

        radarGainScrollbar->drop();
        radarClutterScrollbar->drop();
        radarRainScrollbar->drop();

        radarGainScrollbar2->drop();
        radarClutterScrollbar2->drop();
        radarRainScrollbar2->drop();

        largeRadarControls->drop();

        hdgScrollbar->drop();
        spdScrollbar->drop();

        headingIndicator->drop();
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

    void GUIMain::setLargeRadar(bool radarState)
    {
        radarLarge = radarState;
        updateVisibility();
    }

    bool GUIMain::getLargeRadar() const
    {
        return radarLarge;
    }

    u32 GUIMain::getRadarPixelRadius() const
    {
        if (radarLarge) {
            return largeRadarScreenRadius;
        } else {
            return smallRadarScreenRadius;
        }
    }

    core::vector2di GUIMain::getCursorPositionRadar() const
    {
        //Basic mouse position
        core::vector2di cursorPosition = device->getCursorControl()->getPosition();

        //Radar screen centre position
        core::vector2di radarScreenCentre;
        if (radarLarge) {
            radarScreenCentre.X = largeRadarScreenCentreX;
            radarScreenCentre.Y = largeRadarScreenCentreY;
        } else {
            radarScreenCentre.X = smallRadarScreenCentreX;
            radarScreenCentre.Y = smallRadarScreenCentreY;
        }

        //Return the difference
        return (cursorPosition-radarScreenCentre);
    }

    irr::core::rect<irr::s32> GUIMain::getLargeRadarRect() const
    {
        return core::rect<s32>(largeRadarScreenCentreX - largeRadarScreenRadius, largeRadarScreenCentreY - largeRadarScreenRadius, largeRadarScreenCentreX + largeRadarScreenRadius, largeRadarScreenCentreY + largeRadarScreenRadius);
    }

    void GUIMain::setSingleEngine()
    {
        singleEngine = true; //Used to choose what to show/hide later if we change visibility
        stbdScrollbar->setVisible(false);
        stbdText->setVisible(false);

        //Get max extent of both engine scroll bars
        core::vector2d<s32> lowerRight = stbdScrollbar->getRelativePosition().LowerRightCorner;
        core::vector2d<s32> upperLeft = portScrollbar->getRelativePosition().UpperLeftCorner;
        portScrollbar->setRelativePosition(core::rect<s32>(upperLeft,lowerRight));

        //Change text from 'portEngine' to 'engine', and use all space
        portText->setText(language->translate("engine").c_str());
        portText->enableOverrideColor(false);
        lowerRight = stbdText->getRelativePosition().LowerRightCorner;
        upperLeft = portText->getRelativePosition().UpperLeftCorner;
        portText->setRelativePosition(core::rect<s32>(upperLeft,lowerRight));
    }

    void GUIMain::hideEngineAndRudder()
    {
        stbdScrollbar->setVisible(false);
        portScrollbar->setVisible(false);
        stbdText->setVisible(false);
        portText->setVisible(false);
        rudderScrollbar->setVisible(false);
    }

    void GUIMain::setInstruments(bool hasDepthSounder, irr::f32 maxSounderDepth, bool hasGPS)
    {
        this->hasDepthSounder = hasDepthSounder;
        this->maxSounderDepth = maxSounderDepth;
        this->hasGPS = hasGPS;
    }

    void GUIMain::updateVisibility()
    {
        //Items to show if we're showing interface
        radarTabControl->setVisible(showInterface);
        radarText->setVisible(showInterface);

        headingIndicator->setVisible(showInterface);
        dataDisplay->setVisible(showInterface);
        weatherScrollbar->setVisible(showInterface);
        rainScrollbar->setVisible(showInterface);
        visibilityScrollbar->setVisible(showInterface);
        pcLogButton->setVisible(showInterface);

        portText->setVisible(showInterface);
        stbdText->setVisible(showInterface && !singleEngine);

        //Items not to show if we're on full screen radar
        binosButton->setVisible(!radarLarge);
        bearingButton->setVisible(!radarLarge);
        hideInterfaceButton->setVisible(showInterface && !radarLarge);
        showInterfaceButton->setVisible(!showInterface && !radarLarge);

        bigRadarButton->setVisible(showInterface && !radarLarge);

        smallRadarButton->setVisible(radarLarge);
        largeRadarControls->setVisible(radarLarge);

        //Move gui elements if on largescreen radar
        if (!radarLarge) {
            headingIndicator->setRelativePosition(stdHdgIndicatorPos);
        } else {
            headingIndicator->setRelativePosition(altHdgIndicatorPos);
        }

    }

    std::wstring GUIMain::f32To1dp(irr::f32 value)
    {
        //Convert a floating point value to a wstring, with 1dp
        char tempStr[100];
        snprintf(tempStr,100,"%.1f",value);
        return std::wstring(tempStr, tempStr+strlen(tempStr));
    }

    std::wstring GUIMain::f32To2dp(irr::f32 value)
    {
        //Convert a floating point value to a wstring, with 2dp
        char tempStr[100];
        snprintf(tempStr,100,"%.2f",value);
        return std::wstring(tempStr, tempStr+strlen(tempStr));
    }

    std::wstring GUIMain::f32To3dp(irr::f32 value)
    {
        //Convert a floating point value to a wstring, with 3dp
        char tempStr[100];
        snprintf(tempStr,100,"%.3f",value);
        return std::wstring(tempStr, tempStr+strlen(tempStr));
    }

    bool GUIMain::manuallyTriggerClick(irr::gui::IGUIButton* button)
    {
        irr::SEvent triggerUpdateEvent;
        triggerUpdateEvent.EventType = EET_GUI_EVENT;
        triggerUpdateEvent.GUIEvent.Caller = button;
        triggerUpdateEvent.GUIEvent.Element = 0;
        triggerUpdateEvent.GUIEvent.EventType = irr::gui::EGET_BUTTON_CLICKED ;
        return device->postEventFromUser(triggerUpdateEvent);
    }

    void GUIMain::updateGuiData(irr::f32 lat, irr::f32 longitude, irr::f32 hdg, irr::f32 viewAngle, irr::f32 viewElevationAngle, irr::f32 spd, irr::f32 portEng, irr::f32 stbdEng, irr::f32 rudder, irr::f32 depth, irr::f32 weather, irr::f32 rain, irr::f32 visibility, irr::f32 radarRangeNm, irr::f32 radarGain, irr::f32 radarClutter, irr::f32 radarRain, irr::f32 guiRadarEBLBrg, irr::f32 guiRadarEBLRangeNm, std::string currentTime, bool paused, bool collided, bool headUp)
    {
        //Update scroll bars
        hdgScrollbar->setPos(Utilities::round(hdg));
        spdScrollbar->setPos(Utilities::round(spd));
        portScrollbar->setPos(Utilities::round(portEng * -100));//Engine units are +- 1, scale to -+100, inverted as astern is at bottom of scroll bar
        stbdScrollbar->setPos(Utilities::round(stbdEng * -100));
        rudderScrollbar->setPos(Utilities::round(rudder));
        radarGainScrollbar->setPos(Utilities::round(radarGain));
        radarClutterScrollbar->setPos(Utilities::round(radarClutter));
        radarRainScrollbar->setPos(Utilities::round(radarRain));

        radarGainScrollbar2->setPos(Utilities::round(radarGain));
        radarClutterScrollbar2->setPos(Utilities::round(radarClutter));
        radarRainScrollbar2->setPos(Utilities::round(radarRain));

        weatherScrollbar->setPos(Utilities::round(weather*10.0)); //(Weather scroll bar is 0-120, weather is 0-12)
        rainScrollbar->setPos(Utilities::round(rain*10.0)); //(Rain scroll bar is 0-100, rain is 0-10)
        visibilityScrollbar->setPos(Utilities::round(visibility*10.0)); //Visibility scroll bar is 1-101, visibility is 0.1 to 10.1 Nm
        //Update text display data
        guiLat = lat;
        guiLong = longitude;
        guiHeading = hdg; //Heading in degrees
        headingIndicator->setHeading(guiHeading);
        viewHdg = viewAngle+hdg;
        viewElev = viewElevationAngle;
        while (viewHdg>=360) {viewHdg-=360;}
        while (viewHdg<0) {viewHdg+=360;}
        guiSpeed = spd*MPS_TO_KTS; //Speed in knots
        guiDepth = depth;
        guiRadarRangeNm = radarRangeNm;
        guiTime = currentTime;
        guiPaused = paused;
        guiCollided = collided;

        radarHeadUp = headUp;

        //update EBL Data
        this->guiRadarEBLBrg = guiRadarEBLBrg;
        if (radarHeadUp) {
            this->guiRadarEBLBrg -= guiHeading;
        }
        this->guiRadarEBLRangeNm = guiRadarEBLRangeNm;
    }

    void GUIMain::showLogWindow()
    {
        gui::IGUIWindow* logWindow = guienv->addWindow(core::rect<s32>(0.01*su,0.01*sh,0.99*su,0.99*sh));
        gui::IGUIListBox* logText = guienv->addListBox(core::rect<s32>(0.03*su,0.05*sh,0.95*su,0.95*sh),logWindow);

        if (logWindow && logText && logMessages) {

            logText->setDrawBackground(true);

            for (unsigned int i = 0; i<logMessages->size(); i++) {
                std::string logTextString = logMessages->at(i);
                logText->addItem(core::stringw(logTextString.c_str()).c_str());
            }
        }

    }

    void GUIMain::drawGUI()
    {
        //Remove big paused button when the simulation is started.
        if (pausedButton) {
            if (!guiPaused) {
                pausedButton->remove();
                pausedButton = 0;
            }
            guiPaused = false;
        }

        //Convert lat/long into a readable format
        wchar_t eastWest;
        wchar_t northSouth;
        if (guiLat >= 0) {
            northSouth='N';
        } else {
            northSouth='S';
        }
        if (guiLong >= 0) {
            eastWest='E';
        } else {
            eastWest='W';
        }
        irr::f32 displayLat = fabs(guiLat);
        irr::f32 displayLong = fabs(guiLong);

        f32 latMinutes = (displayLat - (int)displayLat)*60;
        f32 lonMinutes = (displayLong - (int)displayLong)*60;
        u8 latDegrees = (int) displayLat;
        u8 lonDegrees = (int) displayLong;

        //update heading display element
        core::stringw displayText;

        if (hasGPS) {
            displayText.append(language->translate("pos"));
            displayText.append(irr::core::stringw(latDegrees));
            displayText.append(language->translate("deg"));
            displayText.append(f32To3dp(latMinutes).c_str());
            displayText.append(language->translate("minSymbol"));
            displayText.append(northSouth);
            displayText.append(L" ");

            displayText.append(irr::core::stringw(lonDegrees));
            displayText.append(language->translate("deg"));
            displayText.append(f32To3dp(lonMinutes).c_str());
            displayText.append(language->translate("minSymbol"));
            displayText.append(eastWest);
            displayText.append(L"\n");
        }

        displayText.append(language->translate("spd"));
        displayText.append(f32To1dp(guiSpeed).c_str());
        displayText.append(L"\n");

        if (hasDepthSounder) {
            displayText.append(language->translate("depth"));
            if (guiDepth <= maxSounderDepth) {
                displayText.append(f32To1dp(guiDepth).c_str());
            } else {
                displayText.append(L"-");
            }
            displayText.append(L"\n");
        }

        displayText.append(core::stringw(guiTime.c_str()));
        displayText.append(L"\n");

        displayText.append(language->translate("fps"));
        displayText.append(core::stringw(device->getVideoDriver()->getFPS()).c_str());
        displayText.append(L"\n");
        if (guiPaused) {
            displayText.append(language->translate("paused"));
            displayText.append(L"\n");
        }
        dataDisplay->setText(displayText.c_str());

        //add radar text (reuse the displayText)
        f32 displayEBLBearing = guiRadarEBLBrg;
        if (radarHeadUp) {
            displayEBLBearing += guiHeading;
        }
        while (displayEBLBearing>=360) {displayEBLBearing-=360;}
        while (displayEBLBearing<0) {displayEBLBearing+=360;}
        displayText = language->translate("range");
        displayText.append(f32To1dp(guiRadarRangeNm).c_str());
        displayText.append(language->translate("nm"));
        displayText.append(L"\n");
        displayText.append(language->translate("ebl"));
        displayText.append(f32To2dp(guiRadarEBLRangeNm).c_str());
        displayText.append(language->translate("nm"));
        displayText.append(L"/");
        displayText.append(f32To1dp(displayEBLBearing).c_str());
        displayText.append(language->translate("deg"));
        radarText ->setText(displayText.c_str());
        radarText2->setText(displayText.c_str());

        //add a collision warning
        if (guiCollided) {
            drawCollisionWarning();
        }

        //manually trigger gui event if buttons are held down
        if (eblUpButton->isPressed()) {manuallyTriggerClick(eblUpButton);}
        if (eblDownButton->isPressed()) {manuallyTriggerClick(eblDownButton);}
        if (eblLeftButton->isPressed()) {manuallyTriggerClick(eblLeftButton);}
        if (eblRightButton->isPressed()) {manuallyTriggerClick(eblRightButton);}

        if (eblUpButton2->isPressed()) {manuallyTriggerClick(eblUpButton2);}
        if (eblDownButton2->isPressed()) {manuallyTriggerClick(eblDownButton2);}
        if (eblLeftButton2->isPressed()) {manuallyTriggerClick(eblLeftButton2);}
        if (eblRightButton2->isPressed()) {manuallyTriggerClick(eblRightButton2);}

        guienv->drawAll();

        //draw the heading line on the radar
        if (showInterface || radarLarge) {
            draw2dRadar();
        }

        //draw view bearing if needed
        if (bearingButton->isPressed()){
            draw2dBearing();
        }
    }

    void GUIMain::draw2dRadar()
    {
        s32 centreX;
        s32 centreY;
        s32 radius;

        if (radarLarge) {
            centreX = largeRadarScreenCentreX;
            centreY = largeRadarScreenCentreY;
            radius = largeRadarScreenRadius;
        } else {
            centreX = smallRadarScreenCentreX;
            centreY = smallRadarScreenCentreY;
            radius = smallRadarScreenRadius;
        }

        //std::cout << radius*2 << std::endl;

        //If full screen radar, draw a 4:3 box around the radar display area
        if (radarLarge) {
            device->getVideoDriver()->draw2DRectangleOutline(radarLargeRect,video::SColor(255,0,0,0));
        }

        f32 radarHeadingIndicator;
        if (radarHeadUp) {
            radarHeadingIndicator = 0;
        } else {
            radarHeadingIndicator = guiHeading;
        }
        s32 deltaX = radius*sin(core::DEGTORAD*radarHeadingIndicator);
        s32 deltaY = -1*radius*cos(core::DEGTORAD*radarHeadingIndicator);
        core::position2d<s32> radarCentre (centreX,centreY);
        core::position2d<s32> radarHeading (centreX+deltaX,centreY+deltaY);
        device->getVideoDriver()->draw2DLine(radarCentre,radarHeading,video::SColor(255, 255, 255, 255)); //Todo: Make these configurable

        //draw a look direction line
        if (radarHeadUp) {
            radarHeadingIndicator = viewHdg - guiHeading;
        } else {
            radarHeadingIndicator = viewHdg;
        }
        s32 deltaXView = radius*sin(core::DEGTORAD*radarHeadingIndicator);
        s32 deltaYView = -1*radius*cos(core::DEGTORAD*radarHeadingIndicator);
        core::position2d<s32> lookInner (centreX + 0.9*deltaXView,centreY + 0.9*deltaYView);
        core::position2d<s32> lookOuter (centreX + deltaXView,centreY + deltaYView);
        device->getVideoDriver()->draw2DLine(lookInner,lookOuter,video::SColor(255, 255, 0, 0)); //Todo: Make these configurable

        //draw an EBL line
        s32 deltaXEBL = radius*sin(core::DEGTORAD*guiRadarEBLBrg);
        s32 deltaYEBL = -1*radius*cos(core::DEGTORAD*guiRadarEBLBrg);
        core::position2d<s32> eblOuter (centreX + deltaXEBL,centreY + deltaYEBL);
        device->getVideoDriver()->draw2DLine(radarCentre,eblOuter,video::SColor(255, 255, 0, 0));
        //draw EBL range
        if (guiRadarEBLRangeNm > 0 && guiRadarRangeNm >= guiRadarEBLRangeNm) {
            irr::f32 eblRangePx = radius*guiRadarEBLRangeNm/guiRadarRangeNm; //General Fixme: 0.2*sh for radar radius should be changed into a constant or similar
            irr::u8 noSegments = eblRangePx/2;
            if (noSegments < 10) {noSegments=10;}
            device->getVideoDriver()->draw2DPolygon(radarCentre,eblRangePx,video::SColor(255, 255, 0, 0),noSegments); //An n segment polygon, to approximate a circle
        }
    }

    void GUIMain::draw2dBearing()
    {

        //make cross hairs
        s32 screenCentreX = 0.5*su;
        s32 screenCentreY;
        if (showInterface) {
            screenCentreY = 0.3*sh;
        } else {
            screenCentreY = 0.5*sh;
        }
        s32 lineLength = 0.1*sh;
        core::position2d<s32> left(screenCentreX-lineLength,screenCentreY);
        core::position2d<s32> right(screenCentreX+lineLength,screenCentreY);
        core::position2d<s32> top(screenCentreX,screenCentreY-lineLength);
        core::position2d<s32> bottom(screenCentreX,screenCentreY+lineLength);
        core::position2d<s32> centre(screenCentreX,screenCentreY);
        device->getVideoDriver()->draw2DLine(left,right,video::SColor(255, 255, 0, 0));
        device->getVideoDriver()->draw2DLine(top,bottom,video::SColor(255, 255, 0, 0));

        //show view bearing
        guienv->getSkin()->getFont()->draw(f32To1dp(viewHdg).c_str(),core::rect<s32>(screenCentreX-lineLength,screenCentreY-lineLength,screenCentreX, screenCentreY), video::SColor(255,255,0,0),true,true);
        guienv->getSkin()->getFont()->draw(f32To1dp(viewElev).c_str(),core::rect<s32>(screenCentreX-lineLength,screenCentreY,screenCentreX, screenCentreY+lineLength), video::SColor(255,255,0,0),true,true);


        //show angle (from horizon)

    }

    void GUIMain::drawCollisionWarning()
    {
        s32 screenCentreX = 0.5*su;
        s32 screenCentreY = 0.3*su;

        device->getVideoDriver()->draw2DRectangle(video::SColor(255,255,255,255),core::rect<s32>(screenCentreX-0.25*su,screenCentreY-0.025*sh,screenCentreX+0.25*su, screenCentreY+0.025*sh));
        guienv->getSkin()->getFont()->draw(language->translate("collided"),
            core::rect<s32>(screenCentreX-0.25*su,screenCentreY-0.025*sh,screenCentreX+0.25*su, screenCentreY+0.025*sh),
            video::SColor(255,255,0,0),true,true);
    }

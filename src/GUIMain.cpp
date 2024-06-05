
/*
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

// DEE What does this do ... draws the GUI screen



#include "GUIMain.hpp"

#include "Constants.hpp"
#include "Utilities.hpp"
//#include "OutlineScrollBar.h"
#include "ScrollDial.h"
#include "AzimuthDial.h"

#include "SimulationModel.hpp"
#include "Lines.hpp"

#include <iostream> //for debugging
#include <cmath> //For fmod

//using namespace irr;

GUIMain::GUIMain()
{

}

void GUIMain::load(irr::IrrlichtDevice* device, Lang* language, std::vector<std::string>* logMessages, SimulationModel* model, bool singleEngine, bool azimuthDrive, bool controlsHidden, bool hasDepthSounder, irr::f32 maxSounderDepth, bool hasGPS, bool showTideHeight, bool hasBowThruster, bool hasSternThruster, bool hasRateOfTurnIndicator, bool showCollided, bool vr3dMode)
    {
        this->device = device;
        this->model = model;
        this->hasDepthSounder = hasDepthSounder;
        this->maxSounderDepth = maxSounderDepth;
        this->hasGPS = hasGPS;
        this->showTideHeight = showTideHeight;
        this->showCollided = showCollided;
        this->hasBowThruster = hasBowThruster;
        this->hasRateOfTurnIndicator = hasRateOfTurnIndicator;
        this->controlsHidden = controlsHidden;

        this->hasSternThruster = hasSternThruster;
        guienv = device->getGUIEnvironment();

        irr::video::IVideoDriver* driver = device->getVideoDriver();
        su = driver->getScreenSize().Width;
        sh = driver->getScreenSize().Height;

        this->language = language;
        this->logMessages = logMessages;

        //Set gui skin less transparent
        irr::video::SColor col = guienv->getSkin()->getColor(irr::gui::EGDC_3D_SHADOW);
        col.setAlpha(200);
        guienv->getSkin()->setColor(irr::gui::EGDC_3D_SHADOW, col);

        col = guienv->getSkin()->getColor(irr::gui::EGDC_3D_FACE);
        col.setAlpha(200);
        guienv->getSkin()->setColor(irr::gui::EGDC_3D_FACE, col);

        //default to double engine in gui
        this->singleEngine = singleEngine;

        //set if we have azimuth controls, instead of engine and rudder
        this->azimuthDrive = azimuthDrive;

        // GUI position modifications if in azimuth drive mode
        if (azimuthDrive) {
            //azimuthGUIOffset = 0.05*su;
            azimuthGUIOffsetL = -0.02*su;
            azimuthGUIOffsetR = -0.07*su;
        } else {
            azimuthGUIOffsetL = 0;
            azimuthGUIOffsetR = 0;
        }

        //Initial settings for NFU buttons
        nfuPortDown = false;
        nfuStbdDown = false;

        //Default to small radar display
        radarLarge = false;
        //Find available 4:3 rectangle to fit in area for large radar display
        irr::s32 availableWidth;
        irr::s32 availableHeight = (0.95-0.01)*sh;
        if (azimuthDrive) {
            // leave 0.9*su on both sides
            availableWidth  = (0.91-0.09)*su;
        } else {
            // leave 0.9*su on left, 0.01*su on right
            availableWidth  = (0.99-0.09)*su;
        }
        if (availableWidth/(float)availableHeight > 4.0/3.0) {
            // Wider than 4:3
            irr::s32 activeWidth = availableHeight * 4.0/3.0;
            irr::s32 activeHeight = availableHeight;
            radarLargeRect = irr::core::rect<irr::s32>(0.09*su + (availableWidth-activeWidth)/2, 0.01*sh, 0.09*su + activeWidth + (availableWidth-activeWidth)/2, 0.01+activeHeight);
        } else {
            // 4:3 or narrower
            irr::s32 activeWidth = availableWidth;
            irr::s32 activeHeight = availableWidth * 3.0/4.0;
            radarLargeRect = irr::core::rect<irr::s32>(0.09*su, 0.01*sh+(availableHeight-activeHeight)/2, 0.09*su + activeWidth, 0.01+activeHeight+(availableHeight-activeHeight)/2);
        }
        //For brevity, store large radar window width and top left corner.
        irr::s32 radarSu = radarLargeRect.getWidth();
        irr::core::vector2d<irr::s32> radarTL = radarLargeRect.UpperLeftCorner;
        //Find radar screen centre X, Y and radius
        largeRadarScreenRadius = (radarLargeRect.LowerRightCorner.Y-radarTL.Y)/2;
        largeRadarScreenCentreX = radarTL.X + largeRadarScreenRadius;
        largeRadarScreenCentreY = (radarLargeRect.LowerRightCorner.Y+radarTL.Y)/2;
        largeRadarScreenRadius*=0.95; //Make display slightly smaller, keeping the centre in the same place

        smallRadarScreenCentreX = su-0.2*sh+azimuthGUIOffsetR;
        smallRadarScreenCentreY = 0.8*sh;
        smallRadarScreenRadius=0.2*sh;

        //gui - add scroll bars for speed and heading control directly
        hdgScrollbar = new irr::gui::OutlineScrollBar(false,guienv,guienv->getRootGUIElement(),GUI_ID_HEADING_SCROLL_BAR,irr::core::rect<irr::s32>(0.01*su, 0.61*sh, 0.04*su, 0.99*sh));
        hdgScrollbar->setMax(360);
        spdScrollbar = new irr::gui::OutlineScrollBar(false,guienv,guienv->getRootGUIElement(),GUI_ID_SPEED_SCROLL_BAR,irr::core::rect<irr::s32>(0.05*su, 0.61*sh, 0.08*su, 0.99*sh));
        spdScrollbar->setMax(20.f*1852.f/3600.f); //20 knots in m/s
        //Hide speed/heading bars normally
        hdgScrollbar->setVisible(false);
        spdScrollbar->setVisible(false);

        //Add engine, rudder and thruster bars
        irr::core::array<irr::s32> rudderTics; rudderTics.push_back(-25);rudderTics.push_back(-20);rudderTics.push_back(-15);rudderTics.push_back(-10);rudderTics.push_back(-5);
        rudderTics.push_back(5);rudderTics.push_back(10);rudderTics.push_back(15);rudderTics.push_back(20);rudderTics.push_back(25);

        //Values to show on wheel control (should be same size as rudderTics, but we probably want to show an unsigned version in the GUI
        irr::core::array<irr::s32> rudderIndicatorTics; rudderIndicatorTics.push_back(25);rudderIndicatorTics.push_back(20);rudderIndicatorTics.push_back(15);rudderIndicatorTics.push_back(10);rudderIndicatorTics.push_back(5);
        rudderIndicatorTics.push_back(5);rudderIndicatorTics.push_back(10);rudderIndicatorTics.push_back(15);rudderIndicatorTics.push_back(20);rudderIndicatorTics.push_back(25);


        irr::core::array<irr::s32> engineTics; engineTics.push_back(-80);engineTics.push_back(-60);engineTics.push_back(-40);engineTics.push_back(-20);
        engineTics.push_back(20);engineTics.push_back(40);engineTics.push_back(60);engineTics.push_back(80);

        irr::core::array<irr::s32> centreTic; centreTic.push_back(0);

        if (hasBowThruster) {
            irr::f32 verticalScreenPos;
            if (hasSternThruster) {
                verticalScreenPos = 0.99-2*0.04;
            } else {
                verticalScreenPos = 0.99-1*0.04;
            }

// DEE bowthruster position
            bowThrusterScrollbar = new irr::gui::OutlineScrollBar(true,guienv,guienv->getRootGUIElement(),GUI_ID_BOWTHRUSTER_SCROLL_BAR,irr::core::rect<irr::s32>(0.01*su, verticalScreenPos*sh, 0.08*su, (verticalScreenPos+0.04)*sh),engineTics,centreTic);
            bowThrusterScrollbar->setMax(100);
            bowThrusterScrollbar->setMin(-100);
            bowThrusterScrollbar->setPos(0);
            bowThrusterScrollbar->setToolTipText(language->translate("bowThruster").c_str());
        } else {
            bowThrusterScrollbar = 0;
        }

        if (hasSternThruster) {
            irr::f32 verticalScreenPos = 0.99-1*0.04;
            sternThrusterScrollbar = new irr::gui::OutlineScrollBar(true,guienv,guienv->getRootGUIElement(),GUI_ID_STERNTHRUSTER_SCROLL_BAR,irr::core::rect<irr::s32>(0.01*su, verticalScreenPos*sh, 0.08*su, (verticalScreenPos+0.04)*sh),engineTics,centreTic);
            sternThrusterScrollbar->setMax(100);
            sternThrusterScrollbar->setMin(-100);
            sternThrusterScrollbar->setPos(0);
            sternThrusterScrollbar->setToolTipText(language->translate("sternThruster").c_str());
        } else {
            sternThrusterScrollbar = 0;
        }

        if (azimuthDrive) {
            // Azimuth drive
            portText = 0;
            portScrollbar = 0;
            stbdText = 0;
            stbdScrollbar = 0;
            wheelScrollbar = 0;
            nonFollowUpPortButton = 0;
            nonFollowUpStbdButton = 0;
            clickForRudderText = 0;
            clickForEngineText = 0;


	    // DEE_NOV22 defines new objects

	    //DEE_NOV22 comment below code by others.  they combine control and indication,  I'm just going to move them up a little to make room for other
	    //	    indicators.  I'd prefer it to be a simple thrust direction indicator to be honest, 1 is port 2 is stbd
//            azimuth1Control = new irr::gui::AzimuthDial(irr::core::vector2d<irr::s32>(0.035*su,0.8*sh),0.03*su,guienv,guienv->getRootGUIElement(),GUI_ID_AZIMUTH_1); 
//            azimuth2Control = new irr::gui::AzimuthDial(irr::core::vector2d<irr::s32>(0.105*su,0.8*sh),0.03*su,guienv,guienv->getRootGUIElement(),GUI_ID_AZIMUTH_2); 
        
            azimuth1Control = new irr::gui::AzimuthDial(irr::core::vector2d<irr::s32>(0.035*su,0.77*sh),0.04*sh,guienv,guienv->getRootGUIElement(),GUI_ID_AZIMUTH_2); 
            azimuth2Control = new irr::gui::AzimuthDial(irr::core::vector2d<irr::s32>(0.965*su,0.77*sh),0.04*sh,guienv,guienv->getRootGUIElement(),GUI_ID_AZIMUTH_2); 

            azimuth1Control->setMax(360); // DEE_NOV22 comment sets maximum value port azimuth indicator
            azimuth2Control->setMax(360); // DEE_NOV22 comment sets maximum value stbd azimuth indicator
	
// DEE_NOV22 vvvv change position of these
//            azimuth1Master = guienv->addCheckBox(false,irr::core::rect<irr::s32>(0.025*su,0.88*sh,0.045*su,0.90*sh),0,GUI_ID_AZIMUTH_1_MASTER_CHECKBOX);
//            azimuth2Master = guienv->addCheckBox(false,irr::core::rect<irr::s32>(0.095*su,0.88*sh,0.115*su,0.90*sh),0,GUI_ID_AZIMUTH_2_MASTER_CHECKBOX);

            azimuth1Master = guienv->addCheckBox(false,irr::core::rect<irr::s32>(0.025*su,0.82*sh,0.045*su,0.84*sh),0,GUI_ID_AZIMUTH_1_MASTER_CHECKBOX);
            azimuth2Master = guienv->addCheckBox(false,irr::core::rect<irr::s32>(0.955*su,0.82*sh,0.975*su,0.84*sh),0,GUI_ID_AZIMUTH_2_MASTER_CHECKBOX);

// DEE_NOV22 ^^^^

            azimuth1Master->setToolTipText(language->translate("azimuthMaster").c_str());
            azimuth2Master->setToolTipText(language->translate("azimuthMaster").c_str());

// DEE_NOV22 defines how and where indicators are displayed

	    // DEE_NOV22 the schottels ... the bottom most pair of dial

            schottelPort = new irr::gui::AzimuthDial(irr::core::vector2d<irr::s32>(0.035*su,0.89*sh),0.04*sh,guienv,guienv->getRootGUIElement(),GUI_ID_SCHOTTEL_PORT); // DEE_NOV22 visual representation of the physical schottel control todo in time, make it look like a schottel wheel
	    schottelPort->setToolTipText(language->translate("Schottel Port").c_str());
            schottelPort->setMax(360); // DEE_NOV22 sets maximum value port schottel

            schottelStbd = new irr::gui::AzimuthDial(irr::core::vector2d<irr::s32>(0.965*su,0.89*sh),0.04*sh,guienv,guienv->getRootGUIElement(),GUI_ID_SCHOTTEL_STBD); // DEE_NOV22 visual representation of the physical schottel control todo in time, make it look like a schottel wheel
	    schottelStbd->setToolTipText(language->translate("Schottel Starboard").c_str());
            schottelStbd->setMax(360); // DEE_NOV22 sets maximum value stbd schottel

	    // DEE_NOV22 added emergency steering checkox todo background code for this

	    emergencySteering = guienv->addCheckBox(false,irr::core::rect<irr::s32>(0.955*su,0.94*sh,0.975*su,0.96*sh),0,GUI_ID_EMERGENCY_STEERING);
	    emergencySteering->setToolTipText(language->translate("Emergency Steering").c_str());


	    // DEE_NOV22 the engine rpm indicators (0..1) the top most pair

            azimuthEnginePort = new irr::gui::AzimuthDial(irr::core::vector2d<irr::s32>(0.035*su,0.65*sh),0.04*sh,guienv,guienv->getRootGUIElement(),GUI_ID_AZIMUTH_ENGINE_PORT); // DEE_NOV22 visual representation of the port engine rpm as a proportion of max revs so 0..1, there is no reverse engine
	        azimuthEnginePort->setToolTipText(language->translate("Engine Port").c_str());
            azimuthEnginePort->setMax(360); // DEE_NOV22 sets maximum value port engine indicator

            azimuthEngineStbd = new irr::gui::AzimuthDial(irr::core::vector2d<irr::s32>(0.965*su,0.65*sh),0.04*sh,guienv,guienv->getRootGUIElement(),GUI_ID_AZIMUTH_ENGINE_STBD); // DEE_NOV22 visual representation of the starboard engine rpm as a proportion of max revs so 0..1, there is no reverse engine
	        azimuthEngineStbd->setToolTipText(language->translate("Engine Starboard").c_str());
            azimuthEngineStbd->setMax(360); // DEE_NOV22 sets maximum value stbd engine indicator

            azimuthClutchPort = guienv->addCheckBox(false,irr::core::rect<irr::s32>(0.025*su,0.70*sh,0.045*su,0.72*sh),0,GUI_ID_AZIMUTH_CLUTCH_PORT);
            azimuthClutchStbd = guienv->addCheckBox(false,irr::core::rect<irr::s32>(0.955*su,0.70*sh,0.975*su,0.72*sh),0,GUI_ID_AZIMUTH_CLUTCH_STBD);
            azimuthClutchPort->setToolTipText(language->translate("Port Clutch").c_str());
            azimuthClutchStbd->setToolTipText(language->translate("Starboard Clutch").c_str());



	    // DEE_NOV22 ^^^^

        } else {  // is Not azimuth drive
            azimuth1Control = 0;
            azimuth2Control = 0;
            azimuth1Master = 0;
            azimuth2Master = 0;
	    // DEE_NOV22 vvvv hide the azimuth drive controls
	    schottelPort = 0; 
	    schottelStbd = 0;
	    azimuthClutchPort = 0; // DEE_NOV22 not sure about this, I think perhaps clutch should be on some non azi engines like CPP vessels
	    azimuthClutchStbd = 0; // DEE_NOV22 as above
	    azimuthEnginePort = 0; 
	    azimuthEngineStbd = 0;
	    emergencySteering = 0; // though perhaps this would be useful for conventional ships too
            // DEE_NOV22 ^^^^


            portText = guienv->addStaticText(language->translate("portEngine").c_str(),irr::core::rect<irr::s32>(0.005*su, 0.61*sh, 0.045*su, 0.67*sh));
            portText->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
            portText->setOverrideColor(irr::video::SColor(255,128,0,0));
            portScrollbar = new irr::gui::OutlineScrollBar(false,guienv,guienv->getRootGUIElement(),GUI_ID_PORT_SCROLL_BAR,irr::core::rect<irr::s32>(0.01*su, 0.675*sh, 0.04*su, (0.99-0.04*hasBowThruster-0.04*hasSternThruster)*sh),engineTics,centreTic);
            portScrollbar->setMax(100);
            portScrollbar->setMin(-100);
            portScrollbar->setPos(0);
            stbdText = guienv->addStaticText(language->translate("stbdEngine").c_str(),irr::core::rect<irr::s32>(0.045*su, 0.61*sh, 0.085*su, 0.67*sh));
            stbdText->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
            stbdText->setOverrideColor(irr::video::SColor(255,0,128,0));
            stbdScrollbar = new irr::gui::OutlineScrollBar(false,guienv,guienv->getRootGUIElement(),GUI_ID_STBD_SCROLL_BAR,irr::core::rect<irr::s32>(0.05*su, 0.675*sh, 0.08*su, (0.99-0.04*hasBowThruster-0.04*hasSternThruster)*sh),engineTics,centreTic);
            stbdScrollbar->setMax(100);
            stbdScrollbar->setMin(-100);
            stbdScrollbar->setPos(0);

            wheelScrollbar = new irr::gui::OutlineScrollBar(true,guienv,guienv->getRootGUIElement(),GUI_ID_WHEEL_SCROLL_BAR,irr::core::rect<irr::s32>(0.13*su, 0.96*sh, 0.45*su, 0.99*sh),rudderTics,centreTic,true,rudderIndicatorTics);
            wheelScrollbar->setMax(30);
            wheelScrollbar->setMin(-30);
            wheelScrollbar->setPos(0);


            nonFollowUpPortButton = guienv->addButton(irr::core::rect<irr::s32>(0.09*su, 0.96*sh, 0.11*su, 0.99*sh),0,GUI_ID_NFU_PORT_BUTTON,language->translate("NFUPort").c_str());
            nonFollowUpStbdButton = guienv->addButton(irr::core::rect<irr::s32>(0.11*su, 0.96*sh, 0.13*su, 0.99*sh),0,GUI_ID_NFU_STBD_BUTTON,language->translate("NFUStbd").c_str());

            //Adapt if single engine:
            if (singleEngine) {
                stbdScrollbar->setVisible(false);
                stbdText->setVisible(false);

                //Get max extent of both engine scroll bars
                irr::core::vector2d<irr::s32> lowerRight = stbdScrollbar->getRelativePosition().LowerRightCorner;
                irr::core::vector2d<irr::s32> upperLeft = portScrollbar->getRelativePosition().UpperLeftCorner;
                portScrollbar->setRelativePosition(irr::core::rect<irr::s32>(upperLeft,lowerRight));

                //Change text from 'portEngine' to 'engine', and use all space
                portText->setText(language->translate("engine").c_str());
                portText->enableOverrideColor(false);
                lowerRight = stbdText->getRelativePosition().LowerRightCorner;
                upperLeft = portText->getRelativePosition().UpperLeftCorner;
                portText->setRelativePosition(irr::core::rect<irr::s32>(upperLeft,lowerRight));
            }

            //Add 'hint' text to click on the rudder and wheel controls
            clickForRudderText = guienv->addStaticText(language->translate("startupHelpRudder").c_str(),wheelScrollbar->getAbsolutePosition());
            clickForRudderText->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
            clickForRudderText->setOverrideColor(irr::video::SColor(255,255,0,0));

            irr::core::rect<irr::s32> engineHintPos = irr::core::rect<irr::s32>(
                portScrollbar->getRelativePosition().UpperLeftCorner,
                stbdScrollbar->getRelativePosition().LowerRightCorner);

            clickForEngineText = guienv->addStaticText(language->translate("startupHelpEngine").c_str(),engineHintPos);
            clickForEngineText->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
            clickForEngineText->setOverrideColor(irr::video::SColor(255,255,0,0));
        }

        //add data display:
        stdDataDisplayPos = irr::core::rect<irr::s32>(0.09*su+azimuthGUIOffsetL,0.71*sh,0.45*su+azimuthGUIOffsetR,0.95*sh); //In normal view
        radDataDisplayPos = irr::core::rect<irr::s32>(0.83*su,0.96*sh,0.99*su,0.99*sh); //In maximised 3d view
        altDataDisplayPos = irr::core::rect<irr::s32>(0.83*su,0.96*sh,0.99*su,0.99*sh); //In maximised 3d view
        dataDisplay = guienv->addStaticText(L"", stdDataDisplayPos, true, false, 0, -1, true); //Actual text set later
        stdDataDisplayBG = dataDisplay->getBackgroundColor();
        altDataDisplayBG = irr::video::SColor(200/4,255,255,255);
        radDataDisplayBG = irr::video::SColor(200/4,255,255,255);

        guiHeading = 0;
        guiSpeed = 0;

        //Add heading indicator
        stdHdgIndicatorPos = irr::core::rect<irr::s32>(0.09*su+azimuthGUIOffsetL,0.630*sh,0.45*su+azimuthGUIOffsetR,0.680*sh); //In normal view
        radHdgIndicatorPos = irr::core::rect<irr::s32>(0.46*su, 0.96*sh, 0.82*su, 0.99*sh); //In maximised radar view
        maxHdgIndicatorPos = irr::core::rect<irr::s32>(0.46*su, 0.96*sh, 0.82*su, 0.99*sh); //In maximised 3d view
        headingIndicator = new irr::gui::HeadingIndicator(guienv,guienv->getRootGUIElement(),stdHdgIndicatorPos);

        // DEE vvvvv add very basic rate of turn indicator
// rewrite this with its own class so that it is more realistic i.e. either a dial or a conning display

        rateofturnScrollbar = new irr::gui::OutlineScrollBar(true,guienv,guienv->getRootGUIElement(),GUI_ID_RATE_OF_TURN_SCROLL_BAR,irr::core::rect<irr::s32>(0.10*su+azimuthGUIOffsetL, 0.87*sh, 0.20*su+azimuthGUIOffsetL, 0.91*sh),rudderTics,centreTic);

        rateofturnScrollbar->setMax(50);
        rateofturnScrollbar->setMin(-50);
        rateofturnScrollbar->setSmallStep(1);
        rateofturnScrollbar->setPos(0);
        rateofturnScrollbar->setToolTipText(language->translate("rotText").c_str());
        if (!hasRateOfTurnIndicator) {
            rateofturnScrollbar->setVisible(false);
        }

// DEE ^^^^^

        // add indicators for whether the rudder pumps are working
        pump1On = guienv->addStaticText(language->translate("pump1").c_str(),irr::core::rect<irr::s32>(0.35*su+azimuthGUIOffsetR,0.72*sh,0.44*su+azimuthGUIOffsetR,0.745*sh),true,false,0,-1,true);
        pump2On = guienv->addStaticText(language->translate("pump2").c_str(),irr::core::rect<irr::s32>(0.35*su+azimuthGUIOffsetR,0.75*sh,0.44*su+azimuthGUIOffsetR,0.775*sh),true,false,0,-1,true);
        ackAlarms = guienv->addButton(irr::core::rect<irr::s32>(0.35*su+azimuthGUIOffsetR, 0.78*sh, 0.44*su+azimuthGUIOffsetR, 0.805*sh),0,GUI_ID_ACK_ALARMS_BUTTON,language->translate("ackAlarms").c_str());
        pump1On->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
        pump2On->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);

        //Add an additional window for controls (will normally be hidden)
        extraControlsWindow=guienv->addWindow(stdDataDisplayPos);
        extraControlsWindow->getCloseButton()->setVisible(false);
        extraControlsWindow->setText(language->translate("extraControls").c_str());
        guienv->addButton(extraControlsWindow->getCloseButton()->getRelativePosition(),extraControlsWindow,GUI_ID_HIDE_EXTRA_CONTROLS_BUTTON,L"X");
        extraControlsWindow->setVisible(false);

        //Add weather scroll bar
        //weatherScrollbar = guienv->addScrollBar(false,irr::core::rect<irr::s32>(0.417*su, 0.79*sh, 0.440*su, 0.94*sh), 0, GUI_ID_WEATHER_SCROLL_BAR);
        guienv->addStaticText(language->translate("weather").c_str(),irr::core::rect<irr::s32>(0.005*su,0.03*sh,0.055*su,0.06*sh),false,true,extraControlsWindow)->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
        weatherScrollbar = new irr::gui::ScrollDial(irr::core::vector2d<irr::s32>(0.03*su,0.09*sh),0.02*su,guienv,extraControlsWindow,GUI_ID_WEATHER_SCROLL_BAR);
        //weatherScrollbar = new irr::gui::AzimuthDial(irr::core::vector2d<irr::s32>(0.03*su,0.09*sh),0.02*su,guienv,extraControlsWindow,GUI_ID_WEATHER_SCROLL_BAR);
        weatherScrollbar->setMax(120); //Divide by 10 to get weather
        weatherScrollbar->setMin(0);
        weatherScrollbar->setSmallStep(5);
        weatherScrollbar->setToolTipText(language->translate("weather").c_str());


        //Add rain scroll bar
        //rainScrollbar = guienv->addScrollBar(false,irr::core::rect<irr::s32>(0.389*su, 0.79*sh, 0.412*su, 0.94*sh), 0, GUI_ID_RAIN_SCROLL_BAR);
        guienv->addStaticText(language->translate("rain").c_str(),irr::core::rect<irr::s32>(0.055*su,0.03*sh,0.105*su,0.06*sh),false,true,extraControlsWindow)->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
        rainScrollbar = new irr::gui::ScrollDial(irr::core::vector2d<irr::s32>(0.08*su,0.09*sh),0.02*su,guienv,extraControlsWindow,GUI_ID_RAIN_SCROLL_BAR);
        rainScrollbar->setMax(100);
        rainScrollbar->setMin(0);
        rainScrollbar->setLargeStep(5);
        rainScrollbar->setSmallStep(5);
        rainScrollbar->setToolTipText(language->translate("rain").c_str());

        //Add visibility scroll bar: Will be divided by 10 to get visibility in Nm
        //visibilityScrollbar = guienv->addScrollBar(false,irr::core::rect<irr::s32>(0.361*su, 0.79*sh, 0.384*su, 0.94*sh),0,GUI_ID_VISIBILITY_SCROLL_BAR);
        guienv->addStaticText(language->translate("visibility").c_str(),irr::core::rect<irr::s32>(0.105*su,0.03*sh,0.155*su,0.06*sh),false,true,extraControlsWindow)->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
        visibilityScrollbar = new irr::gui::ScrollDial(irr::core::vector2d<irr::s32>(0.13*su,0.09*sh),0.02*su,guienv,extraControlsWindow,GUI_ID_VISIBILITY_SCROLL_BAR);
        visibilityScrollbar->setMax(101);
        visibilityScrollbar->setMin(1);
        visibilityScrollbar->setLargeStep(5);
        visibilityScrollbar->setSmallStep(1);
        visibilityScrollbar->setToolTipText(language->translate("visibility").c_str());

        //Add buttons to control rudder failures etc.
        //Failure parts of GUI
        guienv->addButton(irr::core::rect<irr::s32>(0.16*su,0.03*sh,0.32*su,0.06*sh),extraControlsWindow,GUI_ID_RUDDERPUMP_1_WORKING_BUTTON,language->translate("pump1Working").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.16*su,0.06*sh,0.32*su,0.09*sh),extraControlsWindow,GUI_ID_RUDDERPUMP_1_FAILED_BUTTON,language->translate("pump1Failed").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.16*su,0.09*sh,0.32*su,0.12*sh),extraControlsWindow,GUI_ID_RUDDERPUMP_2_WORKING_BUTTON,language->translate("pump2Working").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.16*su,0.12*sh,0.32*su,0.15*sh),extraControlsWindow,GUI_ID_RUDDERPUMP_2_FAILED_BUTTON,language->translate("pump2Failed").c_str());

        guienv->addButton(irr::core::rect<irr::s32>(0.16*su,0.15*sh,0.32*su,0.18*sh),extraControlsWindow,GUI_ID_FOLLOWUP_WORKING_BUTTON,language->translate("followUpWorking").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.16*su,0.18*sh,0.32*su,0.21*sh),extraControlsWindow,GUI_ID_FOLLOWUP_FAILED_BUTTON,language->translate("followUpFailed").c_str());

        //Add an additional window for lines (will normally be hidden)
        linesControlsWindow=guienv->addWindow(irr::core::rect<irr::s32>(
            stdDataDisplayPos.UpperLeftCorner, 
            stdDataDisplayPos.LowerRightCorner - irr::core::position2d<irr::s32>(0,0.03*sh)
        ));
        linesControlsWindow->getCloseButton()->setVisible(false);
        linesControlsWindow->setText(language->translate("lines").c_str());
        guienv->addButton(linesControlsWindow->getCloseButton()->getRelativePosition(),linesControlsWindow,GUI_ID_HIDE_LINES_CONTROLS_BUTTON,L"X");
        linesControlsWindow->setVisible(false);

        //Lines controls interface
        addLine = guienv->addButton(irr::core::rect<irr::s32>(0.005*su,0.030*sh,0.121*su,0.080*sh),linesControlsWindow,GUI_ID_ADD_LINE_BUTTON,language->translate("addLine").c_str());
        linesList = guienv->addListBox(irr::core::rect<irr::s32>(0.005*su,0.090*sh,0.121*su,0.200*sh),linesControlsWindow,GUI_ID_LINES_LIST);
        
        removeLine = guienv->addButton(irr::core::rect<irr::s32>(0.122*su,0.090*sh,0.300*su,0.120*sh),linesControlsWindow,GUI_ID_REMOVE_LINE_BUTTON,language->translate("removeLine").c_str());
        
        keepLineSlack = guienv->addCheckBox(false,irr::core::rect<irr::s32>(0.280*su,0.130*sh,0.300*su,0.160*sh),linesControlsWindow,GUI_ID_KEEP_SLACK_LINE_CHECKBOX);
        irr::gui::IGUIStaticText* keepLineSlackText = guienv->addStaticText(language->translate("keepLineSlack").c_str(),irr::core::rect<irr::s32>(0.122*su,0.130*sh,0.275*su,0.160*sh),true,true,linesControlsWindow);
        keepLineSlackText->setTextAlignment(irr::gui::EGUIA_LOWERRIGHT, irr::gui::EGUIA_CENTER);
        
        heaveLineIn = guienv->addCheckBox(false,irr::core::rect<irr::s32>(0.280*su,0.170*sh,0.300*su,0.200*sh),linesControlsWindow,GUI_ID_HAUL_IN_LINE_CHECKBOX);
        irr::gui::IGUIStaticText* haulLineInText = guienv->addStaticText(language->translate("haulLineIn").c_str(),irr::core::rect<irr::s32>(0.122*su,0.170*sh,0.275*su,0.200*sh),true,true,linesControlsWindow);
        haulLineInText->setTextAlignment(irr::gui::EGUIA_LOWERRIGHT, irr::gui::EGUIA_CENTER);

        linesText = guienv->addStaticText(L"",irr::core::rect<irr::s32>(0.122*su,0.030*sh,0.300*su,0.080*sh),true,true,linesControlsWindow);
 
        //add radar buttons
        //add tab control for radar
        radarTabControl = guienv->addTabControl(irr::core::rect<irr::s32>(0.455*su+azimuthGUIOffsetR,0.695*sh,0.697*su+azimuthGUIOffsetR,0.990*sh),0,true);
        irr::gui::IGUITab* mainRadarTab = radarTabControl->addTab(language->translate("radarMainTab").c_str(),0);
        //irr::gui::IGUITab* radarEBLTab = radarTabControl->addTab(language->translate("radarEBLVRMTab").c_str(),0);
        irr::gui::IGUITab* radarPITab = radarTabControl->addTab(language->translate("radarPITab").c_str(),0);
        //irr::gui::IGUITab* radarGZoneTab = radarTabControl->addTab(language->translate("radarGuardZoneTab").c_str(),0);
        irr::gui::IGUITab* radarARPATab = radarTabControl->addTab(language->translate("radarARPATab").c_str(),0);
        //irr::gui::IGUITab* radarTrackTab = radarTabControl->addTab(language->translate("radarTrackTab").c_str(),0);
        //irr::gui::IGUITab* radarARPAVectorTab = radarTabControl->addTab(language->translate("radarARPAVectorTab").c_str(),0);
        //irr::gui::IGUITab* radarARPAAlarmTab = radarTabControl->addTab(language->translate("radarARPAAlarmTab").c_str(),0);
        //irr::gui::IGUITab* radarARPATrialTab = radarTabControl->addTab(language->translate("radarARPATrialTab").c_str(),0);

        radarText = guienv->addStaticText(L"",irr::core::rect<irr::s32>(0.460*su+azimuthGUIOffsetR,0.610*sh,0.690*su+azimuthGUIOffsetR,0.690*sh),true,true,0,-1,true);

		//Buttons for radar on/off
		radarOnOffButton = guienv->addButton(irr::core::rect<irr::s32>(0.005*su, 0.010*sh, 0.055*su, 0.040*sh), mainRadarTab, GUI_ID_RADAR_ONOFF_BUTTON, language->translate("onoff").c_str());
		//TODO: Complete this: To go where radar zoom + is, and squash these down a bit

        //Buttons for full or small radar
        bigRadarButton = guienv->addButton(irr::core::rect<irr::s32>(0.700*su+azimuthGUIOffsetR,0.610*sh,0.720*su+azimuthGUIOffsetR,0.640*sh),0,GUI_ID_BIG_RADAR_BUTTON,language->translate("bigRadar").c_str());
        irr::s32 smallRadarButtonLeft = radarTL.X + 0.01*su;
        irr::s32 smallRadarButtonTop = radarTL.Y + 0.01*sh;
        smallRadarButton = guienv->addButton(irr::core::rect<irr::s32>(smallRadarButtonLeft,smallRadarButtonTop,smallRadarButtonLeft+0.020*su,smallRadarButtonTop+0.030*sh),0,GUI_ID_SMALL_RADAR_BUTTON,language->translate("smallRadar").c_str());
        bigRadarButton->setToolTipText(language->translate("fullScreenRadar").c_str());
        smallRadarButton->setToolTipText(language->translate("minimiseRadar").c_str());

        // Radar cursor buttons
        radarCursorLeftButton = guienv->addButton(irr::core::rect<irr::s32>(0.700*su+azimuthGUIOffsetR,0.950*sh,0.715*su+azimuthGUIOffsetR,0.970*sh),0,GUI_ID_RADAR_DECREASE_X_BUTTON,L"<");
        radarCursorRightButton = guienv->addButton(irr::core::rect<irr::s32>(0.730*su+azimuthGUIOffsetR,0.950*sh,0.745*su+azimuthGUIOffsetR,0.970*sh),0,GUI_ID_RADAR_INCREASE_X_BUTTON,L">");
        radarCursorUpButton = guienv->addButton(irr::core::rect<irr::s32>(0.715*su+azimuthGUIOffsetR,0.930*sh,0.730*su+azimuthGUIOffsetR,0.950*sh),0,GUI_ID_RADAR_INCREASE_Y_BUTTON,L"^");
        radarCursorDownButton = guienv->addButton(irr::core::rect<irr::s32>(0.715*su+azimuthGUIOffsetR,0.970*sh,0.730*su+azimuthGUIOffsetR,0.990*sh),0,GUI_ID_RADAR_DECREASE_Y_BUTTON,L"v");

        guienv->addButton(irr::core::rect<irr::s32>(0.005*su,0.045*sh,0.055*su,0.085*sh),mainRadarTab,GUI_ID_RADAR_INCREASE_BUTTON,language->translate("increaserange").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.005*su,0.085*sh,0.055*su,0.125*sh),mainRadarTab,GUI_ID_RADAR_DECREASE_BUTTON,language->translate("decreaserange").c_str());

        guienv->addButton(irr::core::rect<irr::s32>(0.005*su,0.130*sh,0.055*su,0.160*sh),mainRadarTab,GUI_ID_RADAR_NORTH_BUTTON,language->translate("northUp").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.005*su,0.160*sh,0.055*su,0.190*sh),mainRadarTab,GUI_ID_RADAR_COURSE_BUTTON,language->translate("courseUp").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.005*su,0.190*sh,0.055*su,0.220*sh),mainRadarTab,GUI_ID_RADAR_HEAD_BUTTON,language->translate("headUp").c_str());

        //Controls for small radar window
        radarGainScrollbar    = new irr::gui::ScrollDial(irr::core::vector2d<irr::s32>(0.0850*su,0.040*sh),0.02*su,guienv,mainRadarTab,GUI_ID_RADAR_GAIN_SCROLL_BAR);
        radarClutterScrollbar = new irr::gui::ScrollDial(irr::core::vector2d<irr::s32>(0.1425*su,0.040*sh),0.02*su,guienv,mainRadarTab,GUI_ID_RADAR_CLUTTER_SCROLL_BAR);
        radarRainScrollbar    = new irr::gui::ScrollDial(irr::core::vector2d<irr::s32>(0.2000*su,0.040*sh),0.02*su,guienv,mainRadarTab,GUI_ID_RADAR_RAIN_SCROLL_BAR);
        (guienv->addStaticText(language->translate("gain").c_str(),irr::core::rect<irr::s32>(0.0600*su,0.070*sh,0.1100*su,0.100*sh),false,true,mainRadarTab))->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
        (guienv->addStaticText(language->translate("clutter").c_str(),irr::core::rect<irr::s32>(0.1165*su,0.070*sh,0.1675*su,0.100*sh),false,true,mainRadarTab))->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
        (guienv->addStaticText(language->translate("rain").c_str(),irr::core::rect<irr::s32>(0.1750*su,0.070*sh,0.2250*su,0.100*sh),false,true,mainRadarTab))->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
        radarGainScrollbar->setSmallStep(2);
        radarClutterScrollbar->setSmallStep(2);
        radarRainScrollbar->setSmallStep(2);

        eblLeftButton = guienv->addButton(irr::core::rect<irr::s32>(0.060*su,0.160*sh,0.115*su,0.190*sh),mainRadarTab,GUI_ID_RADAR_EBL_LEFT_BUTTON,language->translate("eblLeft").c_str());
        eblRightButton = guienv->addButton(irr::core::rect<irr::s32>(0.170*su,0.160*sh,0.225*su,0.190*sh),mainRadarTab,GUI_ID_RADAR_EBL_RIGHT_BUTTON,language->translate("eblRight").c_str());
        eblUpButton = guienv->addButton(irr::core::rect<irr::s32>(0.115*su,0.130*sh,0.170*su,0.160*sh),mainRadarTab,GUI_ID_RADAR_EBL_UP_BUTTON,language->translate("eblUp").c_str());
        eblDownButton = guienv->addButton(irr::core::rect<irr::s32>(0.115*su,0.190*sh,0.170*su,0.220*sh),mainRadarTab,GUI_ID_RADAR_EBL_DOWN_BUTTON,language->translate("eblDown").c_str());

        radarColourButton = guienv->addButton(irr::core::rect<irr::s32>(0.115*su,0.160*sh,0.170*su,0.190*sh),mainRadarTab,GUI_ID_RADAR_COLOUR_BUTTON,language->translate("radarColour").c_str());

        //Controls for large radar window
        largeRadarControls = new irr::gui::IGUIRectangle(guienv,guienv->getRootGUIElement(),irr::core::rect<irr::s32>(radarTL.X+0.770*radarSu,radarTL.Y+0.020*radarSu,radarTL.X+0.980*radarSu,radarTL.Y+0.730*radarSu));
        largeRadarPIControls = new irr::gui::IGUIRectangle(guienv,guienv->getRootGUIElement(),irr::core::rect<irr::s32>(radarTL.X+0.550*radarSu,radarTL.Y+0.020*radarSu,radarTL.X+0.770*radarSu,radarTL.Y+0.200*radarSu),false);
        radarGainScrollbar2    = new irr::gui::ScrollDial(irr::core::vector2d<irr::s32>(0.040*radarSu,0.040*radarSu),0.03*radarSu,guienv,largeRadarControls,GUI_ID_RADAR_GAIN_SCROLL_BAR);
        radarClutterScrollbar2 = new irr::gui::ScrollDial(irr::core::vector2d<irr::s32>(0.105*radarSu,0.040*radarSu),0.03*radarSu,guienv,largeRadarControls,GUI_ID_RADAR_CLUTTER_SCROLL_BAR);
        radarRainScrollbar2    = new irr::gui::ScrollDial(irr::core::vector2d<irr::s32>(0.170*radarSu,0.040*radarSu),0.03*radarSu,guienv,largeRadarControls,GUI_ID_RADAR_RAIN_SCROLL_BAR);

        radarGainScrollbar2->setSmallStep(2);
        radarClutterScrollbar2->setSmallStep(2);
        radarRainScrollbar2->setSmallStep(2);

        (guienv->addStaticText(language->translate("gain").c_str(),irr::core::rect<irr::s32>(0.010*radarSu,0.070*radarSu,0.070*radarSu,0.100*radarSu),false,true,largeRadarControls))->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
        (guienv->addStaticText(language->translate("clutter").c_str(),irr::core::rect<irr::s32>(0.075*radarSu,0.070*radarSu,0.135*radarSu,0.100*radarSu),false,true,largeRadarControls))->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
        (guienv->addStaticText(language->translate("rain").c_str(),irr::core::rect<irr::s32>(0.140*radarSu,0.070*radarSu,0.200*radarSu,0.100*radarSu),false,true,largeRadarControls))->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);

        guienv->addButton(irr::core::rect<irr::s32>(0.025*radarSu,0.110*radarSu,0.085*radarSu,0.160*radarSu),largeRadarControls,GUI_ID_RADAR_INCREASE_BUTTON,language->translate("increaserange").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.025*radarSu,0.165*radarSu,0.085*radarSu,0.210*radarSu),largeRadarControls,GUI_ID_RADAR_DECREASE_BUTTON,language->translate("decreaserange").c_str());

        guienv->addButton(irr::core::rect<irr::s32>(0.125*radarSu,0.110*radarSu,0.190*radarSu,0.140*radarSu),largeRadarControls,GUI_ID_RADAR_NORTH_BUTTON,language->translate("northUp").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.125*radarSu,0.145*radarSu,0.190*radarSu,0.175*radarSu),largeRadarControls,GUI_ID_RADAR_COURSE_BUTTON,language->translate("courseUp").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.125*radarSu,0.180*radarSu,0.190*radarSu,0.210*radarSu),largeRadarControls,GUI_ID_RADAR_HEAD_BUTTON,language->translate("headUp").c_str());

        eblLeftButton2 = guienv->addButton(irr::core::rect<irr::s32>(0.025*radarSu,0.245*radarSu,0.080*radarSu,0.275*radarSu),largeRadarControls,GUI_ID_RADAR_EBL_LEFT_BUTTON,language->translate("eblLeft").c_str());
        eblRightButton2 = guienv->addButton(irr::core::rect<irr::s32>(0.135*radarSu,0.245*radarSu,0.190*radarSu,0.275*radarSu),largeRadarControls,GUI_ID_RADAR_EBL_RIGHT_BUTTON,language->translate("eblRight").c_str());
        eblUpButton2 = guienv->addButton(irr::core::rect<irr::s32>(0.080*radarSu,0.215*radarSu,0.135*radarSu,0.245*radarSu),largeRadarControls,GUI_ID_RADAR_EBL_UP_BUTTON,language->translate("eblUp").c_str());
        eblDownButton2 = guienv->addButton(irr::core::rect<irr::s32>(0.080*radarSu,0.275*radarSu,0.135*radarSu,0.305*radarSu),largeRadarControls,GUI_ID_RADAR_EBL_DOWN_BUTTON,language->translate("eblDown").c_str());

        radarColourButton2 = guienv->addButton(irr::core::rect<irr::s32>(0.080*radarSu,0.245*radarSu,0.135*radarSu,0.275*radarSu),largeRadarControls,GUI_ID_RADAR_COLOUR_BUTTON,language->translate("radarColour").c_str());

        // Radar cursor buttons
        radarCursorLeftButton2 = guienv->addButton(irr::core::rect<irr::s32>(radarTL.X+0.670*radarSu,radarTL.Y+0.640*radarSu,radarTL.X+0.700*radarSu,radarTL.Y+0.670*radarSu),0,GUI_ID_RADAR_DECREASE_X_BUTTON,L"<");
        radarCursorRightButton2 = guienv->addButton(irr::core::rect<irr::s32>(radarTL.X+0.730*radarSu,radarTL.Y+0.640*radarSu,radarTL.X+0.760*radarSu,radarTL.Y+0.670*radarSu),0,GUI_ID_RADAR_INCREASE_X_BUTTON,L">");
        radarCursorUpButton2 = guienv->addButton(irr::core::rect<irr::s32>(radarTL.X+0.700*radarSu,radarTL.Y+0.610*radarSu,radarTL.X+0.730*radarSu,radarTL.Y+0.640*radarSu),0,GUI_ID_RADAR_INCREASE_Y_BUTTON,L"^");
        radarCursorDownButton2 = guienv->addButton(irr::core::rect<irr::s32>(radarTL.X+0.700*radarSu,radarTL.Y+0.670*radarSu,radarTL.X+0.730*radarSu,radarTL.Y+0.700*radarSu),0,GUI_ID_RADAR_DECREASE_Y_BUTTON,L"v");

        radarText2 = guienv->addStaticText(L"",irr::core::rect<irr::s32>(0.010*radarSu,0.310*radarSu,0.200*radarSu,0.400*radarSu),true,true,largeRadarControls,-1,true);

        //Radar PI tab
        //Drop down box to select PI 1-10
        (guienv->addStaticText(language->translate("parallelIndex").c_str(),irr::core::rect<irr::s32>(0.055*su,0.040*sh,0.205*su,0.080*sh),false,true,radarPITab))->setTextAlignment(irr::gui::EGUIA_UPPERLEFT,irr::gui::EGUIA_CENTER);
        irr::gui::IGUIComboBox* piSelected = guienv->addComboBox(irr::core::rect<irr::s32>(0.005*su,0.040*sh,0.050*su,0.080*sh),radarPITab,GUI_ID_PI_SELECT_BOX);
        piSelected->addItem(L"1");
        piSelected->addItem(L"2");
        piSelected->addItem(L"3");
        piSelected->addItem(L"4");
        piSelected->addItem(L"5");
        piSelected->addItem(L"6");
        piSelected->addItem(L"7");
        piSelected->addItem(L"8");
        piSelected->addItem(L"9");
        piSelected->addItem(L"10");
        //Edit boxes for bearing and range (+ve/-ve)
        (guienv->addStaticText(language->translate("piRange").c_str(),irr::core::rect<irr::s32>(0.055*su,0.100*sh,0.205*su,0.140*sh),false,true,radarPITab))->setTextAlignment(irr::gui::EGUIA_UPPERLEFT,irr::gui::EGUIA_CENTER);;
        guienv->addEditBox(L"0",irr::core::rect<irr::s32>(0.005*su,0.100*sh,0.050*su,0.140*sh),true,radarPITab,GUI_ID_PI_RANGE_BOX);
        (guienv->addStaticText(language->translate("piBearing").c_str(),irr::core::rect<irr::s32>(0.055*su,0.160*sh,0.205*su,0.200*sh),false,true,radarPITab))->setTextAlignment(irr::gui::EGUIA_UPPERLEFT,irr::gui::EGUIA_CENTER);;
        guienv->addEditBox(L"0",irr::core::rect<irr::s32>(0.005*su,0.160*sh,0.050*su,0.200*sh),true,radarPITab,GUI_ID_PI_BEARING_BOX);

        //PI on big radar screen
        (guienv->addStaticText(language->translate("parallelIndex").c_str(),irr::core::rect<irr::s32>(0.005*radarSu,0.010*radarSu,0.075*radarSu,0.070*radarSu),false,true,largeRadarPIControls))->setTextAlignment(irr::gui::EGUIA_LOWERRIGHT,irr::gui::EGUIA_UPPERLEFT);
        irr::gui::IGUIComboBox* piSelectedBig = guienv->addComboBox(irr::core::rect<irr::s32>(0.080*radarSu,0.010*radarSu,0.195*radarSu,0.035*radarSu),largeRadarPIControls,GUI_ID_BIG_PI_SELECT_BOX);
        piSelectedBig->addItem(L"1");
        piSelectedBig->addItem(L"2");
        piSelectedBig->addItem(L"3");
        piSelectedBig->addItem(L"4");
        piSelectedBig->addItem(L"5");
        piSelectedBig->addItem(L"6");
        piSelectedBig->addItem(L"7");
        piSelectedBig->addItem(L"8");
        piSelectedBig->addItem(L"9");
        piSelectedBig->addItem(L"10");

        guienv->addStaticText(language->translate("PIrange").c_str(),irr::core::rect<irr::s32>(0.130*radarSu,0.045*radarSu,0.215*radarSu,0.070*radarSu),false,false,largeRadarPIControls);
        guienv->addEditBox(L"0",irr::core::rect<irr::s32>(0.080*radarSu,0.045*radarSu,0.125*radarSu,0.070*radarSu),true,largeRadarPIControls,GUI_ID_BIG_PI_RANGE_BOX);

        guienv->addStaticText(language->translate("PIbearing").c_str(),irr::core::rect<irr::s32>(0.130*radarSu,0.080*radarSu,0.215*radarSu,0.105*radarSu),false,false,largeRadarPIControls);
        guienv->addEditBox(L"0",irr::core::rect<irr::s32>(0.080*radarSu,0.080*radarSu,0.125*radarSu,0.105*radarSu),true,largeRadarPIControls,GUI_ID_BIG_PI_BEARING_BOX);

        //Radar ARPA tab
        irr::gui::IGUIComboBox* arpaMode = guienv->addComboBox(irr::core::rect<irr::s32>(0.005*su,0.005*sh,0.150*su,0.035*sh),radarARPATab,GUI_ID_ARPA_ON_BOX);
        arpaMode->addItem(language->translate("arpaManual").c_str());
        arpaMode->addItem(language->translate("marpaOn").c_str());
        arpaMode->addItem(language->translate("arpaOn").c_str());
        irr::gui::IGUIComboBox* arpaVectorMode = guienv->addComboBox(irr::core::rect<irr::s32>(0.005*su,0.040*sh,0.150*su,0.070*sh),radarARPATab,GUI_ID_ARPA_TRUE_REL_BOX);
        arpaVectorMode->addItem(language->translate("trueArpa").c_str());
        arpaVectorMode->addItem(language->translate("relArpa").c_str());
        guienv->addEditBox(L"6",irr::core::rect<irr::s32>(0.155*su,0.040*sh,0.195*su,0.070*sh),true,radarARPATab,GUI_ID_ARPA_VECTOR_TIME_BOX);
        (guienv->addStaticText(language->translate("minsARPA").c_str(),irr::core::rect<irr::s32>(0.200*su,0.040*sh,0.237*su,0.070*sh),false,true,radarARPATab))->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
        arpaList = guienv->addListBox(irr::core::rect<irr::s32>(0.005*su,0.075*sh,0.121*su,0.190*sh),radarARPATab,GUI_ID_ARPA_LIST);
        arpaText = guienv->addListBox(irr::core::rect<irr::s32>(0.121*su,0.075*sh,0.237*su,0.190*sh),radarARPATab);
        // Manual/MARPA buttons
        (guienv->addStaticText(language->translate("manualOrMarpa").c_str(), irr::core::rect<irr::s32>(0.005*su,0.190*sh,0.237*su,0.215*sh), false, true, radarARPATab))->setTextAlignment(irr::gui::EGUIA_CENTER, irr::gui::EGUIA_CENTER);
        guienv->addButton(irr::core::rect<irr::s32>(0.005*su,0.215*sh,0.082*su,0.240*sh),radarARPATab,GUI_ID_MANUAL_NEW_BUTTON,language->translate("new").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.082*su,0.215*sh,0.159*su,0.240*sh),radarARPATab,GUI_ID_MANUAL_SCAN_BUTTON,language->translate("manualLog").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.159*su,0.215*sh,0.237*su,0.240*sh),radarARPATab,GUI_ID_MANUAL_CLEAR_BUTTON,language->translate("clear").c_str());

        //Radar ARPA on big radar screen
        arpaMode = guienv->addComboBox(irr::core::rect<irr::s32>(0.010*radarSu,0.405*radarSu,0.200*radarSu,0.435*radarSu),largeRadarControls,GUI_ID_BIG_ARPA_ON_BOX);
        arpaMode->addItem(language->translate("arpaManual").c_str());
        arpaMode->addItem(language->translate("marpaOn").c_str());
        arpaMode->addItem(language->translate("arpaOn").c_str());
        arpaVectorMode = guienv->addComboBox(irr::core::rect<irr::s32>(0.010*radarSu,0.440*radarSu,0.200*radarSu,0.470*radarSu),largeRadarControls,GUI_ID_BIG_ARPA_TRUE_REL_BOX);
        arpaVectorMode->addItem(language->translate("trueArpa").c_str());
        arpaVectorMode->addItem(language->translate("relArpa").c_str());
        guienv->addEditBox(L"6",irr::core::rect<irr::s32>(0.010*radarSu,0.480*radarSu,0.050*radarSu,0.510*radarSu),true,largeRadarControls,GUI_ID_BIG_ARPA_VECTOR_TIME_BOX);
        (guienv->addStaticText(language->translate("minsARPA").c_str(),irr::core::rect<irr::s32>(0.060*radarSu,0.480*radarSu,0.105*radarSu,0.510*radarSu),false,true,largeRadarControls))->setTextAlignment(irr::gui::EGUIA_CENTER,irr::gui::EGUIA_CENTER);
        arpaList2 = guienv->addListBox(irr::core::rect<irr::s32>(0.010*radarSu,0.515*radarSu,0.105*radarSu,0.655*radarSu),largeRadarControls,GUI_ID_BIG_ARPA_LIST);
        arpaText2 = guienv->addListBox(irr::core::rect<irr::s32>(0.105*radarSu,0.515*radarSu,0.200*radarSu,0.655*radarSu),largeRadarControls);
        // Manual/MARPA buttons
        (guienv->addStaticText(language->translate("manualOrMarpa").c_str(), irr::core::rect<irr::s32>(0.010*radarSu,0.655*radarSu,0.200*radarSu,0.675*radarSu), false, true, largeRadarControls))->setTextAlignment(irr::gui::EGUIA_CENTER, irr::gui::EGUIA_CENTER);
        guienv->addButton(irr::core::rect<irr::s32>(0.010*radarSu,0.675*radarSu,0.073*radarSu,0.695*radarSu),largeRadarControls,GUI_ID_MANUAL_NEW_BUTTON,language->translate("new").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.073*radarSu,0.675*radarSu,0.136*radarSu,0.695*radarSu),largeRadarControls,GUI_ID_MANUAL_SCAN_BUTTON,language->translate("manualLog").c_str());
        guienv->addButton(irr::core::rect<irr::s32>(0.136*radarSu,0.675*radarSu,0.200*radarSu,0.695*radarSu),largeRadarControls,GUI_ID_MANUAL_CLEAR_BUTTON,language->translate("clear").c_str());


        //Add paused button
        irr::core::stringw pausedButtonMessage = language->translate("pausedbutton");
        if (vr3dMode) {
            pausedButtonMessage = pausedButtonMessage + language->translate("vrpausedbutton");
        } else {
            pausedButtonMessage = pausedButtonMessage + language->translate("normalpausedbutton");
        }
        pausedButton = guienv->addButton(irr::core::rect<irr::s32>(0.2*su,0.1*sh,0.8*su,0.9*sh),0,GUI_ID_START_BUTTON, pausedButtonMessage.c_str());

        //show/hide interface
        showInterface = true; //If we start with the 2d interface shown
        showInterfaceButton = guienv->addButton(irr::core::rect<irr::s32>(0.09*su+azimuthGUIOffsetL,0.92*sh,0.13*su+azimuthGUIOffsetL,0.95*sh),0,GUI_ID_SHOW_INTERFACE_BUTTON,language->translate("showinterface").c_str());
        hideInterfaceButton = guienv->addButton(irr::core::rect<irr::s32>(0.09*su+azimuthGUIOffsetL,0.92*sh,0.13*su+azimuthGUIOffsetL,0.95*sh),0,GUI_ID_HIDE_INTERFACE_BUTTON,language->translate("hideinterface").c_str());
        showInterfaceButton->setVisible(false);

        //binoculars button
        binosButton = guienv->addButton(irr::core::rect<irr::s32>(0.13*su+azimuthGUIOffsetL,0.92*sh,0.17*su+azimuthGUIOffsetL,0.95*sh),0,GUI_ID_BINOS_INTERFACE_BUTTON,language->translate("zoom").c_str());
        binosButton->setIsPushButton(true);

        //Take bearing button
        bearingButton = guienv->addButton(irr::core::rect<irr::s32>(0.17*su+azimuthGUIOffsetL,0.92*sh,0.21*su+azimuthGUIOffsetL,0.95*sh),0,GUI_ID_BEARING_INTERFACE_BUTTON,language->translate("bearing").c_str());
        bearingButton->setIsPushButton(true);

        //Exit button
        exitButton = guienv->addButton(irr::core::rect<irr::s32>(0.21*su+azimuthGUIOffsetL,0.92*sh,0.25*su+azimuthGUIOffsetL,0.95*sh),0,GUI_ID_EXIT_BUTTON,language->translate("exit").c_str());

        //Show button to display extra controls window
        showExtraControlsButton = guienv->addButton(irr::core::rect<irr::s32>(0.25*su+azimuthGUIOffsetL,0.92*sh,0.33*su+azimuthGUIOffsetL,0.95*sh),0,GUI_ID_SHOW_EXTRA_CONTROLS_BUTTON,language->translate("extraControls").c_str());

        //Show button to display lines control window
        showLinesControlsButton = guienv->addButton(irr::core::rect<irr::s32>(0.33*su+azimuthGUIOffsetL,0.92*sh,0.37*su+azimuthGUIOffsetL,0.95*sh),0,GUI_ID_SHOW_LINES_CONTROLS_BUTTON,language->translate("lines").c_str());

        //Show internal log window button
        pcLogButton = guienv->addButton(irr::core::rect<irr::s32>(0.37*su+azimuthGUIOffsetL,0.92*sh,0.39*su+azimuthGUIOffsetL,0.95*sh),0,GUI_ID_SHOW_LOG_BUTTON,language->translate("log").c_str());
        
        //Set initial visibility
        updateVisibility();

    }

    GUIMain::~GUIMain()
    {
        //Drop scroll bars created with 'new'
        if (portScrollbar) {portScrollbar->drop();}
        if (stbdScrollbar) {stbdScrollbar->drop();}
        if (wheelScrollbar) {wheelScrollbar->drop();}

        if (rateofturnScrollbar) {rateofturnScrollbar->drop();}

        if (bowThrusterScrollbar) {bowThrusterScrollbar->drop();}
        if (sternThrusterScrollbar) {sternThrusterScrollbar->drop();}

        if (azimuth1Control) {azimuth1Control->drop();}
        if (azimuth2Control) {azimuth2Control->drop();}


	// DEE_NOV22 vvvv
	if (schottelPort) {schottelPort->drop();}
	if (schottelStbd) {schottelStbd->drop();}
	if (azimuthEnginePort) {azimuthEnginePort->drop();}
	if (azimuthEngineStbd) {azimuthEngineStbd->drop();}
	// DEE_NOV22 ^^^^

        weatherScrollbar->drop();
        visibilityScrollbar->drop();
        rainScrollbar->drop();

        radarGainScrollbar->drop();
        radarClutterScrollbar->drop();
        radarRainScrollbar->drop();

        radarGainScrollbar2->drop();
        radarClutterScrollbar2->drop();
        radarRainScrollbar2->drop();

        //largeRadarControls->drop();

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

    void GUIMain::showBearings()
    {
        bearingButton->setPressed(true);
    }

    void GUIMain::hideBearings()
    {
        bearingButton->setPressed(false);
    }

    void GUIMain::toggleBearings()
    {
        bearingButton->setPressed(!bearingButton->isPressed());
    }

    void GUIMain::zoomOn()
    {
        binosButton->setPressed(true);
    }

    void GUIMain::zoomOff()
    {
        binosButton->setPressed(false);
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

    void GUIMain::setARPAComboboxes(irr::s32 arpaState)
    {
        //Set both linked inputs - brute force
        irr::gui::IGUIElement* arpaCheckbox = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_ARPA_ON_BOX,true);
        if(arpaCheckbox!=0) {
            ((irr::gui::IGUIComboBox*)arpaCheckbox)->setSelected(arpaState);
        }
        arpaCheckbox = device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUIMain::GUI_ID_BIG_ARPA_ON_BOX,true);
        if(arpaCheckbox!=0) {
            ((irr::gui::IGUIComboBox*)arpaCheckbox)->setSelected(arpaState);
        }
    }

    void GUIMain::setARPAList(int arpaSelected)
    {
        //Set both linked inputs - brute force
        if(arpaList!=0) {
            arpaList->setSelected(arpaSelected);
        }
        
        if(arpaList2!=0) {
            arpaList2->setSelected(arpaSelected);
        }
    }

    irr::u32 GUIMain::getRadarPixelRadius() const
    {
        if (radarLarge) {
            return largeRadarScreenRadius;
        } else {
            return smallRadarScreenRadius;
        }
    }

    irr::core::vector2di GUIMain::getCursorPositionRadar() const
    {
        //Basic mouse position
        irr::core::vector2di cursorPosition = device->getCursorControl()->getPosition();

        //Radar screen centre position
        irr::core::vector2di radarScreenCentre;
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

    irr::core::rect<irr::s32> GUIMain::getSmallRadarRect() const
    {
	    irr::u32 graphicsWidth3d = su;
	    irr::u32 graphicsHeight3d = sh * VIEW_PROPORTION_3D;
        return irr::core::rect<irr::s32>(su-(sh-graphicsHeight3d)+azimuthGUIOffsetR,graphicsHeight3d,su+azimuthGUIOffsetR,sh);
    }
    
    irr::core::rect<irr::s32> GUIMain::getLargeRadarRect() const
    {
        return irr::core::rect<irr::s32>(largeRadarScreenCentreX - largeRadarScreenRadius, largeRadarScreenCentreY - largeRadarScreenRadius, largeRadarScreenCentreX + largeRadarScreenRadius, largeRadarScreenCentreY + largeRadarScreenRadius);
    }

    bool GUIMain::isNFUActive() const
    {
        return (nfuPortDown || nfuStbdDown);
    }

    void GUIMain::updateVisibility()
    {
        //Items to show if we're showing interface
        radarTabControl->setVisible(showInterface);
        radarText->setVisible(showInterface);

        radarCursorLeftButton->setVisible(showInterface && !radarLarge);
        radarCursorRightButton->setVisible(showInterface && !radarLarge);
        radarCursorUpButton->setVisible(showInterface && !radarLarge);
        radarCursorDownButton->setVisible(showInterface && !radarLarge);

        radarCursorLeftButton2->setVisible(radarLarge);
        radarCursorRightButton2->setVisible(radarLarge);
        radarCursorUpButton2->setVisible(radarLarge);
        radarCursorDownButton2->setVisible(radarLarge);

        //weatherScrollbar->setVisible(showInterface);
        //rainScrollbar->setVisible(showInterface);
        //visibilityScrollbar->setVisible(showInterface);
        pcLogButton->setVisible(showInterface);
        showExtraControlsButton->setVisible(showInterface);
        showLinesControlsButton->setVisible(showInterface);

        exitButton->setVisible(showInterface);

        if (portText) {portText->setVisible(showInterface);}
        if (stbdText) {stbdText->setVisible(showInterface && !singleEngine);}

        pump1On->setVisible(showInterface);
        pump2On->setVisible(showInterface);
        ackAlarms->setVisible(showInterface);

        //Items not to show if we're on full screen radar
        //dataDisplay->setVisible(!radarLarge);
        binosButton->setVisible(!radarLarge);
        bearingButton->setVisible(!radarLarge);
		rateofturnScrollbar->setVisible(!radarLarge && hasRateOfTurnIndicator);
        hideInterfaceButton->setVisible(showInterface && !radarLarge);
        showInterfaceButton->setVisible(!showInterface && !radarLarge);

        bigRadarButton->setVisible(showInterface && !radarLarge);

        smallRadarButton->setVisible(radarLarge);
        largeRadarControls->setVisible(radarLarge);
        largeRadarPIControls->setVisible(radarLarge);

        //Move gui elements if on largescreen radar
        //Heading
        if (radarLarge) {
            headingIndicator->setRelativePosition(radHdgIndicatorPos);
        } else if (!showInterface) {
            headingIndicator->setRelativePosition(maxHdgIndicatorPos);
        } else {
            headingIndicator->setRelativePosition(stdHdgIndicatorPos);
        }
        //Set position of data display
        if (radarLarge) {
            dataDisplay->setRelativePosition(radDataDisplayPos);
            dataDisplay->setBackgroundColor(radDataDisplayBG);
        } else if (!showInterface) {
            dataDisplay->setRelativePosition(altDataDisplayPos);
            dataDisplay->setBackgroundColor(altDataDisplayBG);
        } else {
            dataDisplay->setRelativePosition(stdDataDisplayPos);
            dataDisplay->setBackgroundColor(stdDataDisplayBG);
        }

        //If we're in secondary mode, make sure things are hidden if they shouldn't be shown on the secondary screen
        if (controlsHidden) {
            hideInSecondary();
        }

    }

    void GUIMain::hideInSecondary() {
        //Hide user inputs if in secondary mode
        if (stbdScrollbar) {stbdScrollbar->setVisible(false);}
        if (portScrollbar) {portScrollbar->setVisible(false);}
        if (azimuth1Control) {azimuth1Control->setVisible(false);}
        if (azimuth2Control) {azimuth2Control->setVisible(false);}
        if (azimuth1Master) {azimuth1Master->setVisible(false);}
        if (azimuth2Master) {azimuth2Master->setVisible(false);}

        if (showLinesControlsButton) {showLinesControlsButton->setVisible(false);}
        if (showExtraControlsButton) {showExtraControlsButton->setVisible(false);}

	// DEE_NOV22 vvvv hide these in secondary displays
        if (azimuthEnginePort) {azimuthEnginePort->setVisible(false);}
        if (azimuthEngineStbd) {azimuthEngineStbd->setVisible(false);}
        if (schottelPort) {schottelPort->setVisible(false);}
        if (schottelStbd) {schottelStbd->setVisible(false);}
        if (azimuthClutchPort) {azimuthClutchPort->setVisible(false);}
        if (azimuthClutchStbd) {azimuthClutchStbd->setVisible(false);}
        if (emergencySteering) {emergencySteering->setVisible(false);}

	// DEE_NOV22 ^^^^

        if (stbdText) {stbdText->setVisible(false);}
        if (portText) {portText->setVisible(false);}
        if (wheelScrollbar) {wheelScrollbar->setVisible(false);}
        if (nonFollowUpPortButton) {nonFollowUpPortButton->setVisible(false);}
        if (nonFollowUpStbdButton) {nonFollowUpStbdButton->setVisible(false);}
        //rateofturnScrollbar->setVisible(false); // hides rate of turn indicator in full screen
        if (bowThrusterScrollbar) {bowThrusterScrollbar->setVisible(false);}
        if (sternThrusterScrollbar) {sternThrusterScrollbar->setVisible(false);}
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
        triggerUpdateEvent.EventType = irr::EET_GUI_EVENT;
        triggerUpdateEvent.GUIEvent.Caller = button;
        triggerUpdateEvent.GUIEvent.Element = 0;
        triggerUpdateEvent.GUIEvent.EventType = irr::gui::EGET_BUTTON_CLICKED ;
        return device->postEventFromUser(triggerUpdateEvent);
    }

    bool GUIMain::manuallyTriggerScroll(irr::gui::IGUIScrollBar* bar)
    {
        irr::SEvent triggerUpdateEvent;
        triggerUpdateEvent.EventType = irr::EET_GUI_EVENT;
        triggerUpdateEvent.GUIEvent.Caller = bar;
        triggerUpdateEvent.GUIEvent.Element = 0;
        triggerUpdateEvent.GUIEvent.EventType = irr::gui::EGET_SCROLL_BAR_CHANGED ;
        return device->postEventFromUser(triggerUpdateEvent);
    }

    void GUIMain::updateGuiData(GUIData* guiData)
    {

        // TODO: Check the scroll bars exist!

        //Hide the 'hint' bars
        if (device->getTimer()->getTime()>3000) {
            if (clickForEngineText) {clickForEngineText->setVisible(false);}
            if (clickForRudderText) {clickForRudderText->setVisible(false);}
        }

        //Update scroll bars
        hdgScrollbar->setPos(Utilities::round(guiData->hdg));
        spdScrollbar->setPos(Utilities::round(guiData->spd));
        if (portScrollbar) {portScrollbar->setPos(Utilities::round(guiData->portEng * -100));}//Engine units are +- 1, scale to -+100, inverted as astern is at bottom of scroll bar
        if (stbdScrollbar) {stbdScrollbar->setPos(Utilities::round(guiData->stbdEng * -100));}

        if (azimuth1Control) {
	    // DEE_NOV22 should be read only with the needle showing direction only
//            azimuth1Control->setMag(Utilities::round(guiData->portEng * 100));
            azimuth1Control->setMag(Utilities::round(90));
            azimuth1Control->setPos(Utilities::round(guiData->portAzimuthAngle));
        }

        if (azimuth2Control) {
	    // DEE_NOV22 should be read only with the needle showing direction only
//            azimuth2Control->setMag(Utilities::round(guiData->stbdEng * 100));
            azimuth2Control->setMag(Utilities::round(90));
            azimuth2Control->setPos(Utilities::round(guiData->stbdAzimuthAngle));
        }

	// DEE_NOV22 vvvv sets the displayed data in the GUI it does not receive mouse clicks

	if (schottelPort)
	{ // refers to the GUI object schottelPort as opposed to the value
		schottelPort->setMag(Utilities::round(90)); // this is because I want a needle of fixed size
		schottelPort->setPos(Utilities::round(guiData->schottelPort));
	} // end if schottelPort 

	if (schottelStbd)
	{ // refers to the GUI object schottelPort as opposed to the value
		schottelStbd->setMag(Utilities::round(90)); // this is because I want a needle of fixed size
		schottelStbd->setPos(Utilities::round(guiData->schottelStbd));
	} // end if schottelStbd

	if (azimuthEnginePort)
	{ // refers to the GUI object that represents the engine rpm
		azimuthEnginePort->setMag(Utilities::round(90)); // fixed length needle
		azimuthEnginePort->setPos(Utilities::round(guiData->azimuthEnginePort));  // i think this has already been adjusted to be
									   // * 360, the engine proportion 0..1 
	}

	if (azimuthEngineStbd)
	{ // refers to the GUI object that represents the engine rpm
		azimuthEngineStbd->setMag(Utilities::round(90)); // fixed length needle
		azimuthEngineStbd->setPos(Utilities::round(guiData->azimuthEngineStbd));  // i think this has already been adjusted to be
									   // * 360, the engine proportion 0..1 
	}


	if(azimuthClutchPort) 
	{
		azimuthClutchPort->setChecked(guiData->azimuthClutchPort);
	}

	if(azimuthClutchStbd) 
	{
		azimuthClutchStbd->setChecked(guiData->azimuthClutchStbd);
	}

	// DEE_NOV22 ^^^^


// DEE_NOV22 would prefer to get rid of all this "master" business never seen it on a ship
//           as you can steer with one azi dead ahead, steering input with the other

        if (azimuth1Master) {
            azimuth1Master->setChecked(guiData->azimuth1Master);
        }

        if (azimuth2Master) {
            azimuth2Master->setChecked(guiData->azimuth2Master);
        }

        //rudderScrollbar->setPos(Utilities::round(guiData->rudder));
        if (wheelScrollbar) {
            wheelScrollbar->setSecondary(Utilities::round(guiData->rudder));
            wheelScrollbar->setPos(Utilities::round(guiData->wheel));
        }
        if (bowThrusterScrollbar) {bowThrusterScrollbar->setPos(Utilities::round(guiData->bowThruster * 100));}
        if (sternThrusterScrollbar) {sternThrusterScrollbar->setPos(Utilities::round(guiData->sternThruster * 100));}

        radarGainScrollbar->setPos(Utilities::round(guiData->radarGain));
        radarClutterScrollbar->setPos(Utilities::round(guiData->radarClutter));
        radarRainScrollbar->setPos(Utilities::round(guiData->radarRain));

        radarGainScrollbar2->setPos(Utilities::round(guiData->radarGain));
        radarClutterScrollbar2->setPos(Utilities::round(guiData->radarClutter));
        radarRainScrollbar2->setPos(Utilities::round(guiData->radarRain));

        weatherScrollbar->setPos(Utilities::round(guiData->weather*10.0)); //(Weather scroll bar is 0-120, weather is 0-12)
        rainScrollbar->setPos(Utilities::round(guiData->rain*10.0)); //(Rain scroll bar is 0-100, rain is 0-10)
        visibilityScrollbar->setPos(Utilities::round(guiData->visibility*10.0)); //Visibility scroll bar is 1-101, visibility is 0.1 to 10.1 Nm


// DEE vvvvv  this should display the rate of turn data on the screen
// DEE        since internalrate of turn is in rads per second then for deg per min x 3438
        rateofturnScrollbar->setPos(Utilities::round(3438*guiData->RateOfTurn));
// DEE ^^^^

        //Update text display data
        guiLat = guiData->lat;
        guiLong = guiData->longitude;
        guiHeading = guiData->hdg; //Heading in degrees
        headingIndicator->setHeading(guiHeading);
        viewHdg = guiData->viewAngle+guiData->hdg;
        viewElev = guiData->viewElevationAngle;
        while (viewHdg>=360) {viewHdg-=360;}
        while (viewHdg<0) {viewHdg+=360;}
        guiSpeed = guiData->spd*MPS_TO_KTS; //Speed in knots
        guiDepth = guiData->depth;
        guiRadarOn = guiData->radarOn;
        guiRadarRangeNm = guiData->radarRangeNm;
        guiTime = guiData->currentTime;
        guiPaused = guiData->paused;
        guiCollided = guiData->collided;
// DEE Feb 23 vvvv height of tide
guiTideHeight = guiData->tideHeight;


        radarHeadUp = guiData->headUp;

        //update EBL Data
        this->guiRadarEBLBrg = guiData->guiRadarEBLBrg;
        if (radarHeadUp) {
            this->guiRadarEBLBrg -= guiHeading;
        }
        this->guiRadarEBLRangeNm = guiData->guiRadarEBLRangeNm;

        //update cursor data
        this->guiRadarCursorBrg = guiData->guiRadarCursorBrg;
        if (radarHeadUp) {
            this->guiRadarCursorBrg -= guiHeading;
        }
        this->guiRadarCursorRangeNm = guiData->guiRadarCursorRangeNm;

        //Update ARPA data
        arpaContactStates = guiData->arpaContactStates;
        setARPAList(guiData->arpaListSelection);

        //Update rudder pump indicators
        if (guiData->pump1On == true) {
            pump1On->setBackgroundColor(irr::video::SColor(255,0,128,0));
        } else {
            pump1On->setBackgroundColor(irr::video::SColor(255,128,0,0));
        }
        if (guiData->pump2On == true) {
            pump2On->setBackgroundColor(irr::video::SColor(255,0,128,0));
        } else {
            pump2On->setBackgroundColor(irr::video::SColor(255,128,0,0));
        }
    }

    void GUIMain::showLogWindow()
    {

    	irr::gui::IGUIWindow* logWindow = guienv->addWindow(irr::core::rect<irr::s32>(0.01*su,0.01*sh,0.99*su,0.99*sh));
    	irr::gui::IGUIListBox* logText = guienv->addListBox(irr::core::rect<irr::s32>(0.03*su,0.05*sh,0.95*su,0.95*sh),logWindow);

        if (logWindow && logText && logMessages) {

            logText->setDrawBackground(true);
            logText->clear();

            for (unsigned int i = 0; i<logMessages->size(); i++) {
                std::string logTextString = logMessages->at(i);
                logText->addItem(irr::core::stringw(logTextString.c_str()).c_str());
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

        irr::f32 latMinutes = (displayLat - (int)displayLat)*60;
        irr::f32 lonMinutes = (displayLong - (int)displayLong)*60;
        irr::u8 latDegrees = (int) displayLat;
        irr::u8 lonDegrees = (int) displayLong;

        //update heading display element
        irr::core::stringw displayText;

        displayText.append(language->translate("spd"));
        displayText.append(f32To1dp(guiSpeed).c_str());
        displayText.append(L" ");
        displayText.append(language->translate("kts"));
        displayText.append(L" ");

        if (showInterface) { //Only show speed in minimal 2d interface
            displayText.append(L"\n");
            if (hasDepthSounder) {
                displayText.append(language->translate("depth"));
                if (guiDepth <= maxSounderDepth) {
                    displayText.append(f32To1dp(guiDepth).c_str());
                } else {
                    displayText.append(L"-");
                }
                displayText.append(L" m \n");
            }

            displayText.append(irr::core::stringw(guiTime.c_str()));
            displayText.append(L"\n");

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

            displayText.append(language->translate("fps"));
            displayText.append(irr::core::stringw(device->getVideoDriver()->getFPS()).c_str());
            displayText.append(L"\n");

	        if (showTideHeight) {
                // DEE FEB 23 vvv add height of tide to the display
                displayText.append(language->translate("hot"));
                displayText.append(f32To1dp(guiTideHeight).c_str());
                displayText.append(L"\n");
                // DEE FEB 23 ^^^
            }
	    
        }
        if (guiPaused) {
            displayText.append(language->translate("paused"));
            displayText.append(L"\n");
        }
        dataDisplay->setText(displayText.c_str());

        //add radar text (reuse the displayText)
        irr::f32 displayEBLBearing = guiRadarEBLBrg;
        irr::f32 displayCursorBearing = guiRadarCursorBrg;
        if (radarHeadUp) {
            displayEBLBearing += guiHeading;
            displayCursorBearing += guiHeading;
        }
        while (displayEBLBearing>=360) {displayEBLBearing-=360;}
        while (displayEBLBearing<0) {displayEBLBearing+=360;}
        while (displayCursorBearing>=360) {displayCursorBearing-=360;}
        while (displayCursorBearing<0) {displayCursorBearing+=360;}

        displayText = language->translate("range");
        displayText.append(f32To1dp(guiRadarRangeNm).c_str());
        displayText.append(language->translate("nm"));
        displayText.append(L"\n");

        displayText.append(language->translate("vrm"));
        displayText.append(f32To2dp(guiRadarEBLRangeNm).c_str());
        displayText.append(language->translate("nm"));
        if (guiRadarCursorRangeNm > 0){
            displayText.append(" ");
            displayText.append(language->translate("cursor"));
            displayText.append(f32To2dp(guiRadarCursorRangeNm).c_str());
            displayText.append(language->translate("nm"));
        }
        displayText.append(L"\n");

        displayText.append(language->translate("ebl"));
        displayText.append(f32To1dp(displayEBLBearing).c_str());
        displayText.append(language->translate("deg"));
        if (guiRadarCursorRangeNm > 0){
            displayText.append(" ");
            displayText.append(language->translate("cursor"));
            displayText.append(f32To2dp(displayCursorBearing).c_str());
            displayText.append(language->translate("deg"));
        }
        radarText ->setText(displayText.c_str());
        radarText2->setText(displayText.c_str());

        //Use guiCPAs and guiTCPAs to display ARPA data
        //Todo: Store current position and reset here
        irr::s32 selectedItem = arpaList->getSelected();
        irr::s32 selectedItem2 = arpaList2->getSelected();
        irr::s32 selectedPosition = 0;
        irr::s32 selectedPosition2 =0;
        if (arpaList->getVerticalScrollBar()) {selectedPosition=arpaList->getVerticalScrollBar()->getPos();}
        if (arpaList2->getVerticalScrollBar()) {selectedPosition2=arpaList2->getVerticalScrollBar()->getPos();}
        arpaList->clear();
        arpaList2->clear();
        arpaText->clear();
        arpaText2->clear();

        //if (guiCPAs.size() == guiTCPAs.size() && guiCPAs.size() == guiARPAspeeds.size() && guiCPAs.size() == guiARPAheadings.size()) {
        for (unsigned int i = 0; i < arpaContactStates.size(); i++) {

            //Convert TCPA from decimal minutes into minutes and seconds.
            //TODO: Filter list based on risk?

            // If stationary, show placeholder only
            if (arpaContactStates.at(i).stationary) {
                displayText = language->translate("untracked");
                arpaList->addItem(displayText.c_str());
                arpaList2->addItem(displayText.c_str());
                continue;
            }

            displayText = L"";

            irr::f32 tcpa = arpaContactStates.at(i).tcpa;
            irr::f32 cpa  = arpaContactStates.at(i).cpa;
            irr::u32 arpahdg = round(arpaContactStates.at(i).absHeading);
            irr::u32 arpaspd = round(arpaContactStates.at(i).speed);

            irr::u32 tcpaMins = floor(tcpa);
            irr::u32 tcpaSecs = floor(60*(tcpa - tcpaMins));

            irr::core::stringw tcpaDisplayMins = irr::core::stringw(tcpaMins);
            if (tcpaDisplayMins.size() == 1) {
                irr::core::stringw zeroPadded = L"0";
                zeroPadded.append(tcpaDisplayMins);
                tcpaDisplayMins = zeroPadded;
            }

            irr::core::stringw tcpaDisplaySecs = irr::core::stringw(tcpaSecs);
            if (tcpaDisplaySecs.size() == 1) {
                irr::core::stringw zeroPadded = L"0";
                zeroPadded.append(tcpaDisplaySecs);
                tcpaDisplaySecs = zeroPadded;
            }

            if (arpaContactStates.at(i).contactType == CONTACT_MANUAL) {
                displayText.append(language->translate("manualContact"));
            } else {
                displayText.append(language->translate("arpaContact"));
            }
            displayText.append(L" ");
            displayText.append(irr::core::stringw(i+1)); //Contact ID (1,2,...)
            displayText.append(L":");

            arpaList->addItem(displayText.c_str());
            arpaList2->addItem(displayText.c_str());

            if ( i==selectedItem || i==selectedItem2 ) {
                //Show arpa details

                if (arpaContactStates.at(i).range == 0) {
                    // Exactly 0 means untracked
                    displayText = language->translate("untracked");
                    if (i==selectedItem) {
                        arpaText->addItem(displayText.c_str());
                    }
                    if (i==selectedItem2) {
                        arpaText2->addItem(displayText.c_str());
                    }
                } else {
                    //CPA
                    displayText = L"";
                    displayText.append(language->translate("cpa"));
                    displayText.append(L":");
                    displayText.append(f32To2dp(cpa).c_str());
                    displayText.append(language->translate("nm"));
                    //Add to the correct box
                    if (i==selectedItem) {
                        arpaText->addItem(displayText.c_str());
                    }
                    if (i==selectedItem2) {
                        arpaText2->addItem(displayText.c_str());
                    }

                    //TCPA
                    displayText = L"";
                    displayText.append(language->translate("tcpa"));
                    displayText.append(L":");
                    if (tcpa >= 0) {
                        displayText.append(tcpaDisplayMins);
                        displayText.append(L":");
                        displayText.append(tcpaDisplaySecs);
                    } else {
                        displayText.append(L" ");
                        displayText.append(language->translate("past"));
                    }
                    //Add to the correct box
                    if (i==selectedItem) {
                        arpaText->addItem(displayText.c_str());
                    }
                    if (i==selectedItem2) {
                        arpaText2->addItem(displayText.c_str());
                    }

                    //Heading and speed
                    //Pad heading to three decimals
                    irr::core::stringw headingText = irr::core::stringw(arpahdg);
                    if (headingText.size() == 1) {
                        irr::core::stringw zeroPadded = L"00";
                        zeroPadded.append(headingText);
                        headingText = zeroPadded;
                    }
                    else if (headingText.size() == 2) {
                        irr::core::stringw zeroPadded = L"0";
                        zeroPadded.append(headingText);
                        headingText = zeroPadded;
                    }
                    displayText = L"";
                    displayText.append(headingText);
                    displayText.append(L"° ");
                    displayText.append(irr::core::stringw(arpaspd));
                    displayText.append(L" kts");
                    //Add to the correct box
                    if (i==selectedItem) {
                        arpaText->addItem(displayText.c_str());
                    }
                    if (i==selectedItem2) {
                        arpaText2->addItem(displayText.c_str());
                    }
                }

            }

        }
        //}
        if (selectedItem > -1 && (irr::s32)arpaList->getItemCount()>selectedItem) {
            arpaList->setSelected(selectedItem);
        }
        if (selectedItem2 > -1 && (irr::s32)arpaList2->getItemCount()>selectedItem2) {
            arpaList2->setSelected(selectedItem2);
        }
        if(arpaList->getVerticalScrollBar()) {
            arpaList->getVerticalScrollBar()->setPos(selectedPosition);
        }
        if(arpaList2->getVerticalScrollBar()) {
            arpaList2->getVerticalScrollBar()->setPos(selectedPosition2);
        }


        //add a collision warning
        if (guiCollided && showCollided) {
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

        if (radarCursorLeftButton->isPressed()) {manuallyTriggerClick(radarCursorLeftButton);}
        if (radarCursorRightButton->isPressed()) {manuallyTriggerClick(radarCursorRightButton);}
        if (radarCursorUpButton->isPressed()) {manuallyTriggerClick(radarCursorUpButton);}
        if (radarCursorDownButton->isPressed()) {manuallyTriggerClick(radarCursorDownButton);}

        if (radarCursorLeftButton2->isPressed()) {manuallyTriggerClick(radarCursorLeftButton2);}
        if (radarCursorRightButton2->isPressed()) {manuallyTriggerClick(radarCursorRightButton2);}
        if (radarCursorUpButton2->isPressed()) {manuallyTriggerClick(radarCursorUpButton2);}
        if (radarCursorDownButton2->isPressed()) {manuallyTriggerClick(radarCursorDownButton2);}

        if (nonFollowUpPortButton && wheelScrollbar ) {
            //Handle port NFU rudder button
            if (nonFollowUpPortButton->isPressed() && !nfuPortDown) {
                nfuPortDown = true; //Set this before we trigger the event, as this will be checked for override
                wheelScrollbar->setPos(-30);
                manuallyTriggerScroll(wheelScrollbar);
            }
            if (!nonFollowUpPortButton->isPressed() && nfuPortDown) {
                wheelScrollbar->setPos(wheelScrollbar->getSecondary());
                manuallyTriggerScroll(wheelScrollbar);
                nfuPortDown = false; //Set this after we trigger the event, as this will be checked for override
            }
        }

        if (nonFollowUpStbdButton && wheelScrollbar) {
            //Handle stbd NFU rudder button
            if (nonFollowUpStbdButton->isPressed() && !nfuStbdDown) {
                nfuStbdDown = true; //Set this before we trigger the event, as this will be checked for override
                wheelScrollbar->setPos(30);
                manuallyTriggerScroll(wheelScrollbar);
            }
            if (!nonFollowUpStbdButton->isPressed() && nfuStbdDown) {
                wheelScrollbar->setPos(wheelScrollbar->getSecondary());
                manuallyTriggerScroll(wheelScrollbar);
                nfuStbdDown = false; //Set this after we trigger the event, as this will be checked for override
            }
        }

        // Update lines display
        if (model && model->getLines()) {
            std::vector<std::string> linesNames = model->getLines()->getLineNames();
            
            irr::s32 previousSelection = linesList->getSelected();
            linesList->clear();

            for (unsigned int i = 0; i < linesNames.size(); i++) {
                linesList->addItem(irr::core::stringw(linesNames.at(i).c_str()).c_str());
            }

            if (linesList->getItemCount() > previousSelection) {
                linesList->setSelected(previousSelection);
            }

            // Get 'keepSlack' & 'heaveIn' status of current line
            if (linesList->getSelected() > -1) {
                keepLineSlack->setChecked(model->getLines()->getKeepSlack(linesList->getSelected()));
                heaveLineIn->setChecked(model->getLines()->getHeaveIn(linesList->getSelected()));
            } else {
                keepLineSlack->setChecked(false);
                heaveLineIn->setChecked(false);
            }


        }

        guienv->drawAll();

        //draw the heading line on the radar
        if (showInterface || radarLarge) {
            if (guiRadarOn) {
                draw2dRadar();
            }
        }

        //draw view bearing if needed
        if (bearingButton->isPressed()){
            draw2dBearing();
        }
    }

    void GUIMain::draw2dRadar()
    {
        irr::s32 centreX;
        irr::s32 centreY;
        irr::s32 radius;

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
            device->getVideoDriver()->draw2DRectangleOutline(radarLargeRect,irr::video::SColor(255,0,0,0));
        }

        irr::f32 radarHeadingIndicator;
        if (radarHeadUp) {
            radarHeadingIndicator = 0;
        } else {
            radarHeadingIndicator = guiHeading;
        }
        irr::s32 deltaX = radius*sin(irr::core::DEGTORAD*radarHeadingIndicator);
        irr::s32 deltaY = -1*radius*cos(irr::core::DEGTORAD*radarHeadingIndicator);
        irr::core::position2d<irr::s32> radarCentre (centreX,centreY);
        irr::core::position2d<irr::s32> radarHeading (centreX+deltaX,centreY+deltaY);
        device->getVideoDriver()->draw2DLine(radarCentre,radarHeading,irr::video::SColor(255, 255, 255, 255)); //Todo: Make these colours configurable

        //draw a look direction line
        if (radarHeadUp) {
            radarHeadingIndicator = viewHdg - guiHeading;
        } else {
            radarHeadingIndicator = viewHdg;
        }
        irr::s32 deltaXView = radius*sin(irr::core::DEGTORAD*radarHeadingIndicator);
        irr::s32 deltaYView = -1*radius*cos(irr::core::DEGTORAD*radarHeadingIndicator);
        irr::core::position2d<irr::s32> lookInner (centreX + 0.9*deltaXView,centreY + 0.9*deltaYView);
        irr::core::position2d<irr::s32> lookOuter (centreX + deltaXView,centreY + deltaYView);
        device->getVideoDriver()->draw2DLine(lookInner,lookOuter,irr::video::SColor(255, 255, 0, 0)); //Todo: Make these colours configurable

        //draw an EBL line
        irr::s32 deltaXEBL = radius*sin(irr::core::DEGTORAD*guiRadarEBLBrg);
        irr::s32 deltaYEBL = -1*radius*cos(irr::core::DEGTORAD*guiRadarEBLBrg);
        irr::core::position2d<irr::s32> eblOuter (centreX + deltaXEBL,centreY + deltaYEBL);
        device->getVideoDriver()->draw2DLine(radarCentre,eblOuter,irr::video::SColor(255, 255, 0, 0));
        //draw EBL range
        if (guiRadarEBLRangeNm > 0 && guiRadarRangeNm >= guiRadarEBLRangeNm) {
            irr::f32 eblRangePx = radius*guiRadarEBLRangeNm/guiRadarRangeNm;
            irr::u8 noSegments = eblRangePx/2;
            if (noSegments < 10) {noSegments=10;}
            device->getVideoDriver()->draw2DPolygon(radarCentre,eblRangePx,irr::video::SColor(255, 255, 0, 0),noSegments); //An n segment polygon, to approximate a circle
        }

        //draw radar cursor
        irr::s32 cursorPixelRadius = radius*guiRadarCursorRangeNm/guiRadarRangeNm;
        irr::s32 deltaXCursor = cursorPixelRadius*sin(irr::core::DEGTORAD*guiRadarCursorBrg);
        irr::s32 deltaYCursor = -1*cursorPixelRadius*cos(irr::core::DEGTORAD*guiRadarCursorBrg);
        //Plot if within the display and not at zero range
        if (cursorPixelRadius <= radius && guiRadarCursorRangeNm > 0) {
            irr::core::position2d<irr::s32> cursorCentre (centreX + deltaXCursor,centreY + deltaYCursor);
            device->getVideoDriver()->draw2DPolygon(cursorCentre,radius/20,irr::video::SColor(255, 255, 0, 0),4); //a 4 segment polygon, i.e. a square!
        }

        //Draw compass rose around radar (?Rotate with radar in head up and course up?)
        for (irr::u32 ticAngle = 0; ticAngle < 360; ticAngle += 5) {

            irr::f32 displayTicAngle = ticAngle;
            if (radarHeadUp) {
                displayTicAngle -= guiHeading;
            }

            irr::f32 scaling = 0.98;
            bool showValue = false;
            if(ticAngle % 20 == 0 ) {
                scaling = 0.90;
                showValue = true;
            } else if (ticAngle % 10 == 0) {
                scaling = 0.94;
            }

            irr::s32 deltaXTic = radius*sin(irr::core::DEGTORAD*displayTicAngle);
            irr::s32 deltaYTic = -1*radius*cos(irr::core::DEGTORAD*displayTicAngle);
            irr::core::position2d<irr::s32> ticInner (centreX + scaling*deltaXTic,centreY + scaling*deltaYTic);
            irr::core::position2d<irr::s32> ticOuter (centreX + deltaXTic,centreY + deltaYTic);

            device->getVideoDriver()->draw2DLine(ticInner,ticOuter,irr::video::SColor(255, 128, 128, 128));

            //Show the angle if needed
            if (showValue) {

                irr::core::stringw angleText = irr::core::stringw(ticAngle);

                irr::s32 textWidth = guienv->getSkin()->getFont()->getDimension(angleText.c_str()).Width;
                irr::s32 textHeight = guienv->getSkin()->getFont()->getDimension(angleText.c_str()).Height;
                irr::s32 textStartX = centreX + 0.8*deltaXTic-0.5*textWidth;
                irr::s32 textEndX = textStartX+textWidth;
                irr::s32 textStartY = centreY + 0.8*deltaYTic-0.5*textHeight;
                irr::s32 textEndY = textStartY+textHeight;
                guienv->getSkin()->getFont()->draw(angleText,irr::core::rect<irr::s32>(textStartX,textStartY,textEndX,textEndY),irr::video::SColor(255,128,128,128));
            }

        }

        //Draw range rings

        //Draw 4 range rings if radar range is divisible by 1.5, otherwise draw 4
        irr::u32 rangeRings;
        if ( std::fmod(guiRadarRangeNm,1.5) < 0.1) {
            rangeRings = 3;
        } else {
            rangeRings = 4;
        }
        for (unsigned int i = 1; i<rangeRings; i++) {
            irr::f32 ringRadius = radius*i/(float)rangeRings;
            irr::u8 noSegments = ringRadius/2;
            device->getVideoDriver()->draw2DPolygon(radarCentre,ringRadius,irr::video::SColor(128, 128, 128, 128),noSegments);
        }

    }

    void GUIMain::draw2dBearing()
    {

        //make cross hairs
        irr::s32 screenCentreX = 0.5*su;
        irr::s32 screenCentreY;
        if (showInterface) {
            screenCentreY = 0.3*sh;
        } else {
            screenCentreY = 0.5*sh;
        }
        irr::s32 lineLength = 0.1*sh;
        irr::core::position2d<irr::s32> left(screenCentreX-lineLength,screenCentreY);
        irr::core::position2d<irr::s32> right(screenCentreX+lineLength,screenCentreY);
        irr::core::position2d<irr::s32> top(screenCentreX,screenCentreY-lineLength);
        irr::core::position2d<irr::s32> bottom(screenCentreX,screenCentreY+lineLength);
        irr::core::position2d<irr::s32> centre(screenCentreX,screenCentreY);
        device->getVideoDriver()->draw2DLine(left,right,irr::video::SColor(255, 255, 0, 0));
        device->getVideoDriver()->draw2DLine(top,bottom,irr::video::SColor(255, 255, 0, 0));

        //show view bearing
        guienv->getSkin()->getFont()->draw(f32To1dp(viewHdg).c_str(),irr::core::rect<irr::s32>(screenCentreX-lineLength,screenCentreY-lineLength,screenCentreX, screenCentreY), irr::video::SColor(255,255,0,0),true,true);
        guienv->getSkin()->getFont()->draw(f32To1dp(viewElev).c_str(),irr::core::rect<irr::s32>(screenCentreX-lineLength,screenCentreY,screenCentreX, screenCentreY+lineLength), irr::video::SColor(255,255,0,0),true,true);


        //show angle (from horizon)

    }

    void GUIMain::drawCollisionWarning()
    {
        irr::s32 screenCentreX = 0.5*su;
        irr::s32 screenCentreY = 0.05*sh;

        device->getVideoDriver()->draw2DRectangle(irr::video::SColor(255,255,255,255),irr::core::rect<irr::s32>(screenCentreX-0.25*su,screenCentreY-0.025*sh,screenCentreX+0.25*su, screenCentreY+0.025*sh));
        guienv->getSkin()->getFont()->draw(language->translate("collided"),
            irr::core::rect<irr::s32>(screenCentreX-0.25*su,screenCentreY-0.025*sh,screenCentreX+0.25*su, screenCentreY+0.025*sh),
			irr::video::SColor(255,255,0,0),true,true);
    }

    void GUIMain::setExtraControlsWindowVisible(bool windowVisible)
    {
        extraControlsWindow->setVisible(windowVisible);
        if (windowVisible) {
            guienv->setFocus(extraControlsWindow);
        }
    }

    void GUIMain::setLinesControlsWindowVisible(bool windowVisible)
    {
        linesControlsWindow->setVisible(windowVisible);
        if (windowVisible) {
            guienv->setFocus(linesControlsWindow);
        }
    }

    void GUIMain::setLinesControlsText(std::string textToShow)
    {
        linesText->setText(irr::core::stringw(textToShow.c_str()).c_str());
    }

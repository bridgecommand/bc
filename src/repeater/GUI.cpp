/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2015 James Packer

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

#include "GUI.hpp"
#include "../Constants.hpp"
#include "../Utilities.hpp"

#include <iostream>
#include <limits>
#include <string>

//using namespace irr;

GUIMain::GUIMain(irr::IrrlichtDevice* device, Lang* language, irr::core::stringw message)
{
    this->device = device;
    guienv = device->getGUIEnvironment();

    irr::video::IVideoDriver* driver = device->getVideoDriver();
    unsigned int su = driver->getScreenSize().Width;
    unsigned int sh = driver->getScreenSize().Height;

    this->language = language;

    //gui

    messageText = guienv->addStaticText(message.c_str(),irr::core::rect<signed int>(su*0.0,sh*0.0,su*0.9,sh*0.1));

    //choice buttons
    headingButton = guienv->addButton(irr::core::rect<signed int>(su*0.1,sh*0.1,su*0.9,sh*0.5),0,GUI_ID_HEADING_CHOICE,language->translate("headingIndicator").c_str());
    repeaterButton = guienv->addButton(irr::core::rect<signed int>(su*0.1,sh*0.5,su*0.9,sh*0.9),0,GUI_ID_REPEATER_CHOICE,language->translate("repeater").c_str());

    //Heading indicator
    heading = new irr::gui::HeadingIndicator(guienv, guienv->getRootGUIElement(), irr::core::rect<signed int>(10,10,su-10,10+(su-20)/4));
    heading->setVisible(false);

    //User hasn't selected what mode to use
    modeChosen=false;

}

GUIMain::~GUIMain()
{
    heading->drop();
}

void GUIMain::updateGuiData(float time, float ownShipHeading, float rudderAngle, float wheelAngle, signed int portEngineRPM, signed int stbdEngineRPM)
{

    if(!modeChosen) {
        //Show GUI choice buttons only


    } else if (showHeadingIndicator) {

        float angleScale = 2.0; //Magnification factor for indicator

        heading->setVisible(true);

        //hide choice buttons
        headingButton->setVisible(false);
        repeaterButton->setVisible(false);
        messageText->setVisible(false);

        //Set value
        heading->setHeading(ownShipHeading);


        //Draw rudder angle
        irr::video::IVideoDriver* driver = device->getVideoDriver();
        unsigned int su = driver->getScreenSize().Width;
        unsigned int sh = driver->getScreenSize().Height;
        irr::core::vector2d<signed int> rudderIndicatorCentre = irr::core::vector2d<signed int>(0.5*su,0.5*sh);
        irr::core::vector2d<signed int> rudderAngleVectorHead = irr::core::vector2d<signed int> (        0,  0.240*sh);
        irr::core::vector2d<signed int> wheelAngleVectorHead = irr::core::vector2d<signed int> (        0,  0.240*sh);
        irr::core::vector2d<signed int> rudderAngleVectorBack1 = irr::core::vector2d<signed int>(-0.040*sh,  -0.040*sh);
        irr::core::vector2d<signed int> rudderAngleVectorBack2 = irr::core::vector2d<signed int>( 0.040*sh,  -0.040*sh);
        rudderAngleVectorHead.rotateBy(-1*rudderAngle*angleScale);
        wheelAngleVectorHead.rotateBy(-1*wheelAngle*angleScale);
        rudderAngleVectorBack1.rotateBy(-1*rudderAngle*angleScale);
        rudderAngleVectorBack2.rotateBy(-1*rudderAngle*angleScale);
        driver->draw2DLine(rudderIndicatorCentre+rudderAngleVectorBack1,rudderIndicatorCentre+rudderAngleVectorHead,irr::video::SColor(255,0,0,0));
        driver->draw2DLine(rudderIndicatorCentre+rudderAngleVectorHead,rudderIndicatorCentre+rudderAngleVectorBack2,irr::video::SColor(255,0,0,0));
        driver->draw2DLine(rudderIndicatorCentre+rudderAngleVectorBack2,rudderIndicatorCentre+rudderAngleVectorBack1,irr::video::SColor(255,0,0,0));
        driver->draw2DLine(rudderIndicatorCentre,rudderIndicatorCentre+wheelAngleVectorHead,irr::video::SColor(200,0,0,0));
        driver->draw2DPolygon(rudderIndicatorCentre,0.01*sh,irr::video::SColor(255,0,0,0));

        //Draw scale
        for(int i = -30; i<=30; i+=5) {

            signed int centreX = rudderIndicatorCentre.X;
            signed int centreY = rudderIndicatorCentre.Y;
            signed int radius = 0.25*sh;

            //Draw compass bearings
            float xVector = sin(i*irr::core::DEGTORAD*angleScale);
            float yVector = cos(i*irr::core::DEGTORAD*angleScale);

            //set scale of line
            float lineEnd;
            bool printAngle = false;
            if (i%10==0) {
                lineEnd = 1.1;
                printAngle = true;
            } else if (i%5==0) {
                lineEnd = 1.05;
            }

            signed int startX = centreX + xVector*radius;
            signed int endX = centreX + lineEnd*xVector*radius;
            signed int startY = centreY + yVector*radius;
            signed int endY = centreY + lineEnd*yVector*radius;

            //Set colour
            irr::video::SColor indicatorColour;
            if (i<0) {
                indicatorColour = irr::video::SColor(255,175,0,0);
            } else if (i>0) {
                indicatorColour = irr::video::SColor(255,0,175,0);
            } else {
                indicatorColour = irr::video::SColor(255,0,0,0);
            }

            driver->draw2DLine(irr::core::vector2d<signed int>(startX,startY),irr::core::vector2d<signed int>(endX,endY),indicatorColour);
            if (printAngle) {

                irr::core::stringw text;
                text = irr::core::stringw(abs(i));

                signed int textWidth = guienv->getSkin()->getFont()->getDimension(text.c_str()).Width;
                signed int textHeight = guienv->getSkin()->getFont()->getDimension(text.c_str()).Height;
                signed int textStartX = centreX + 1.2*xVector*radius-0.5*textWidth;
                signed int textEndX = textStartX+textWidth;
                signed int textStartY = centreY + 1.2*yVector*radius-0.5*textHeight;
                signed int textEndY = textStartY+textHeight;
                guienv->getSkin()->getFont()->draw(text,irr::core::rect<signed int>(textStartX,textStartY,textEndX,textEndY),indicatorColour);
            }
        }
        //End of draw scale

        //Show engine RPM
        irr::core::stringw portRPM(portEngineRPM);
        irr::core::stringw stbdRPM(stbdEngineRPM);
        guienv->getSkin()->getFont()->draw(portRPM,irr::core::rect<signed int>(0.25*su,0.515*sh,0.5*su,0.55*sh),irr::video::SColor(255,0,0,0));
        guienv->getSkin()->getFont()->draw(stbdRPM,irr::core::rect<signed int>(0.75*su,0.515*sh,1.0*su,0.55*sh),irr::video::SColor(255,0,0,0));



    } else {

        //hide choice buttons
        headingButton->setVisible(false);
        repeaterButton->setVisible(false);
        messageText->setVisible(false);

        //draw compass rose
        signed int centreX = device->getVideoDriver()->getScreenSize().Width/2;
        signed int centreY = device->getVideoDriver()->getScreenSize().Height/2;
        signed int radius = device->getVideoDriver()->getScreenSize().Height*0.45;

        irr::video::IVideoDriver* driver = device->getVideoDriver();

        driver->draw2DPolygon(irr::core::vector2d<signed int>(centreX,centreY),radius,irr::video::SColor(255,255,255,255),60);

        for(unsigned int i = 0; i<360; i++) {
            //Draw compass bearings
            float xVector = sin(irr::core::degToRad(i-ownShipHeading));
            float yVector = -1*cos(irr::core::degToRad(i-ownShipHeading));

            //set scale of line
            float lineStart;
            bool printAngle = false;
            bool printText = true;
            if (i%45==0) {
                lineStart = 0.7;
                printText=true;
            } else if (i%10==0) {
                lineStart = 0.85;
                printAngle = true;
            } else if (i%5==0) {
                lineStart = 0.88;
            } else {
                lineStart = 0.9;
            }

            signed int startX = centreX + lineStart*xVector*radius;
            signed int endX = centreX + 1.0*xVector*radius;
            signed int startY = centreY + lineStart*yVector*radius;
            signed int endY = centreY + 1.0*yVector*radius;

            driver->draw2DLine(irr::core::vector2d<signed int>(startX,startY),irr::core::vector2d<signed int>(endX,endY),irr::video::SColor(255,255,255,255));
            if (printAngle || printText) {

                irr::core::stringw text;
                if (printAngle) {
                    text = irr::core::stringw(i);
                } else {
                    if (i==0) {
                        text = L"N";
                    } else if (i==90) {
                        text = L"E";
                    } else if (i==180) {
                        text = L"S";
                    } else if (i==270) {
                        text = L"W";
                    }
                     else {
                        text = L"";
                    }
                }

                signed int textWidth = guienv->getSkin()->getFont()->getDimension(text.c_str()).Width;
                signed int textHeight = guienv->getSkin()->getFont()->getDimension(text.c_str()).Height;
                signed int textStartX = centreX + 0.8*xVector*radius-0.5*textWidth;
                signed int textEndX = textStartX+textWidth;
                signed int textStartY = centreY + 0.8*yVector*radius-0.5*textHeight;
                signed int textEndY = textStartY+textHeight;
                guienv->getSkin()->getFont()->draw(text,irr::core::rect<signed int>(textStartX,textStartY,textEndX,textEndY),irr::video::SColor(255,255,255,255));
            }
        }


        //Draw heading indicator
        driver->draw2DLine(irr::core::vector2d<signed int>(centreX,centreY),irr::core::vector2d<signed int>(centreX,centreY-1.1*radius),irr::video::SColor(255,255,255,255));
    }
    guienv->drawAll();

}

void GUIMain::setMode(bool headingMode)
{
    modeChosen=true;
    showHeadingIndicator=headingMode;
}

std::wstring GUIMain::f32To1dp(float value)
{
    //Convert a floating point value to a wstring, with 1dp
    char tempStr[100];
    snprintf(tempStr,100,"%.1f",value);
    return std::wstring(tempStr, tempStr+strlen(tempStr));
}

std::wstring GUIMain::f32To3dp(float value)
{
    //Convert a floating point value to a wstring, with 3dp
    char tempStr[100];
    snprintf(tempStr,100,"%.3f",value);
    return std::wstring(tempStr, tempStr+strlen(tempStr));
}

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

using namespace irr;

GUIMain::GUIMain(IrrlichtDevice* device, Lang* language)
{
    this->device = device;
    guienv = device->getGUIEnvironment();

    video::IVideoDriver* driver = device->getVideoDriver();
    u32 su = driver->getScreenSize().Width;
    u32 sh = driver->getScreenSize().Height;

    this->language = language;

    //gui

    heading = new gui::HeadingIndicator(guienv, guienv->getRootGUIElement(), core::rect<s32>(10,10,su-10,10+(su-20)/4));

}

GUIMain::~GUIMain()
{
    heading->drop();
}

void GUIMain::updateGuiData(irr::f32 time, irr::f32 ownShipHeading)
{

    bool showHeadingIndicator = false;

    if (showHeadingIndicator) {
        heading->setHeading(ownShipHeading);
        heading->setVisible(true);
    } else {

        heading->setVisible(false);

        //draw compass rose
        s32 centreX = device->getVideoDriver()->getScreenSize().Width/2;
        s32 centreY = device->getVideoDriver()->getScreenSize().Height/2;
        s32 radius = device->getVideoDriver()->getScreenSize().Height*0.45;

        video::IVideoDriver* driver = device->getVideoDriver();

        driver->draw2DPolygon(core::vector2d<s32>(centreX,centreY),radius,video::SColor(255,255,255,255),60);

        for(unsigned int i = 0; i<360; i++) {
            //Draw compass bearings
            f32 xVector = sin(core::degToRad(i-ownShipHeading));
            f32 yVector = -1*cos(core::degToRad(i-ownShipHeading));

            //set scale of line
            f32 lineStart;
            bool printAngle = false;
            if (i%45==0) {
                lineStart = 0.7;
            } else if (i%10==0) {
                lineStart = 0.85;
                printAngle = true;
            } else if (i%5==0) {
                lineStart = 0.88;
            } else {
                lineStart = 0.9;
            }

            s32 startX = centreX + lineStart*xVector*radius;
            s32 endX = centreX + 1.0*xVector*radius;
            s32 startY = centreY + lineStart*yVector*radius;
            s32 endY = centreY + 1.0*yVector*radius;

            driver->draw2DLine(core::vector2d<s32>(startX,startY),core::vector2d<s32>(endX,endY),video::SColor(255,255,255,255));
            if (printAngle) {
                s32 textWidth = guienv->getSkin()->getFont()->getDimension(core::stringw(i).c_str()).Width;
                s32 textHeight = guienv->getSkin()->getFont()->getDimension(core::stringw(i).c_str()).Height;
                s32 textStartX = centreX + 0.8*xVector*radius-0.5*textWidth;
                s32 textEndX = textStartX+textWidth;
                s32 textStartY = centreY + 0.8*yVector*radius-0.5*textHeight;
                s32 textEndY = textStartY+textHeight;
                guienv->getSkin()->getFont()->draw(core::stringw(i),core::rect<s32>(textStartX,textStartY,textEndX,textEndY),video::SColor(255,255,255,255));
            }
        }


        //Draw heading indicator
        driver->draw2DLine(core::vector2d<s32>(centreX,centreY),core::vector2d<s32>(centreX,centreY-1.1*radius),video::SColor(255,255,255,255));
    }
    guienv->drawAll();

}

std::wstring GUIMain::f32To1dp(irr::f32 value)
{
    //Convert a floating point value to a wstring, with 1dp
    char tempStr[100];
    snprintf(tempStr,100,"%.1f",value);
    return std::wstring(tempStr, tempStr+strlen(tempStr));
}

std::wstring GUIMain::f32To3dp(irr::f32 value)
{
    //Convert a floating point value to a wstring, with 3dp
    char tempStr[100];
    snprintf(tempStr,100,"%.3f",value);
    return std::wstring(tempStr, tempStr+strlen(tempStr));
}

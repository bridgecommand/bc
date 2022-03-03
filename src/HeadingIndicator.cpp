// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

//#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IVideoDriver.h"
#include "IGUIFont.h"
#include "IGUIFontBitmap.h"

#include "HeadingIndicator.h"

namespace irr
{
namespace gui
{


HeadingIndicator::HeadingIndicator(IGUIEnvironment* environment, IGUIElement* parent, irr::core::rect<irr::s32> rectangle) :
IGUIElement(EGUIET_ELEMENT ,environment,parent,0,rectangle), heading(0)
{
}

//! destructor
HeadingIndicator::~HeadingIndicator()
{
}

//! draws the element and its children
void HeadingIndicator::draw()
{

if (!IsVisible)
		return;

    IGUISkin* skin = Environment->getSkin();

    if (!skin)
        return;

    irr::u32 skinAlpha = skin->getColor(irr::gui::EGDC_3D_FACE).getAlpha();
    irr::gui::IGUIFont* font = skin->getFont();

	// draws the background
	//skin->draw2DRectangle(this, skin->getColor(EGDC_SCROLLBAR), SliderRect, &AbsoluteClippingRect);
	//Environment->getVideoDriver()->draw2DRectangle(video::SColor(255,128,128,128),SliderRect,&AbsoluteClippingRect);
	Environment->getVideoDriver()->draw2DRectangle(video::SColor(skinAlpha/4,255,255,255),AbsoluteRect); //Todo: Think about clipping (find smaller of AbsoluteClippingRect and SliderRect?)
    
    Environment->getVideoDriver()->draw2DRectangleOutline(AbsoluteRect,video::SColor(skinAlpha,0,0,0)); //Todo: Think about clipping (find smaller of AbsoluteClippingRect and SliderRect?)

	//Find the centre point
	irr::core::vector2d<irr::s32> centrePoint = AbsoluteRect.getCenter();

    //Find integer angles within +- angleRange degrees of heading

    irr::s32 angleRange = 20;

    irr::s32 minAngle = ceil(heading - angleRange);
    irr::s32 maxAngle = floor(heading + angleRange);

    irr::f32 pxPerDegree = (irr::f32)AbsoluteRect.getWidth()/(irr::f32)(angleRange*2);

    //Draw tics
    irr::core::vector2d<irr::s32> startPoint;
	irr::core::vector2d<irr::s32> endPoint;

    for (int ticAng = minAngle; ticAng<= maxAngle; ticAng++) {
        startPoint.X = centrePoint.X + pxPerDegree*(ticAng-heading);
        endPoint.X = startPoint.X;
        endPoint.Y = AbsoluteRect.LowerRightCorner.Y;

        //Make tic length depend on whether it's a multiple of 10, 5 or other
        if (ticAng % 10 == 0) {
            startPoint.Y = 0.5*centrePoint.Y + 0.5*AbsoluteRect.UpperLeftCorner.Y;

            //Draw position
            irr::s32 displayAng = ticAng;
            while (displayAng < 0) {displayAng+=360;}
            while (displayAng >= 360) {displayAng-=360;}
            font->draw(irr::core::stringw(displayAng),irr::core::rect<irr::s32>(startPoint.X - 100, AbsoluteRect.UpperLeftCorner.Y, startPoint.X + 100, AbsoluteRect.LowerRightCorner.Y),video::SColor(skinAlpha,0,0,0),true,false,&AbsoluteRect);

        } else if (ticAng % 5 == 0) {
            startPoint.Y = centrePoint.Y;
        } else {
            startPoint.Y = 0.5*centrePoint.Y + 0.5*AbsoluteRect.LowerRightCorner.Y;
        }

        Environment->getVideoDriver()->draw2DLine(startPoint,endPoint,video::SColor(skinAlpha,0,0,0));
    }

    //Draw a centre line
    startPoint.X = centrePoint.X;
    startPoint.Y = AbsoluteRect.UpperLeftCorner.Y;
    endPoint.X = centrePoint.X;
    endPoint.Y = AbsoluteRect.LowerRightCorner.Y;
    Environment->getVideoDriver()->draw2DLine(startPoint,endPoint,video::SColor(skinAlpha,0,0,0));

}

void HeadingIndicator::setHeading(irr::f32 heading)
{
    this->heading = heading;
}

}
}



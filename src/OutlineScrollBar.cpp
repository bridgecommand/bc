// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

//#include "IGUISkin.h"
#include "IGUIEnvironment.h"
#include "IVideoDriver.h"
#include "IGUIFont.h"
#include "IGUIFontBitmap.h"

#include "OutlineScrollBar.h"

namespace irr
{
namespace gui
{


//! constructor
OutlineScrollBar::OutlineScrollBar(bool horizontal, IGUIEnvironment* environment,
				IGUIElement* parent, s32 id,
				core::rect<s32> rectangle, core::array<s32> shortTicMarks, core::array<s32> longTicMarks, bool secondaryIndicator, core::array<s32> ticIndicators)
	: IGUIScrollBar(environment, parent, id, rectangle), Dragging(false), Horizontal(horizontal),
	Pos(0), DrawPos(0),
	DrawHeight(0), Min(0), Max(100), SmallStep(10), LargeStep(50), DesiredPos(0),
	shortTicMarks(shortTicMarks), longTicMarks(longTicMarks), Secondary(secondaryIndicator), ticIndicators(ticIndicators)
{
	#ifdef _DEBUG
	setDebugName("OutlineScrollBar");
	#endif

//	refreshControls();

	//setNotClipped(noclip);

	// this element can be tabbed to
	setTabStop(true);
	setTabOrder(-1);

	setPos(0);

}


//! destructor
OutlineScrollBar::~OutlineScrollBar()
{

}


//! called if an event happened.
bool OutlineScrollBar::OnEvent(const SEvent& event)
{
	if (isEnabled())
	{

		switch(event.EventType)
		{
		case EET_KEY_INPUT_EVENT:
			if (event.KeyInput.PressedDown)
			{
				const s32 oldPos = Pos;
				bool absorb = true;
				switch (event.KeyInput.Key)
				{
				case KEY_LEFT:
				case KEY_UP:
					setPos(Pos-SmallStep);
					break;
				case KEY_RIGHT:
				case KEY_DOWN:
					setPos(Pos+SmallStep);
					break;
				case KEY_HOME:
					setPos(Min);
					break;
				case KEY_PRIOR:
					setPos(Pos-LargeStep);
					break;
				case KEY_END:
					setPos(Max);
					break;
				case KEY_NEXT:
					setPos(Pos+LargeStep);
					break;
				default:
					absorb = false;
				}

				if (Pos != oldPos)
				{
					SEvent newEvent;
					newEvent.EventType = EET_GUI_EVENT;
					newEvent.GUIEvent.Caller = this;
					newEvent.GUIEvent.Element = 0;
					newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
					Parent->OnEvent(newEvent);
				}
				if (absorb)
					return true;
			}
			break;
		case EET_GUI_EVENT:
			if (event.GUIEvent.EventType == EGET_ELEMENT_FOCUS_LOST)
			{
				if (event.GUIEvent.Caller == this)
					Dragging = false;
			}
			break;
		case EET_MOUSE_INPUT_EVENT:
		{
			const core::position2di p(event.MouseInput.X, event.MouseInput.Y);
			bool isInside = isPointInside ( p );
			switch(event.MouseInput.Event)
			{
			case EMIE_MOUSE_WHEEL:
				if (Environment->hasFocus(this))
				{
					// thanks to a bug report by REAPER
					// thanks to tommi by tommi for another bugfix
					// everybody needs a little thanking. hallo niko!;-)
					setPos(	getPos() +
							( (event.MouseInput.Wheel < 0 ? -1 : 1) * SmallStep * (Horizontal ? 1 : -1 ) )
							);

					SEvent newEvent;
					newEvent.EventType = EET_GUI_EVENT;
					newEvent.GUIEvent.Caller = this;
					newEvent.GUIEvent.Element = 0;
					newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
					Parent->OnEvent(newEvent);
					return true;
				}
				break;
			case EMIE_LMOUSE_PRESSED_DOWN:
			case EMIE_RMOUSE_PRESSED_DOWN: //JAMES: Allow right click for scroll bar movement
			{
				if (isInside)
				{
					Dragging = true;
					//DraggedBySlider = SliderRect.isPointInside(p);
					//TrayClick = !DraggedBySlider;
					//DesiredPos = getPosFromMousePos(p);
					setPos(getPosFromMousePos(p));
					SEvent newEvent;
					newEvent.EventType = EET_GUI_EVENT;
					newEvent.GUIEvent.Caller = this;
					newEvent.GUIEvent.Element = 0;
					newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
					Parent->OnEvent(newEvent);
					Environment->setFocus ( this );
					return true;
				}
				break;
			}
			case EMIE_LMOUSE_LEFT_UP:
			case EMIE_RMOUSE_LEFT_UP: //JAMES: Allow right click for scroll bar movement
			case EMIE_MOUSE_MOVED:
			{
				if ( !event.MouseInput.isLeftPressed () && !event.MouseInput.isRightPressed ()  ) //JAMES: Allow right click for scroll bar movement
					Dragging = false;

				if ( !Dragging )
				{
					if ( event.MouseInput.Event == EMIE_MOUSE_MOVED )
						break;
					return isInside;
				}

				if ( event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP || event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP ) //JAMES: Allow right click for scroll bar movement
					Dragging = false;

				const s32 newPos = getPosFromMousePos(p);
				const s32 oldPos = Pos;
                setPos(newPos);

				if (Pos != oldPos && Parent)
				{
					SEvent newEvent;
					newEvent.EventType = EET_GUI_EVENT;
					newEvent.GUIEvent.Caller = this;
					newEvent.GUIEvent.Element = 0;
					newEvent.GUIEvent.EventType = EGET_SCROLL_BAR_CHANGED;
					Parent->OnEvent(newEvent);
				}
				return isInside;
			} break;

			default:
				break;
			}
		} break;
		default:
			break;
		}
	}

	return IGUIElement::OnEvent(event);
}

void OutlineScrollBar::OnPostRender(u32 timeMs)
{

}

//! draws the element and its children
void OutlineScrollBar::draw()
{

	if (!IsVisible)
		return;



    irr::gui::IGUIFont* font = 0;
    IGUISkin* skin = Environment->getSkin();
    u32 skinAlpha = 255;
    if (skin) {
        skinAlpha = skin->getColor(gui::EGDC_3D_FACE).getAlpha();
        font = skin->getFont();
    }

	SliderRect = AbsoluteRect;

	// draws the background
	//skin->draw2DRectangle(this, skin->getColor(EGDC_SCROLLBAR), SliderRect, &AbsoluteClippingRect);
	//Environment->getVideoDriver()->draw2DRectangle(video::SColor(255,128,128,128),SliderRect,&AbsoluteClippingRect);
	
	Environment->getVideoDriver()->draw2DRectangle(video::SColor(skinAlpha/4,255,255,255),SliderRect); //Todo: Think about clipping (find smaller of AbsoluteClippingRect and SliderRect?)
	
	Environment->getVideoDriver()->draw2DRectangleOutline(SliderRect,video::SColor(skinAlpha,0,0,0)); //Todo: Think about clipping (find smaller of AbsoluteClippingRect and SliderRect?)

	if ( core::isnotzero ( range() ) )
	{
		// recalculate slider line
		core::vector2d<s32> startPoint;
		core::vector2d<s32> endPoint;
		if (Horizontal)
		{
			//Slider line
            startPoint.X = AbsoluteRect.UpperLeftCorner.X + DrawPos;
            endPoint.X = startPoint.X;
            startPoint.Y = AbsoluteRect.UpperLeftCorner.Y;
            endPoint.Y = AbsoluteRect.LowerRightCorner.Y;
		}
		else
		{
			startPoint.Y = AbsoluteRect.UpperLeftCorner.Y + DrawPos;
            endPoint.Y = startPoint.Y;
            startPoint.X = AbsoluteRect.UpperLeftCorner.X;
            endPoint.X = AbsoluteRect.LowerRightCorner.X;
		}

		Environment->getVideoDriver()->draw2DLine(startPoint,endPoint,video::SColor(skinAlpha,0,0,0));


		if (Secondary) {
            //Start secondary line
            if ( core::isnotzero ( range() ) ) {
                // recalculate slider line
                core::vector2d<s32> startPoint;
                core::vector2d<s32> endPoint;
                if (Horizontal)
                {
                    //Slider line
                    startPoint.X = AbsoluteRect.UpperLeftCorner.X + DrawPosSecondary;
                    endPoint.X = startPoint.X;
                    startPoint.Y = AbsoluteRect.UpperLeftCorner.Y;
                    endPoint.Y = AbsoluteRect.LowerRightCorner.Y;
                }
                else
                {
                    startPoint.Y = AbsoluteRect.UpperLeftCorner.Y + DrawPosSecondary;
                    endPoint.Y = startPoint.Y;
                    startPoint.X = AbsoluteRect.UpperLeftCorner.X;
                    endPoint.X = AbsoluteRect.LowerRightCorner.X;
                }

                Environment->getVideoDriver()->draw2DLine(startPoint,endPoint,video::SColor(skinAlpha,64,64,64));
            }
            //End secondary line
		}

		//draw tic marks
		for (unsigned int i = 0; i<shortTicMarks.size();i++) {
            if (Horizontal)
            {
                f32 f = RelativeRect.getWidth() / range();
                s32 ticPos = (s32)(( shortTicMarks[i] - Min ) * f);

                startPoint.X = AbsoluteRect.UpperLeftCorner.X + ticPos;
                endPoint.X = startPoint.X;

                startPoint.Y = 0.6*AbsoluteRect.UpperLeftCorner.Y + 0.4*AbsoluteRect.LowerRightCorner.Y;
                endPoint.Y   = 0.4*AbsoluteRect.UpperLeftCorner.Y + 0.6*AbsoluteRect.LowerRightCorner.Y;

                if (ticIndicators.size() == shortTicMarks.size() && font ) {
                    font->draw(irr::core::stringw(ticIndicators[i]),irr::core::rect<irr::s32>(startPoint.X - 100, AbsoluteRect.LowerRightCorner.Y - font->getDimension(irr::core::stringw(ticIndicators[i]).c_str()).Height, startPoint.X + 100, AbsoluteRect.LowerRightCorner.Y),video::SColor(skinAlpha,0,0,0),true,false,&AbsoluteRect);
                }
            }
            else
            {
                f32 f = RelativeRect.getHeight()/ range();
                s32 ticPos = (s32)(( shortTicMarks[i] - Min ) * f);

                startPoint.Y = AbsoluteRect.UpperLeftCorner.Y + ticPos;
                endPoint.Y = startPoint.Y;

                startPoint.X = 0.6*AbsoluteRect.UpperLeftCorner.X + 0.4*AbsoluteRect.LowerRightCorner.X;
                endPoint.X   = 0.4*AbsoluteRect.UpperLeftCorner.X + 0.6*AbsoluteRect.LowerRightCorner.X;

				if (ticIndicators.size() == shortTicMarks.size() && font ) {
                    font->draw(irr::core::stringw(ticIndicators[i]),irr::core::rect<irr::s32>(AbsoluteRect.UpperLeftCorner.X + 0.5*AbsoluteRect.getWidth(), startPoint.Y-100, AbsoluteRect.LowerRightCorner.X, startPoint.Y+100),video::SColor(skinAlpha,0,0,0),false,true,&AbsoluteRect);
                }
            }
            Environment->getVideoDriver()->draw2DLine(startPoint,endPoint,video::SColor(skinAlpha,0,0,0));

        }
        for (unsigned int i = 0; i<longTicMarks.size();i++) {
            if (Horizontal)
            {
                f32 f = RelativeRect.getWidth() / range();
                s32 ticPos = (s32)(( longTicMarks[i] - Min ) * f);

                startPoint.X = AbsoluteRect.UpperLeftCorner.X + ticPos;
                endPoint.X = startPoint.X;

                startPoint.Y = 0.8*AbsoluteRect.UpperLeftCorner.Y + 0.2*AbsoluteRect.LowerRightCorner.Y;
                endPoint.Y   = 0.2*AbsoluteRect.UpperLeftCorner.Y + 0.8*AbsoluteRect.LowerRightCorner.Y;
            }
            else
            {
                f32 f = RelativeRect.getHeight()/ range();
                s32 ticPos = (s32)(( longTicMarks[i] - Min ) * f);

                startPoint.Y = AbsoluteRect.UpperLeftCorner.Y + ticPos;
                endPoint.Y = startPoint.Y;

                startPoint.X = 0.8*AbsoluteRect.UpperLeftCorner.X + 0.2*AbsoluteRect.LowerRightCorner.X;
                endPoint.X   = 0.2*AbsoluteRect.UpperLeftCorner.X + 0.8*AbsoluteRect.LowerRightCorner.X;
            }
            Environment->getVideoDriver()->draw2DLine(startPoint,endPoint,video::SColor(skinAlpha,0,0,0));
        }
	}

	// draw buttons
	//IGUIElement::draw();
}


void OutlineScrollBar::updateAbsolutePosition()
{
	IGUIElement::updateAbsolutePosition();
	// todo: properly resize
//	refreshControls();
	setPos ( Pos );
}

//!
s32 OutlineScrollBar::getPosFromMousePos(const core::position2di &pos) const
{
	f32 w, p;
	if (Horizontal)
	{
		w = RelativeRect.getWidth();
		p = pos.X - AbsoluteRect.UpperLeftCorner.X;
	}
	else
	{
		w = RelativeRect.getHeight();
		p = pos.Y - AbsoluteRect.UpperLeftCorner.Y;
	}
	return (s32) ( p/w * range() ) + Min;
}


//! sets the position of the scrollbar
void OutlineScrollBar::setPos(s32 pos)
{
	Pos = core::s32_clamp ( pos, Min, Max );

	if (Horizontal)
	{
		f32 f = RelativeRect.getWidth() / range();
		DrawPos = (s32)(( Pos - Min ) * f);
		DrawHeight = RelativeRect.getHeight();
	}
	else
	{
		f32 f = RelativeRect.getHeight()/ range();

		DrawPos = (s32)(( Pos - Min ) * f);
		DrawHeight = RelativeRect.getWidth();
	}

}

//! sets the position of the secondary indicator
void OutlineScrollBar::setSecondary(s32 pos)
{
	PosSecondary = core::s32_clamp ( pos, Min, Max );

	if (Horizontal)
	{
		f32 f = RelativeRect.getWidth() / range();
		DrawPosSecondary = (s32)(( PosSecondary - Min ) * f);
		DrawHeightSecondary = RelativeRect.getHeight();
	}
	else
	{
		f32 f = RelativeRect.getHeight()/ range();

		DrawPosSecondary = (s32)(( PosSecondary - Min ) * f);
		DrawHeightSecondary = RelativeRect.getWidth();
	}

}

//! gets the small step value
s32 OutlineScrollBar::getSmallStep() const
{
	return SmallStep;
}


//! sets the small step value
void OutlineScrollBar::setSmallStep(s32 step)
{
	if (step > 0)
		SmallStep = step;
	else
		SmallStep = 10;
}


//! gets the small step value
s32 OutlineScrollBar::getLargeStep() const
{
	return LargeStep;
}


//! sets the small step value
void OutlineScrollBar::setLargeStep(s32 step)
{
	if (step > 0)
		LargeStep = step;
	else
		LargeStep = 50;
}


//! gets the maximum value of the scrollbar.
s32 OutlineScrollBar::getMax() const
{
	return Max;
}


//! sets the maximum value of the scrollbar.
void OutlineScrollBar::setMax(s32 max)
{
	Max = max;
	if ( Min > Max )
		Min = Max;

	setPos(Pos);
}

//! gets the minimum value of the scrollbar.
s32 OutlineScrollBar::getMin() const
{
	return Min;
}


//! sets the minimum value of the scrollbar.
void OutlineScrollBar::setMin(s32 min)
{
	Min = min;
	if ( Max < Min )
		Max = Min;

	setPos(Pos);
}


//! gets the current position of the scrollbar
s32 OutlineScrollBar::getPos() const
{
	return Pos;
}

s32 OutlineScrollBar::getSecondary() const
{
	return PosSecondary;
}

/*
//! refreshes the position and text on child buttons
void OutlineScrollBar::refreshControls()
{
	CurrentIconColor = video::SColor(255,255,255,255);

	IGUISkin* skin = Environment->getSkin();

	if (skin)
	{
		CurrentIconColor = skin->getColor(isEnabled() ? EGDC_WINDOW_SYMBOL : EGDC_GRAY_WINDOW_SYMBOL);
	}

	if (Horizontal)
	{
		s32 h = RelativeRect.getHeight();
	}
	else
	{
		s32 w = RelativeRect.getWidth();
	}
}
*/

/*
//! Writes attributes of the element.
void OutlineScrollBar::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const
{
	IGUIScrollBar::serializeAttributes(out,options);

	out->addBool("Horizontal",	Horizontal);
	out->addInt ("Value",		Pos);
	out->addInt ("Min",			Min);
	out->addInt ("Max",			Max);
	out->addInt ("SmallStep",	SmallStep);
	out->addInt ("LargeStep",	LargeStep);
	// CurrentIconColor - not serialized as continuiously updated
}


//! Reads attributes of the element
void OutlineScrollBar::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0)
{
	IGUIScrollBar::deserializeAttributes(in,options);

	Horizontal = in->getAttributeAsBool("Horizontal");
	setMin(in->getAttributeAsInt("Min"));
	setMax(in->getAttributeAsInt("Max"));
	setPos(in->getAttributeAsInt("Value"));
	setSmallStep(in->getAttributeAsInt("SmallStep"));
	setLargeStep(in->getAttributeAsInt("LargeStep"));
	// CurrentIconColor - not serialized as continuiously updated

	refreshControls();
}
*/

} // end namespace gui
} // end namespace irr


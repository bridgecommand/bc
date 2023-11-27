// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
// This device code is based on the original SDL device implementation
// contributed by Shane Parker (sirshane).

#ifndef IRR_C_IRR_DEVICE_SDL_H_INCLUDED
#define IRR_C_IRR_DEVICE_SDL_H_INCLUDED

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_SDL_DEVICE_

#include "IrrlichtDevice.h"
#include "CIrrDeviceStub.h"
#include "IImagePresenter.h"
#include "ICursorControl.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

namespace irr
{

	class CIrrDeviceSDL : public CIrrDeviceStub, video::IImagePresenter
	{
	public:

		//! constructor
		CIrrDeviceSDL(const SIrrlichtCreationParameters& param);

		//! destructor
		virtual ~CIrrDeviceSDL();

		//! runs the device. Returns false if device wants to be deleted
		virtual bool run() IRR_OVERRIDE;

		//! pause execution temporarily
		virtual void yield() IRR_OVERRIDE;

		//! pause execution for a specified time
		virtual void sleep(u32 timeMs, bool pauseTimer) IRR_OVERRIDE;

		//! sets the caption of the window
		virtual void setWindowCaption(const wchar_t* text) IRR_OVERRIDE;

		//! returns if window is active. if not, nothing need to be drawn
		virtual bool isWindowActive() const IRR_OVERRIDE;

		//! returns if window has focus.
		bool isWindowFocused() const IRR_OVERRIDE;

		//! returns if window is minimized.
		bool isWindowMinimized() const IRR_OVERRIDE;

		//! returns color format of the window.
		video::ECOLOR_FORMAT getColorFormat() const IRR_OVERRIDE;

		//! presents a surface in the client area
		virtual bool present(video::IImage* surface, void* windowId=0, core::rect<s32>* src=0) IRR_OVERRIDE;

		//! notifies the device that it should close itself
		virtual void closeDevice() IRR_OVERRIDE;

		//! \return Returns a pointer to a list with all video modes supported
		virtual video::IVideoModeList* getVideoModeList() IRR_OVERRIDE;

		//! Sets if the window should be resizable in windowed mode.
		virtual void setResizable(bool resize=false) IRR_OVERRIDE;

		//! Minimizes the window.
		virtual void minimizeWindow() IRR_OVERRIDE;

		//! Maximizes the window.
		virtual void maximizeWindow() IRR_OVERRIDE;

		//! Restores the window size.
		virtual void restoreWindow() IRR_OVERRIDE;

		//! Get the position of this window on screen
		virtual core::position2di getWindowPosition() IRR_OVERRIDE;

		//! Activate any joysticks, and generate events for them.
		virtual bool activateJoysticks(core::array<SJoystickInfo> & joystickInfo) IRR_OVERRIDE;

		//! Set the current Gamma Value for the Display
		virtual bool setGammaRamp( f32 red, f32 green, f32 blue, f32 brightness, f32 contrast ) IRR_OVERRIDE;

		//! Get the current Gamma Value for the Display
		virtual bool getGammaRamp( f32 &red, f32 &green, f32 &blue, f32 &brightness, f32 &contrast ) IRR_OVERRIDE;

		//! Get the device type
		virtual E_DEVICE_TYPE getType() const IRR_OVERRIDE
		{
			return EIDT_SDL;
		}

		//! Implementation of the linux cursor control
		class CCursorControl : public gui::ICursorControl
		{
		public:

			CCursorControl(CIrrDeviceSDL* dev)
				: Device(dev), IsVisible(true)
			{
			}

			//! Changes the visible state of the mouse cursor.
			virtual void setVisible(bool visible) IRR_OVERRIDE
			{
				IsVisible = visible;
				if ( visible )
					SDL_ShowCursor( SDL_ENABLE );
				else
					SDL_ShowCursor( SDL_DISABLE );
			}

			//! Returns if the cursor is currently visible.
			virtual bool isVisible() const IRR_OVERRIDE
			{
				return IsVisible;
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(const core::position2d<f32> &pos) IRR_OVERRIDE
			{
				setPosition(pos.X, pos.Y);
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(f32 x, f32 y) IRR_OVERRIDE
			{
				setPosition((s32)(x*Device->Width), (s32)(y*Device->Height));
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(const core::position2d<s32> &pos) IRR_OVERRIDE
			{
				setPosition(pos.X, pos.Y);
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(s32 x, s32 y) IRR_OVERRIDE
			{
				SDL_WarpMouse( x, y );
				CursorPos.X = x;
				CursorPos.Y = y;
			}

			//! Returns the current position of the mouse cursor.
			virtual const core::position2d<s32>& getPosition(bool updateCursor) IRR_OVERRIDE
			{
				if ( updateCursor )
					updateCursorPos();
				return CursorPos;
			}

			//! Returns the current position of the mouse cursor.
			virtual core::position2d<f32> getRelativePosition(bool updateCursor) IRR_OVERRIDE
			{
				if ( updateCursor )
					updateCursorPos();
				return core::position2d<f32>(CursorPos.X / (f32)Device->Width,
					CursorPos.Y / (f32)Device->Height);
			}

			virtual void setReferenceRect(core::rect<s32>* rect=0) IRR_OVERRIDE
			{
			}

			virtual bool getReferenceRect(core::rect<s32>& rect) IRR_OVERRIDE
			{ 
				rect.UpperLeftCorner = core::vector2di(0,0);
				rect.LowerRightCorner.X = (irr::s32)Device->Width;
				rect.LowerRightCorner.Y = (irr::s32)Device->Height;
				return false;
			}

		private:

			void updateCursorPos()
			{
				CursorPos.X = Device->MouseX;
				CursorPos.Y = Device->MouseY;
			}

			CIrrDeviceSDL* Device;
			core::position2d<s32> CursorPos;
			bool IsVisible;
		};

	private:

		//! create the driver
		void createDriver();

		bool createWindow();

		void createKeyMap();

		SDL_Surface* Screen;
		int SDL_Flags;
#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
		core::array<SDL_Joystick*> Joysticks;
#endif

		s32 MouseX, MouseY;
		u32 MouseButtonStates;

		u32 Width, Height;

		bool Resizable;
		bool WindowMinimized;

		struct SKeyMap
		{
			SKeyMap() {}
			SKeyMap(s32 x11, s32 win32)
				: SDLKey(x11), Win32Key(win32)
			{
			}

			s32 SDLKey;
			s32 Win32Key;

			bool operator<(const SKeyMap& o) const
			{
				return SDLKey<o.SDLKey;
			}
		};

		core::array<SKeyMap> KeyMap;
		SDL_SysWMinfo Info;
	};

} // end namespace irr

#endif // _IRR_COMPILE_WITH_SDL_DEVICE_
#endif // IRR_C_IRR_DEVICE_SDL_H_INCLUDED

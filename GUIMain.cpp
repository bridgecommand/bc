#include "GUIMain.hpp"

#include "Constants.hpp"

using namespace irr;

GUIMain::GUIMain(IrrlichtDevice* device)
    {
        this->device = device;
        guienv = device->getGUIEnvironment();

        //gui - add scroll bars
        hdgScrollbar = guienv->addScrollBar(false,core::rect<s32>(10, 240, 30, 470), 0, GUI_ID_HEADING_SCROLL_BAR);
        hdgScrollbar->setMax(360);

        spdScrollbar = guienv->addScrollBar(false,core::rect<s32>(40, 240, 60, 470), 0, GUI_ID_SPEED_SCROLL_BAR);
        spdScrollbar->setMax(20.f*1852.f/3600.f); //20 knots in m/s

        //add data display:
        dataDisplay = guienv->addStaticText(L"", core::rect<s32>(20,20,120,80), true); //Actual text set later
        guiHeading = 0;
        guiSpeed = 0;

        //Add paused button
        pausedButton = guienv->addButton(core::rect<s32>(200,200,600,400),0,GUI_ID_START_BUTTON,L"Paused, click to start");//i18n
    }

    void GUIMain::updateGuiData(f32 hdg, f32 spd, f32 depth, std::string currentTime, bool paused)
    {
        //Update scroll bars
        hdgScrollbar->setPos(hdg);
        spdScrollbar->setPos(spd);
        //Update text display data
        guiHeading = hdg; //Heading in degrees
        guiSpeed = spd*MPS_TO_KTS; //Speed in knots
        guiDepth = depth;
        guiTime = currentTime;
        guiPaused = paused;
    }

    void GUIMain::drawGUI()
    {
        //update heading display element
        core::stringw displayText = L"Heading: "; //i18n
        displayText.append(core::stringw(guiHeading));
        displayText.append(L"\nSpeed: ");
        displayText.append(core::stringw(guiSpeed));
        displayText.append(L"\nDepth: ");
        displayText.append(core::stringw(guiDepth));
        displayText.append(L"\n");
        displayText.append(core::stringw(guiTime.c_str()));
        if (guiPaused) {
            displayText.append(L"\n");
            displayText.append(L"Paused"); //i18n
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

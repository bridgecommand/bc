/*   Bridge Command 5.0 Ship Simulator
     Copyright (C) 2024 James Packer

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

#ifndef __IMPORTEXPORTGUI_HPP_INCLUDED__
#define __IMPORTEXPORTGUI_HPP_INCLUDED__

#include "irrlicht.h"
#include <string>

#include "../Lang.hpp"

class GUIImportExport {
    
public:
    
    GUIImportExport(
        irr::IrrlichtDevice* device, 
        Lang* language, 
        irr::u32 su, 
        irr::u32 sh,
        irr::s32 importExportOKButtonID);
    void setVisible(bool isVisible, irr::u32 importExportMode); //importExportMode: 0 = export, 1 = import
    void setText(std::string text);
    std::string getText() const;
    irr::u32 getMode() const;
private:

    Lang* language;

    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* guienv;
    irr::gui::IGUIWindow* importExportWindow;
    irr::gui::IGUIEditBox* importExportText;
    irr::gui::IGUIButton* importExportOKButton;
    irr::gui::IGUIStaticText* importDescriptionText;
    irr::gui::IGUIStaticText* exportDescriptionText;

    irr::u32 importExportMode; // 0 = export, 1 = import
    
};

#endif /* __IMPORTEXPORTGUI_HPP_INCLUDED__ */

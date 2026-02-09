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

#include <string>
#include <cstdint>

#include "../Lang.hpp"

namespace irr { class IrrlichtDevice; namespace gui { class IGUIEnvironment; class IGUIWindow; class IGUIEditBox; class IGUIButton; class IGUIStaticText; } }

class GUIImportExport {

public:

    GUIImportExport(
        irr::IrrlichtDevice* device,
        Lang* language,
        uint32_t su,
        uint32_t sh,
        int32_t importExportOKButtonID);
    void setVisible(bool isVisible, uint32_t importExportMode); //importExportMode: 0 = export, 1 = import
    void setText(std::string text);
    std::string getText() const;
    uint32_t getMode() const;
private:

    Lang* language;

    irr::IrrlichtDevice* device;
    irr::gui::IGUIEnvironment* guienv;
    irr::gui::IGUIWindow* importExportWindow;
    irr::gui::IGUIEditBox* importExportText;
    irr::gui::IGUIButton* importExportOKButton;
    irr::gui::IGUIStaticText* importDescriptionText;
    irr::gui::IGUIStaticText* exportDescriptionText;

    uint32_t importExportMode; // 0 = export, 1 = import

};

#endif /* __IMPORTEXPORTGUI_HPP_INCLUDED__ */

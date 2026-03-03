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

#include "ImportExportGUI.hpp"


GUIImportExport::GUIImportExport(
    irr::IrrlichtDevice* device, 
    Lang* language, 
    irr::u32 su, 
    irr::u32 sh,
    irr::s32 importExportOKButtonID){
    this->device = device;
    this->language = language;
    guienv = device->getGUIEnvironment();

    importExportMode = 0;

    importExportWindow = guienv->addWindow(irr::core::rect<irr::s32>(0.01*su, 0.01*sh, 0.99*su, 0.99*sh), true);
    if (importExportWindow) {
        importExportWindow->getCloseButton()->setVisible(false);

        importDescriptionText = guienv->addStaticText(language->translate("importDescription").c_str(), irr::core::rect<irr::s32>(0.06*su,0.100*sh,0.920*su,0.200*sh), false, true, importExportWindow);
        exportDescriptionText = guienv->addStaticText(language->translate("exportDescription").c_str(), irr::core::rect<irr::s32>(0.06*su,0.100*sh,0.920*su,0.200*sh), false, true, importExportWindow);
        
        importExportText = guienv->addEditBox(L"",irr::core::rect<irr::s32>(0.06*su,0.200*sh,0.920*su,0.80*sh), true, importExportWindow);
        if (importExportText) {
            importExportText->setMultiLine(true);
            importExportText->setWordWrap(true);
            importExportText->setTextAlignment(irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_UPPERLEFT);
        }

        guienv->addButton(irr::core::rect<irr::s32>(0.06*su,0.810*sh,0.920*su,0.860*sh), importExportWindow, importExportOKButtonID, language->translate("ok").c_str());
    } else {
        importExportText = 0;
        importDescriptionText = 0;
        exportDescriptionText = 0;

    }
}

void GUIImportExport::setVisible(bool isVisible, irr::u32 importExportMode)
{
    if (importExportWindow) {
        importExportWindow->setVisible(isVisible);
    }

    this->importExportMode = importExportMode;

    if (importExportMode == 0) {
        importDescriptionText->setVisible(false);
        exportDescriptionText->setVisible(true);
    } else {
        importDescriptionText->setVisible(true);
        exportDescriptionText->setVisible(false);
    }
}

void GUIImportExport::setText(std::string text)
{
    if (importExportText) {
        importExportText->setText(irr::core::stringw(text.c_str()).c_str());
    }
}

std::string GUIImportExport::getText() const
{
    if (importExportText) {
        std::wstring wideText(importExportText->getText());
        std::string text(wideText.begin(),wideText.end());
        return text;
    } else {
        return "";
    }
}

irr::u32 GUIImportExport::getMode() const
{
    return importExportMode;
}

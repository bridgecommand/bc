;setup names
!define PROGRAMNAME "Bridge Command 5.0 Alpha 5"
!define OUTPUTFILE "bc50a5_setup.exe"
!define INSTALLLOCATION "Bridge Command 5.0a5"
!define SMFOLDER "Bridge Command 5.0 Alpha 5"
!define REGKEY "BridgeCommand5.0a5"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SetCompressor /SOLID lzma

Name "${PROGRAMNAME}"

OutFile "${OUTPUTFILE}"

InstallDir "$PROGRAMFILES\${INSTALLLOCATION}"

LicenseData "LICENSE.txt"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

Page license
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Bridge Command (required)"

;set to all users for start menu
SetShellVarContext all

SectionIn RO

SetOutPath $INSTDIR

;include all files, excluding the .svn directories
File /r /x .svn /x .objs /x .git /x EnetServer /x BridgeCommand.app /x MapController.app /x *.m /x *.nsi /x *.cscope_file_list /x RadarCache /x misc /x shiplights.ods /x gmon.out /x cscope.out *.*

  CreateDirectory "$SMPROGRAMS\${SMFOLDER}"
  CreateShortCut "$SMPROGRAMS\${SMFOLDER}\${PROGRAMNAME}.lnk" "$INSTDIR\launcher.exe"
  CreateShortCut "$SMPROGRAMS\${SMFOLDER}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGKEY}" "DisplayName" "${PROGRAMNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGKEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGKEY}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGKEY}" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

Section "Uninstall"

;set to all users for start menu
SetShellVarContext all
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${REGKEY}"

  ; Remove files and uninstaller
  RMDir /r "$INSTDIR\controller"
  RMDir /r "$INSTDIR\Models"  
  RMDir /r "$INSTDIR\Scenarios"
  RMDir /r "$INSTDIR\media"
  RMDir /r "$INSTDIR\world"
  RMDir /r "$INSTDIR\libs"
  RMDir /r "$INSTDIR\shaders"
  RMDir /r "$INSTDIR\launcher"

  Delete "$INSTDIR\Leg.hpp"
  Delete "$INSTDIR\Sky.hpp"
  Delete "$INSTDIR\Angles.hpp"
  Delete "$INSTDIR\Water.hpp"
  Delete "$INSTDIR\Network.hpp"
  Delete "$INSTDIR\NetworkPrimary.hpp"
  Delete "$INSTDIR\NetworkSecondary.hpp"
  Delete "$INSTDIR\Light.hpp"
  Delete "$INSTDIR\IniFile.hpp"
  Delete "$INSTDIR\StartupEventReceiver.hpp"
  Delete "$INSTDIR\RadarScreen.hpp"
  Delete "$INSTDIR\LandObject.hpp"
  Delete "$INSTDIR\MyEventReceiver.hpp"
  Delete "$INSTDIR\Tide.hpp"
  Delete "$INSTDIR\LandObjects.hpp"
  Delete "$INSTDIR\ScenarioChoice.hpp"
  Delete "$INSTDIR\RadarData.hpp"
  Delete "$INSTDIR\Sky.cpp"
  Delete "$INSTDIR\Utilities.hpp"
  Delete "$INSTDIR\LandLights.hpp"
  Delete "$INSTDIR\Constants.hpp"
  Delete "$INSTDIR\Terrain.hpp"
  Delete "$INSTDIR\Buoys.hpp"
  Delete "$INSTDIR\Camera.hpp"
  Delete "$INSTDIR\OtherShips.hpp"
  Delete "$INSTDIR\Angles.cpp"
  Delete "$INSTDIR\Buoy.hpp"
  Delete "$INSTDIR\NavLight.hpp"
  Delete "$INSTDIR\OtherShip.hpp"
  Delete "$INSTDIR\StartupEventReceiver.cpp"
  Delete "$INSTDIR\Ship.hpp"
  Delete "$INSTDIR\RadarCalculation.hpp"
  Delete "$INSTDIR\GUIMain.hpp"
  Delete "$INSTDIR\Ship.cpp"
  Delete "$INSTDIR\RadarScreen.cpp"
  Delete "$INSTDIR\Utilities.cpp"
  Delete "$INSTDIR\Light.cpp"
  Delete "$INSTDIR\OwnShip.hpp"
  Delete "$INSTDIR\main.cpp"
  Delete "$INSTDIR\BridgeCommand.cbp"
  Delete "$INSTDIR\bc5.ini"
  Delete "$INSTDIR\Structure.txt"
  Delete "$INSTDIR\Tide.cpp"
  Delete "$INSTDIR\LandObjects.cpp"
  Delete "$INSTDIR\Camera.cpp"
  Delete "$INSTDIR\LandObject.cpp"
  Delete "$INSTDIR\SimulationModel.hpp"
  Delete "$INSTDIR\LandLights.cpp"
  Delete "$INSTDIR\Network.cpp"
  Delete "$INSTDIR\NetworkPrimary.cpp"
  Delete "$INSTDIR\NetworkSecondary.cpp"
  Delete "$INSTDIR\Water.cpp"
  Delete "$INSTDIR\OtherShips.cpp"
  Delete "$INSTDIR\install.nsi"
  Delete "$INSTDIR\IniFile.cpp"
  Delete "$INSTDIR\GUIMain.cpp"
  Delete "$INSTDIR\ScenarioChoice.cpp"
  Delete "$INSTDIR\Buoys.cpp"
  Delete "$INSTDIR\Terrain.cpp"
  Delete "$INSTDIR\MyEventReceiver.cpp"
  Delete "$INSTDIR\NavLight.cpp"
  Delete "$INSTDIR\Buoy.cpp"
  Delete "$INSTDIR\OtherShip.cpp"
  Delete "$INSTDIR\SimulationModel.cpp"
  Delete "$INSTDIR\OwnShip.cpp"
  Delete "$INSTDIR\BridgeCommand.layout"
  Delete "$INSTDIR\RadarCalculation.cpp"
  Delete "$INSTDIR\RealisticWater.h" 
  Delete "$INSTDIR\RealisticWater.cpp"
  Delete "$INSTDIR\LICENSE.txt"
  Delete "$INSTDIR\libenet.dll"
  Delete "$INSTDIR\BridgeCommand.depend"
  Delete "$INSTDIR\BridgeCommand.exe"
  Delete "$INSTDIR\controller.exe"
  Delete "$INSTDIR\Irrlicht.dll"
  Delete "$INSTDIR\uninstall.exe"
  Delete "$INSTDIR\IniEditor.exe"
  Delete "$INSTDIR\Makefile"
  Delete "$INSTDIR\CompilingLinuxAndMac.txt"
  Delete "$INSTDIR\language.txt"
  Delete "$INSTDIR\languageController.txt"
  Delete "$INSTDIR\languageLauncher.txt"
  Delete "$INSTDIR\Lang.cpp"
  Delete "$INSTDIR\Lang.hpp"
  Delete "$INSTDIR\map.ini"
  Delete "$INSTDIR\NMEA.cpp"
  Delete "$INSTDIR\NMEA.hpp"
  Delete "$INSTDIR\Rain.cpp"
  Delete "$INSTDIR\Rain.hpp"
  Delete "$INSTDIR\README"
  Delete "$INSTDIR\Icon.ico"
  Delete "$INSTDIR\launcher.exe"

  ; Remove shortcuts, if any
  ;Delete "$SMPROGRAMS\${SMFOLDER}\Settings\*.*"
  Delete "$SMPROGRAMS\${SMFOLDER}\*.*"

  ; Remove directories used
  ;RMDir "$SMPROGRAMS\${SMFOLDER}\Settings"
  RMDir "$SMPROGRAMS\${SMFOLDER}"
  RMDir "$INSTDIR"

SectionEnd


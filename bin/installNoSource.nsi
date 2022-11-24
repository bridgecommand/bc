;setup names
!define PROGRAMNAME "Bridge Command 5.8"
!ifndef OUTPUTFILE
!define OUTPUTFILE "..\BridgeCommand5.8.0.exe"
!endif
!define INSTALLLOCATION "Bridge Command 5.8"
!define SMFOLDER "Bridge Command 5.8"
!define REGKEY "BridgeCommand5.8"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SetCompressor /SOLID lzma

Name "${PROGRAMNAME}"

OutFile "${OUTPUTFILE}"

InstallDir "$PROGRAMFILES\${INSTALLLOCATION}"

LicenseData "LICENSE"

Icon "..\src\Icon.ico"

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
File /r /x *.cpp /x *.hpp /x *.h /x *.rc /x *.bat /x *.aps /x *.depend /x *.layout /x *.cbp /x *.iobj /x *.ipdb /x *.pdb /x *.tmp /x *.gcc /x macOScopy /x makeAndBuildApp /x CMakeLists.txt /x createDeb /x "Visual Studio 2017 solution" /x "Visual Studio 2017 solution for XP" /x Irrlicht_mingw.dll /x Irrlicht_VS.dll /x CompilingLinuxAndMac.txt /x Makefile /x MakefileWithSound /x MakefileForDeb /x controller /x repeater /x editor /x launcher /x iniEditor /x multiplayerHub /x libs /x .svn /x .objs /x .git /x .gitignore /x EnetServer /x BridgeCommand.app /x *.db /x *.m /x *.nsi /x *.cscope_file_list /x RadarCache /x misc /x shiplights.ods /x gmon.out /x cscope.out /x Cubemaps_HLSL_Test /x Portsmouth /x StraitOfJuanDeFuca /x "h) Haro Strait" *.*
File /r ..\doc

  CreateDirectory "$SMPROGRAMS\${SMFOLDER}"
  CreateShortCut "$SMPROGRAMS\${SMFOLDER}\${PROGRAMNAME}.lnk" "$INSTDIR\bridgecommand.exe"
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
  RMDir /r "$INSTDIR\Models"  
  RMDir /r "$INSTDIR\Scenarios"
  RMDir /r "$INSTDIR\media"
  RMDir /r "$INSTDIR\world"
  RMDir /r "$INSTDIR\shaders"
  RMDir /r "$INSTDIR\doc"
  RMDir /r "$INSTDIR\Sounds"
  
  Delete "$INSTDIR\bc5.ini"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\bridgecommand.exe"
  Delete "$INSTDIR\bridgecommand-bc.exe"
  Delete "$INSTDIR\bridgecommand-mc.exe"
  Delete "$INSTDIR\bridgecommand-rp.exe"
  Delete "$INSTDIR\bridgecommand-mh.exe"
  Delete "$INSTDIR\bridgecommand-ed.exe"
  Delete "$INSTDIR\bridgecommand-ini.exe"
  Delete "$INSTDIR\bridgecommand-mh.exe"
  Delete "$INSTDIR\Irrlicht.dll"
  Delete "$INSTDIR\libsndfile-1.dll"
  Delete "$INSTDIR\portaudio_x86.dll"
  Delete "$INSTDIR\libenet.dll"
  Delete "$INSTDIR\uninstall.exe"
  Delete "$INSTDIR\language-en.txt"
  Delete "$INSTDIR\languageController-en.txt"
  Delete "$INSTDIR\languageRepeater-en.txt"
  Delete "$INSTDIR\languageLauncher-en.txt"
  Delete "$INSTDIR\languageIniEditor-en.txt"
  Delete "$INSTDIR\languageMultiplayer-en.txt"
  Delete "$INSTDIR\mph.ini"
  Delete "$INSTDIR\map.ini"
  Delete "$INSTDIR\repeater.ini"
  Delete "$INSTDIR\README"
  Delete "$INSTDIR\Icon.ico"

  ; Remove shortcuts, if any
  ;Delete "$SMPROGRAMS\${SMFOLDER}\Settings\*.*"
  Delete "$SMPROGRAMS\${SMFOLDER}\*.*"

  ; Remove directories used
  ;RMDir "$SMPROGRAMS\${SMFOLDER}\Settings"
  RMDir "$SMPROGRAMS\${SMFOLDER}"
  RMDir "$INSTDIR"

SectionEnd


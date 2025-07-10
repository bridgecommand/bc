;setup names
!define PROGRAMNAME "Bridge Command 5.10.3 SOMOS"
!ifndef OUTPUTFILE
!define OUTPUTFILE "..\..\BridgeCommand5.10.3-SOMOS.exe"
!endif
!define INSTALLLOCATION "Bridge Command 5.10.3 SOMOS"
!define SMFOLDER "Bridge Command 5.10.3 SOMOS"
!define REGKEY "BridgeCommand5.10.3SOMOS"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SetCompressor /SOLID lzma

Name "${PROGRAMNAME}"

OutFile "${OUTPUTFILE}"

InstallDir "$PROGRAMFILES64\${INSTALLLOCATION}"

LicenseData "..\LICENSE"

Icon "..\resources\icon\Icon.ico"

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
File /r /x *.cpp /x *.hpp /x *.h /x *.rc /x *.bat /x *.aps /x *.depend /x *.layout /x *.cbp /x *.iobj /x *.ipdb /x *.pdb /x *.tmp /x *.gcc /x macOScopy /x makeAndBuildApp /x CMakeLists.txt /x createDeb /x "Visual Studio solution" /x Irrlicht_mingw.dll /x Irrlicht_mingw64.dll /x Irrlicht_VS.dll /x Irrlicht_VS64.dll /x libenet32.dll /x libenet64.dll /x CompilingLinuxAndMac.txt /x Makefile /x MakefileWithSound /x MakefileForDeb /x controller /x repeater /x editor /x launcher /x iniEditor /x multiplayerHub /x libs /x .svn /x .objs /x .git /x .gitignore /x EnetServer /x BridgeCommand.app /x *.db /x *.m /x *.nsi /x *.cscope_file_list /x RadarCache /x misc /x shiplights.ods /x gmon.out /x cscope.out /x Cubemaps_HLSL_Test /x Portsmouth /x StraitOfJuanDeFuca /x "h) Haro Strait" ..\*.*
;File /r /x src ..\doc

SetOutPath $INSTDIR\bin\win\

  CreateDirectory "$SMPROGRAMS\${SMFOLDER}"
  CreateShortCut "$SMPROGRAMS\${SMFOLDER}\${PROGRAMNAME}.lnk" "$INSTDIR\bin\win\bridgecommand.exe"
  CreateShortCut "$SMPROGRAMS\${SMFOLDER}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

SetOutPath $INSTDIR

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
  RMDir /r "$INSTDIR\*"  

  
  Delete "$INSTDIR\*.*"

  ; Remove shortcuts, if any
  ;Delete "$SMPROGRAMS\${SMFOLDER}\Settings\*.*"
  Delete "$SMPROGRAMS\${SMFOLDER}\*.*"

  ; Remove directories used
  ;RMDir "$SMPROGRAMS\${SMFOLDER}\Settings"
  RMDir "$SMPROGRAMS\${SMFOLDER}"
  RMDir "$INSTDIR"

SectionEnd


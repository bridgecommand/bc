;setup names
!define PROGRAMNAME "Bridge Command 5.0 Beta 1"
!define OUTPUTFILE "bc50b1_setup_noSource.exe"
!define INSTALLLOCATION "Bridge Command 5.0b1"
!define SMFOLDER "Bridge Command 5.0 Beta 1"
!define REGKEY "BridgeCommand5.0b1"

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
File /r /x *.cpp /x *.hpp /x *.h /x *.depend /x *.layout /x *.cbp /x CompilingLinuxAndMac.txt /x Makefile /x controller /x editor /x launcher /x libs /x .svn /x .objs /x .git /x EnetServer /x BridgeCommand.app /x MapController.app /x *.db /x *.m /x *.nsi /x *.cscope_file_list /x RadarCache /x misc /x shiplights.ods /x gmon.out /x cscope.out *.*

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
  RMDir /r "$INSTDIR\Models"  
  RMDir /r "$INSTDIR\Scenarios"
  RMDir /r "$INSTDIR\media"
  RMDir /r "$INSTDIR\world"
  RMDir /r "$INSTDIR\shaders"
  
  Delete "$INSTDIR\bc5.ini"
  Delete "$INSTDIR\LICENSE.txt"
  Delete "$INSTDIR\libenet.dll"
  Delete "$INSTDIR\BridgeCommand.exe"
  Delete "$INSTDIR\controller.exe"
  Delete "$INSTDIR\editor.exe"
  Delete "$INSTDIR\Irrlicht.dll"
  Delete "$INSTDIR\uninstall.exe"
  Delete "$INSTDIR\IniEditor.exe"
  Delete "$INSTDIR\language.txt"
  Delete "$INSTDIR\languageController.txt"
  Delete "$INSTDIR\languageLauncher.txt"
  Delete "$INSTDIR\map.ini"
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


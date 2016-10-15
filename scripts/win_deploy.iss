#define DisplayAppName "Auto Panorama"
#define AppName "AutoPanorama"  
#define AppNameExe "AutoPanorama.exe"
#define Version "1.0.0-rc1"  
#define DeployFolder "windows"

[Setup]
AppName={#AppName}
AppVersion={#Version}
DefaultDirName={pf}\{#AppName}
DefaultGroupName={#DisplayAppName}
UninstallDisplayIcon={app}\{#AppNameExe}
Compression=lzma2
SolidCompression=yes   
OutputDir=..\{#DeployFolder}\
OutputBaseFilename={#AppName}_{#Version}_Installer

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; \
    GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "..\{#DeployFolder}\{#AppNameExe}"; DestDir: "{app}"    
Source: "..\{#DeployFolder}\*.dll"; DestDir: "{app}"       
Source: "..\{#DeployFolder}\imageformats\*"; DestDir: "{app}\imageformats\"
Source: "..\{#DeployFolder}\platforms\*"; DestDir: "{app}\platforms\"

[Icons]
Name: "{commondesktop}\{#DisplayAppName}"; Filename: "{app}\{#AppNameExe}"; Tasks: desktopicon
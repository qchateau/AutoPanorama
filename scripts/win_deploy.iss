#define DisplayAppName "Auto Panorama"
#define AppName "AutoPanorama"  
#define AppNameExe "AutoPanorama.exe"
#define Arch "x64"
#define DeployFolder "..\windows\"+Arch+"\"
#define OutFolder "..\windows"   
#define Version GetFileVersion(DeployFolder + AppNameExe)

#if Arch == "x64"
#define PF "{pf64}"
#else   
#define PF "{pf32}"
#endif

[Setup]
AppName={#AppName}
AppVersion={#Version}_{#Arch}
DefaultDirName={#PF}\{#AppName}
DefaultGroupName={#DisplayAppName}
UninstallDisplayIcon={app}\{#AppNameExe}
Compression=lzma2
SolidCompression=yes   
OutputDir={#OutFolder}
OutputBaseFilename={#AppName}_{#Version}_{#Arch}_Installer
VersionInfoVersion={#Version}

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; \
    GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "{#DeployFolder}\{#AppNameExe}"; DestDir: "{app}"    
Source: "{#DeployFolder}\*.dll"; DestDir: "{app}"       
Source: "{#DeployFolder}\imageformats\*"; DestDir: "{app}\imageformats\"
Source: "{#DeployFolder}\platforms\*"; DestDir: "{app}\platforms\"

[Icons]
Name: "{commondesktop}\{#DisplayAppName}"; Filename: "{app}\{#AppNameExe}"; Tasks: desktopicon      
Name: "{commonprograms}\{#DisplayAppName}"; Filename: "{app}\{#AppNameExe}";
Name: "{commonstartup}\{#DisplayAppName}"; Filename: "{app}\{#AppNameExe}";

[Setup]
AppName=EOLEAPP
AppVersion=1.6
DefaultDirName={commonpf}\EOLEAPP
UsePreviousAppDir=no
DefaultGroupName=EOLEAPP
OutputDir=.\Installer
OutputBaseFilename=EOLEAPP_SetUp_V_1_6
Compression=lzma
SolidCompression=yes

[Files]
Source: "C:\Users\JavierLagodeDiego\OneDrive - Sensia Solutions SL\Escritorio\EoleAPP\EOLEAPP\icono.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\JavierLagodeDiego\Desktop\EoleAPP\EOLEAPP\build\Desktop_Qt_6_8_2_MinGW_64_bit-Release\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{group}\EOLEAPP"; Filename: "{app}\EOLEAPP.exe"; IconFilename: "{app}\icono.ico"
Name: "{commondesktop}\EOLEAPP"; Filename: "{app}\EOLEAPP.exe"; IconFilename: "{app}\icono.ico"

[Run]
Filename: "{app}\EOLEAPP.exe"; Description: "Launch EOLEAPPToolkit"; Flags: nowait postinstall

; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CKdNetCloneSrvDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "KdNetCloneSrv.h"

ClassCount=4
Class1=CKdNetCloneSrvApp
Class2=CKdNetCloneSrvDlg

ResourceCount=4
Resource2=IDD_DIALOG_SERVER
Resource1=IDR_MAINFRAME
Class3=CServerDlg
Resource3=IDD_KDNETCLONESRV_DIALOG
Class4=CNetCardListDialog
Resource4=IDD_DIALOG_NETCARDLIST

[CLS:CKdNetCloneSrvApp]
Type=0
HeaderFile=KdNetCloneSrv.h
ImplementationFile=KdNetCloneSrv.cpp
Filter=N
BaseClass=CWinApp
VirtualFilter=AC

[CLS:CKdNetCloneSrvDlg]
Type=0
HeaderFile=KdNetCloneSrvDlg.h
ImplementationFile=KdNetCloneSrvDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_BUTTON_START



[DLG:IDD_KDNETCLONESRV_DIALOG]
Type=1
Class=CKdNetCloneSrvDlg
ControlCount=12
Control1=IDC_COMBO_PARTLIST,combobox,1344340227
Control2=IDC_RADIO_DISK,button,1342177289
Control3=IDC_RADIO_PART,button,1342177289
Control4=IDC_STATIC,button,1342177287
Control5=IDC_STATIC,button,1342177287
Control6=IDC_EDIT_PATH,edit,1350631552
Control7=IDC_BUTTON_BROWSE,button,1342242816
Control8=IDC_BUTTON_START,button,1342242816
Control9=IDC_STATIC,static,1342308352
Control10=IDC_COMBO_GHOPARTLIST,combobox,1344340227
Control11=IDC_STATIC,static,1342308352
Control12=IDC_BUTTON_EXIT,button,1342242816

[DLG:IDD_DIALOG_SERVER]
Type=1
Class=CServerDlg
ControlCount=0

[CLS:CServerDlg]
Type=0
HeaderFile=ServerDlg.h
ImplementationFile=ServerDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CServerDlg
VirtualFilter=dWC

[DLG:IDD_DIALOG_NETCARDLIST]
Type=1
Class=CNetCardListDialog
ControlCount=8
Control1=IDC_COMBO_NETCARDLIST,combobox,1344340227
Control2=IDC_STATIC,button,1342177287
Control3=IDC_STATIC,button,1342177287
Control4=IDC_STATIC_DEVICEDESC,static,1342308352
Control5=IDC_STATIC_DHCPTYPE,static,1342308352
Control6=IDC_STATIC_NETMASK,static,1342308352
Control7=IDC_STATIC_IPADDRESS,static,1342308352
Control8=IDC_BUTTON_OK,button,1342242816

[CLS:CNetCardListDialog]
Type=0
HeaderFile=NetCardListDialog.h
ImplementationFile=NetCardListDialog.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_STATIC_IPADDRESS
VirtualFilter=dWC


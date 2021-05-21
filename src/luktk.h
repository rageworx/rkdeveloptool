#ifndef __LUKTOOLKIT_H__
#define __LUKTOOLKIT_H__

#include "DefineHeader.h"

bool luktk_GetTestDevice( KLST_HANDLE* DeviceList, KLST_DEVINFO_HANDLE* DeviceInfo, USHORT vid, USHORT pid );
bool luktk_GetTestDeviceEx( KLST_HANDLE* DeviceList, KLST_DEVINFO_HANDLE* DeviceInfo, USHORT vid, USHORT pid, KLST_FLAG Flags );

#endif /// of __LUKTOOLKIT_H__
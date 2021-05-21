#include <windows.h>
#include <libusbk.h>
#include "luktk.h"

bool luktk_GetTestDevice( KLST_HANDLE* DeviceList,
                          KLST_DEVINFO_HANDLE* DeviceInfo,
                          USHORT vid,
                          USHORT pid )
{
	return luktk_GetTestDeviceEx(DeviceList,
	                                DeviceInfo,
	                                vid,
	                                pid,
	                                KLST_FLAG_NONE );

}

bool luktk_GetTestDeviceEx( KLST_HANDLE* DeviceList,
                            KLST_DEVINFO_HANDLE* DeviceInfo,
                            USHORT vid,
                            USHORT pid,
                            KLST_FLAG Flags)
{
    if ( ( vid == 0 ) ||  ( pid == 0 ) )
    {
#ifdef DEBUG
		printf("luktk debug: Error no VID and PID in parameter.\n");
#endif
		return FALSE;
    }

	USHORT vidArg = 0;
	USHORT pidArg = 0;
	UINT   deviceCount = 0L;
	KLST_HANDLE deviceList = NULL;
	KLST_DEVINFO_HANDLE deviceInfo = NULL;

	// init
	*DeviceList = NULL;
	*DeviceInfo = NULL;

    if ( vid > 0 )
    {
        vidArg = vid;
    }
    
    if ( pid  > 0 )
    {
        pidArg = pid;
    }
    
	// Get the device list
	if (!LstK_Init(&deviceList, Flags))
	{
#ifdef DEBUG
		printf("luktk debug: Error initializing device list.\n");
#endif
		return FALSE;
	}

	LstK_Count( deviceList, &deviceCount );
    
	if ( deviceCount == 0 )
	{
#ifdef DEBUG
		printf("luktk debug: Device list empty.\n");
#endif
		SetLastError(ERROR_DEVICE_NOT_CONNECTED);

		// If LstK_Init returns TRUE, the list must be freed.
		LstK_Free(deviceList);

		return false;
	}

#ifdef DEBUGG
	printf("lukti debug: Looking for device vid/pid %04X/%04X..\n", vidArg, pidArg);
#endif

	LstK_FindByVidPid(deviceList, vidArg, pidArg, &deviceInfo);

	if (deviceInfo)
	{
		// This function returns the device list and the device info
		// element which matched.  The caller is responsible for freeing
		// this list when it is no longer needed.
		*DeviceList = deviceList;
		*DeviceInfo = deviceInfo;

#ifdef DEBUG
		// Report the connection state of the example device
		printf("Using %04X:%04X (%s): %s - %s\n",
		       deviceInfo->Common.Vid,
		       deviceInfo->Common.Pid,
		       deviceInfo->Common.InstanceID,
		       deviceInfo->DeviceDesc,
		       deviceInfo->Mfg);
#endif

		return true;
	}
	
    // If LstK_Init returns TRUE, the list must be freed.
    LstK_Free(deviceList);

    return false;
}
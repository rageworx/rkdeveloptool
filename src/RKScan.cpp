/*
 * (C) Copyright 2017 Fuzhou Rockchip Electronics Co., Ltd
 * Seth Liu 2017.03.01
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include "RKScan.h"
#include "luktk.h"

#define BUSID(id)  ((id & 0x0000FF00) >> 8)

int CRKScan::GetDEVICE_COUNTS()
{
    return m_list.size();
}

UINT CRKScan::GetMSC_TIMEOUT()
{
    return m_waitMscSecond;
}

UINT CRKScan::GetRKUSB_TIMEOUT()
{
    return m_waitRKusbSecond;
}

void CRKScan::SetMSC_TIMEOUT(UINT value)
{
    m_waitMscSecond = value;
}

void CRKScan::SetRKUSB_TIMEOUT(UINT value)
{
    m_waitRKusbSecond = value;
}

CRKScan::CRKScan( void* usbkhandle, UINT uiMscTimeout, UINT uiRKusbTimeout )
 : m_usbkHandle( usbkhandle ),
   m_waitMscSecond( uiMscTimeout ),
   m_waitRKusbSecond( uiRKusbTimeout ),
   m_log( NULL )
{
    DEVICE_COUNTS.setContainer(this);
    DEVICE_COUNTS.getter(&CRKScan::GetDEVICE_COUNTS);

    MSC_TIMEOUT.setContainer(this);
    MSC_TIMEOUT.getter(&CRKScan::GetMSC_TIMEOUT);
    MSC_TIMEOUT.setter(&CRKScan::SetMSC_TIMEOUT);

    RKUSB_TIMEOUT.setContainer(this);
    RKUSB_TIMEOUT.getter(&CRKScan::GetRKUSB_TIMEOUT);
    RKUSB_TIMEOUT.setter(&CRKScan::SetRKUSB_TIMEOUT);

    m_list.clear();
    m_deviceConfigSet.clear();
    m_deviceMscConfigSet.clear();
}

bool CRKScan::FindRockusbVidPid(ENUM_RKDEVICE_TYPE type, USHORT &usVid, USHORT &usPid)
{
    bool bRet = false;

    for ( size_t i = 0; i < m_deviceConfigSet.size(); i++ ) 
    {
        if (m_deviceConfigSet[i].emDeviceType == type) 
        {
            usVid = m_deviceConfigSet[i].usVid;
            usPid = m_deviceConfigSet[i].usPid;
            bRet = true;
            break;
        }
    }
    
    return bRet;
}

void CRKScan::AddRockusbVidPid(USHORT newVid, USHORT newPid, USHORT oldVid, USHORT oldPid)
{
    if ((newVid == 0) || (newPid == 0) || (oldVid == 0) || (oldPid == 0)) 
    {
        return;
    }
    
    size_t i = 0;
    STRUCT_DEVICE_CONFIG config;

    for ( i = 0; i < m_deviceConfigSet.size(); i++) 
    {
        if ((m_deviceConfigSet[i].usVid == oldVid) && (m_deviceConfigSet[i].usPid == oldPid)) 
        {
            config.usVid = newVid;
            config.usPid = newPid;
            config.emDeviceType = m_deviceConfigSet[i].emDeviceType;
            break;
        }
    }
    
    if (i < m_deviceConfigSet.size())
        m_deviceConfigSet.push_back(config);
}

void CRKScan::SetVidPid(USHORT mscVid, USHORT mscPid)
{
    STRUCT_DEVICE_CONFIG config;
    
    if ( m_deviceConfigSet.size() > 0 )
    {
        m_deviceConfigSet.clear();
    }

    if ((mscVid != 0) || (mscPid != 0)) 
    {
        if (FindConfigSetPos(m_deviceMscConfigSet, mscVid, mscPid) == -1) 
        {
            config.emDeviceType = RKNONE_DEVICE;
            config.usPid = mscPid;
            config.usVid = mscVid;
            m_deviceMscConfigSet.push_back(config);
        }
    }
}

int CRKScan::FindWaitSetPos(const RKDEVICE_CONFIG_SET &waitDeviceSet, USHORT vid, USHORT pid)
{
    for ( size_t i = 0; i < waitDeviceSet.size(); i++ ) 
    {
        if ( (vid == waitDeviceSet[i].usVid) && (pid == waitDeviceSet[i].usPid) ) 
        {
            return (int)i;
        }
    }
    
    return -1;
}

int CRKScan::FindConfigSetPos(RKDEVICE_CONFIG_SET &devConfigSet, USHORT vid, USHORT pid)
{
    for ( size_t i = 0; i < devConfigSet.size(); i++ ) 
    {
        if ( (vid == devConfigSet[i].usVid) && (pid == devConfigSet[i].usPid) ) 
        {
            return (int)i;
        }
    }
    
    return -1;
}

void CRKScan::EnumerateUsbDevice(RKDEVICE_DESC_SET &list, UINT &uiTotalMatchDevices)
{
    STRUCT_RKDEVICE_DESC    desc;
    KUSB_DRIVER_API         kusbAPI;
    
    KLST_HANDLE         hDevList = NULL;
    KLST_DEVINFO_HANDLE hDevInfo = NULL;
        
    if ( LstK_Init( &hDevList, KLST_FLAG_INCLUDE_DISCONNECT  ) == FALSE )
    {
        if ( m_log )
            m_log->Record( "Error, Lst_Init() failure.\n" );
        
        return;
    }
    
    UINT devcount = 0;
    
    LstK_Count( hDevList, &devcount );
    
    if ( devcount == 0 )
    {
        if ( m_log )
            m_log->Record( "Error, device not found !\n" );
        
        LstK_Free( hDevList );
        return;
    }
    
    // enumerate device list ....
    
    LstK_MoveReset( hDevList );
    while( LstK_MoveNext( hDevList, &hDevInfo ) )
    {
        if ( hDevInfo != NULL )
        {
            // Create a new instance for device handling.
            KLST_HANDLE* hDevList = NULL;
            KLST_DEVINFO_HANDLE* hDevInfoNew = NULL;

            desc.emDeviceType   = RKNONE_DEVICE;
            desc.emUsbType      = RKUSB_NONE;
            desc.pUsbHandle     = NULL;
            desc.usbcdUsb       = 0;
            desc.usVid          = hDevInfo->Common.Vid;
            desc.usPid          = hDevInfo->Common.Pid;
            desc.driverID       = hDevInfo->DriverID;
            desc.uiLocationID   = *(DWORD*)hDevInfo->SerialNumber;
            
            /* -- old types --
            desc.uiLocationID   = hDevInfo->BusNumber;
            desc.uiLocationID <<= 8;
            desc.uiLocationID  |= 0;            /// DeviceAddress;
            */
            
            // Get some descriptor from handle --
            if ( LibK_LoadDriverAPI( &kusbAPI, hDevInfo->DriverID ) == TRUE )
            {
                USB_DEVICE_DESCRIPTOR deviceDescriptor = {0};
                KUSB_SETUP_PACKET setupPacket;
                
                KUSB_HANDLE* pHandle = NULL;
                if ( kusbAPI.Init( pHandle, hDevInfo ) == FALSE )
                {
                    DWORD errorCode = GetLastError();
                    printf("Open device failed. Win32Error=%u (0x%08X)\n", errorCode, errorCode);
                    LstK_Free( hDevList );
                    return;
                }
                
                desc.pUsbHandle = pHandle;
                
                // Setup packets are always 8 bytes (64 bits)
                *((__int64*)&setupPacket) = 0;                    
                // Fill the setup packet.
                setupPacket.BmRequest.Dir       = BMREQUEST_DIR_DEVICE_TO_HOST;
                setupPacket.BmRequest.Type      = BMREQUEST_TYPE_STANDARD;
                setupPacket.BmRequest.Recipient = BMREQUEST_RECIPIENT_DEVICE;
                setupPacket.Value               = USB_DESCRIPTOR_MAKE_TYPE_AND_INDEX(USB_DESCRIPTOR_TYPE_DEVICE, 0);
                setupPacket.Request             = USB_REQUEST_GET_DESCRIPTOR;
                setupPacket.Length              = sizeof(deviceDescriptor);
                BOOL \
                success = kusbAPI.ControlTransfer( pHandle, 
                                                   *((WINUSB_SETUP_PACKET*)&setupPacket), 
                                                   (PUCHAR)&deviceDescriptor, 
                                                   sizeof(deviceDescriptor), 
                                                   NULL, NULL );
                
                desc.usbcdUsb = deviceDescriptor.bcdUSB;
            }
                                
            uiTotalMatchDevices++;
            list.push_back(desc);
        }
    } /// of while()
    
    LstK_Free( hDevList );
}

void CRKScan::FreeDeviceList(RKDEVICE_DESC_SET &list)
{
    device_list_iter iter;
    for (iter = list.begin(); iter != list.end(); iter++) 
    {
        if ((*iter).pUsbHandle) 
        {
            //libusb_unref_device((libusb_device *)((*iter).pUsbHandle));
            KUSB_HANDLE hDev = (KUSB_HANDLE)(*iter).pUsbHandle;
            UsbK_Free( hDev );
            (*iter).pUsbHandle = NULL;
        }
    }
    list.clear();
}

bool CRKScan::IsRockusbDevice(ENUM_RKDEVICE_TYPE &type, USHORT vid, USHORT pid)
{
    int iPos = FindConfigSetPos(m_deviceConfigSet, vid, pid);

    if (iPos != -1) 
    {
        type = m_deviceConfigSet[iPos].emDeviceType;
        return true;
    }

    if (vid == 0x2207) 
    {
        if ((pid >> 8) > 0) 
        {
            type = RKNONE_DEVICE;
            return true;
        }
    }
    
    return false;
}

int CRKScan::Search(UINT type)
{
    device_list_iter iter,new_iter;
    ENUM_RKDEVICE_TYPE devType;
    UINT uiTotalDevice;
    int iPos;

    FreeDeviceList(m_list);
    EnumerateUsbDevice( m_list, uiTotalDevice );

    for ( iter = m_list.begin(); iter != m_list.end(); ) 
    {
        if( (iPos = FindConfigSetPos(m_deviceMscConfigSet, (*iter).usVid, (*iter).usPid)) != -1 ) 
        {
            (*iter).emDeviceType = RKNONE_DEVICE;
            iter++;
            continue;
        } 
        else 
        if (IsRockusbDevice(devType, (*iter).usVid, (*iter).usPid) ) 
        {
            (*iter).emDeviceType = devType;
            iter++;
            continue;
        } 
        else 
        {
            if ((*iter).pUsbHandle) 
            {
                //libusb_unref_device((libusb_device *)((*iter).pUsbHandle));
                KUSB_HANDLE hDev = (KUSB_HANDLE)(*iter).pUsbHandle;
                UsbK_Free( hDev );                
                (*iter).pUsbHandle = NULL;
            }
            
            iter = m_list.erase(iter);
            uiTotalDevice--;
        }
    }

    if (m_list.size() <= 0) 
    {
        return 0;
    }

    if ( (type & RKUSB_MASKROM) == 0 ) 
    {
        for ( iter = m_list.begin(); iter != m_list.end(); ) 
        {
            if( (IsRockusbDevice(devType, (*iter).usVid, (*iter).usPid)) && (((*iter).usbcdUsb & 0x1) == 0) ) 
            {
                if ((*iter).pUsbHandle) 
                {
                    //libusb_unref_device((libusb_device *)((*iter).pUsbHandle));
                    KUSB_HANDLE hDev = (KUSB_HANDLE)(*iter).pUsbHandle;
                    UsbK_Free( hDev );
                    (*iter).pUsbHandle = NULL;
                }
                
                iter = m_list.erase(iter);
                uiTotalDevice--;
            } 
            else 
            {
                iter++;
                continue;
            }
        }
    }
    
    if (m_list.size() <= 0) 
    {
        return 0;
    }

    if ( (type & RKUSB_LOADER) == 0 ) 
    {
        for ( iter = m_list.begin(); iter != m_list.end(); ) 
        {
            if( (IsRockusbDevice(devType, (*iter).usVid, (*iter).usPid)) && (((*iter).usbcdUsb & 0x1) == 1) ) 
            {
                if ((*iter).pUsbHandle) 
                {
                    //libusb_unref_device((libusb_device *)((*iter).pUsbHandle));
                    KUSB_HANDLE hDev = (KUSB_HANDLE)(*iter).pUsbHandle;
                    UsbK_Free( hDev );                    
                    (*iter).pUsbHandle = NULL;
                }
                
                iter = m_list.erase(iter);
                uiTotalDevice--;
            } 
            else 
            {
                iter++;
                continue;
            }
        }
    }
    
    if (m_list.size() <= 0) 
    {
        return 0;
    }

    if ( (type & RKUSB_MSC) == 0 ) 
    {
        for ( iter = m_list.begin(); iter != m_list.end(); ) 
        {
            if(FindConfigSetPos(m_deviceMscConfigSet, (*iter).usVid, (*iter).usPid) != -1) 
            {
                if ((*iter).pUsbHandle) 
                {
                    //libusb_unref_device((libusb_device *)((*iter).pUsbHandle));
                    KUSB_HANDLE hDev = (KUSB_HANDLE)(*iter).pUsbHandle;
                    UsbK_Free( hDev );
                    (*iter).pUsbHandle = NULL;
                }
                
                iter = m_list.erase(iter);
                uiTotalDevice--;
            } 
            else 
            {
                iter++;
                continue;
            }
        }
    }
    
    if (m_list.size() <= 0) 
    {
        return 0;
    }

    for ( iter = m_list.begin(); iter != m_list.end(); iter++ ) 
    {
        if (FindConfigSetPos(m_deviceMscConfigSet, (*iter).usVid, (*iter).usPid) != -1) 
        {
            (*iter).emUsbType = RKUSB_MSC;
        } 
        else 
        {
            USHORT usTemp;
            usTemp = (*iter).usbcdUsb;
            usTemp= usTemp & 0x1;

            if ( usTemp == 0 )
                (*iter).emUsbType = RKUSB_MASKROM;
            else
                (*iter).emUsbType = RKUSB_LOADER;
        }
    }
    
    return m_list.size();
}

bool CRKScan::MutexWait(UINT_VECTOR &vecExistedDevice, STRUCT_RKDEVICE_DESC &device, ENUM_RKUSB_TYPE usbType, USHORT usVid, USHORT usPid)
{
    int uiWaitSecond;
    int iFoundCount = 0;
    UINT iRet;
    bool bFound = false;
    
    if (usbType == RKUSB_MSC)
        uiWaitSecond = m_waitMscSecond;
    else
        uiWaitSecond = m_waitRKusbSecond;

    time_t tmInit, tmNow;
    time(&tmInit);
    device.uiLocationID = 0;
    
    while( difftime(time(&tmNow), tmInit) <= uiWaitSecond ) 
    {
        device_list_iter iter;
        iRet = Search(RKUSB_MASKROM | RKUSB_LOADER | RKUSB_MSC);
        
        if ( iRet == vecExistedDevice.size() + 1 ) 
        {
            for ( size_t i = 0; i < vecExistedDevice.size(); i++) 
            {
                for (iter = m_list.begin(); iter != m_list.end(); ) 
                {
                    if ((*iter).uiLocationID == vecExistedDevice[i]) 
                    {
                        iter = m_list.erase(iter);
                    } 
                    else
                        iter++;
                }
            }
            
            if (m_list.size() != 1) 
            {
                device.uiLocationID = 0;
                iFoundCount = 0;
            } 
            else 
            {
                iter = m_list.begin();
                
                if (device.uiLocationID == 0) 
                {
                    iFoundCount++;
                    device.uiLocationID = (*iter).uiLocationID;
                } 
                else 
                {
                    if (device.uiLocationID == (*iter).uiLocationID) 
                    {
                        iFoundCount++;
                    } 
                    else 
                    {
                        device.uiLocationID = 0;
                        iFoundCount = 0;
                    }
                }
            }
        } 
        else 
        {
            device.uiLocationID = 0;
            iFoundCount = 0;
        }
        
        if (iFoundCount >= 10) 
        {
            bFound = true;
            break;
        }
    }
    
    if (!bFound) 
    {
        return false;
    }
    
    bFound = Wait(device, usbType, usVid, usPid);
    return bFound;
}

bool CRKScan::MutexWaitPrepare(UINT_VECTOR &vecExistedDevice, DWORD uiOfflineDevice)
{
    int iRet, iRet2;
    time_t timeInit;
    time_t timeNow;
    
    time(&timeInit);
    iRet = iRet2 =0;
    
    while ((time(&timeNow) - timeInit) <= 3) 
    {
        iRet = Search(RKUSB_MASKROM | RKUSB_LOADER | RKUSB_MSC);
        usleep(20000);
        iRet2 = Search(RKUSB_MASKROM | RKUSB_LOADER | RKUSB_MSC);
        
        if (iRet2 == iRet) 
        {
            break;
        }
    }
    
    if ((iRet <= 0) || (iRet2 != iRet)) 
    {
        return false;
    }
    
    vecExistedDevice.clear();
    bool bFound = false;
    
    for ( device_list_iter iter = m_list.begin(); iter != m_list.end(); iter++ ) 
    {
        if ((*iter).uiLocationID != uiOfflineDevice) 
        {
            vecExistedDevice.push_back((*iter).uiLocationID);
        } 
        else
            bFound = true;
    }

    return bFound;
}

bool CRKScan::Wait(STRUCT_RKDEVICE_DESC &device, ENUM_RKUSB_TYPE usbType, USHORT usVid, USHORT usPid)
{
    RKDEVICE_DESC_SET deviceList;
    ENUM_RKUSB_TYPE curDeviceType;
    ENUM_RKDEVICE_TYPE devType;
    UINT totalDevice;
    int uiWaitSecond;
    int iFoundCount = 0;
    bool bRet = false;
    
    if (usbType == RKUSB_MSC)
        uiWaitSecond = m_waitMscSecond;
    else
        uiWaitSecond = m_waitRKusbSecond;

    time_t tmInit, tmNow;
    time(&tmInit);

    while( difftime(time(&tmNow), tmInit) <= uiWaitSecond ) 
    {
        device_list_iter iter;
        FreeDeviceList(deviceList);
        EnumerateUsbDevice(deviceList, totalDevice);
        
        for ( iter = deviceList.begin(); iter != deviceList.end(); iter++ ) 
        {
            if ((BUSID((*iter).uiLocationID) != BUSID(device.uiLocationID)) ||
                ((BUSID((*iter).uiLocationID) == BUSID(device.uiLocationID)) && ((*iter).uiLocationID >= device.uiLocationID))) 
            {
                if ((usVid != 0) || (usPid != 0)) 
                {
                    if ( ((*iter).usVid != usVid) || ((*iter).usPid != usPid) )
                        continue;
                }
                
                if (IsRockusbDevice(devType, (*iter).usVid, (*iter).usPid)) 
                {
                    if ( ((*iter).usbcdUsb & 0x0001) == 0 )
                        curDeviceType = RKUSB_MASKROM;
                    else
                        curDeviceType = RKUSB_LOADER;
                } 
                else 
                if ( FindConfigSetPos(m_deviceMscConfigSet, (*iter).usVid, (*iter).usPid) != -1 ) 
                {
                    curDeviceType = RKUSB_MSC;
                } 
                else
                    curDeviceType = RKUSB_NONE;

                if ( curDeviceType == usbType ) 
                {
                    iFoundCount++;
                    break;
                }
            }
        }
        
        if ( iter == deviceList.end() ) 
        {
            iFoundCount = 0;
        }
        
        if ( iFoundCount >= 8 ) 
        {
            device.usVid = (*iter).usVid;
            device.usPid = (*iter).usPid;
            device.uiLocationID = (*iter).uiLocationID;
            device.pUsbHandle= (*iter).pUsbHandle;
            device.emUsbType = usbType;
            device.usbcdUsb = (*iter).usbcdUsb;
            //libusb_ref_device((libusb_device *)device.pUsbHandle);
            KUSB_HANDLE hDev = (KUSB_HANDLE)device.pUsbHandle;
            UsbK_Free( hDev );

            if (usbType == RKUSB_MSC) 
            {
                device.emDeviceType = RKNONE_DEVICE;
            } 
            else 
            {
                if (IsRockusbDevice(devType, device.usVid, device.usPid))
                    device.emDeviceType = devType;
            }
            
            bRet = true;
            break;
        }
        
        usleep(50000);
    }

    FreeDeviceList(deviceList);
    return bRet;
}

int CRKScan::GetPos(UINT locationID)
{
    int pos = 0;
    bool bFound = false;
    
    for ( device_list_iter iter = m_list.begin(); iter != m_list.end(); iter++) 
    {
        if (locationID == (*iter).uiLocationID) 
        {
            bFound=true;
            break;
        }
        pos++;
    }
    
    return (bFound ? pos : -1);
}

bool CRKScan::GetDevice(STRUCT_RKDEVICE_DESC &device, int pos)
{
    if ( (pos < 0) || (pos >= (int)m_list.size()) ) 
    {
        return false;
    }
    
    device_list_iter iter;
    
    for (iter = m_list.begin(); iter != m_list.end(); iter++) 
    {
        if (pos == 0) 
        {
            break;
        }
        
        pos--;
    }
    
    device.usVid = (*iter).usVid;
    device.usPid = (*iter).usPid;
    device.emDeviceType = (*iter).emDeviceType;
    device.emUsbType = (*iter).emUsbType;
    device.uiLocationID = (*iter).uiLocationID;
    device.pUsbHandle= (*iter).pUsbHandle;
    device.usbcdUsb = (*iter).usbcdUsb;
    
    return true;
}

bool CRKScan::SetLogObject(CRKLog *pLog)
{
    if (pLog) 
    {
        if (m_log) 
        {
            delete m_log;
        }
        
        m_log = pLog;
    } 
    else
        return false;
    
    return true;
}

CRKScan::~CRKScan()
{
    FreeDeviceList(m_list);

    if (m_log) 
    {
        delete m_log;
        m_log = NULL;
    }
}

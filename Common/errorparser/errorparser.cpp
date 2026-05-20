/********************************************************************
	created:	2006/07/01
	created:	1:7:2006   9:36
	filename: 	d:\IxEngine\Common\errorparser\errorparser.cpp
	author:		cxi
	
	purpose:	convert error code to error string
*********************************************************************/
#include "stdh.h"

#include "FileSystem/FileSystemDefines.h"
#include "RenderSystem/IRenderSystemDefines.h"
 
#define ERRSTRING(code)\
	case code:\
		return #code;

const char *ErrorString(DWORD err)
{
	switch(err)
	{
		//FS
		ERRSTRING(Result_TooLongPath)
		ERRSTRING(Result_InvalidPath)
		ERRSTRING(Result_DirectoryAlreadyExist)
		ERRSTRING(Result_CannotCreateDirectory)
		ERRSTRING(Result_CannotCreatePackage)
		ERRSTRING(Result_CannotOpenPackage)
		ERRSTRING(Result_InvalidOpenMode)
		ERRSTRING(Result_NoSearchPath)
		ERRSTRING(Result_DirectoryInUse)
		ERRSTRING(Result_FileAlreadyExist)
		ERRSTRING(Result_WritingReadOnly)

		//RS
		ERRSTRING(DeviceErr_NODIRECT3D)
		ERRSTRING(DeviceErr_NOCOMPATIBLEDEVICES)
		ERRSTRING(DeviceErr_MEDIANOTFOUND)
		ERRSTRING(DeviceErr_NONZEROREFCOUNT)
		ERRSTRING(DeviceErr_CREATINGDEVICE)
		ERRSTRING(DeviceErr_RESETTINGDEVICE)
		ERRSTRING(DeviceErr_CREATINGDEVICEOBJECTS)
		ERRSTRING(DeviceErr_RESETTINGDEVICEOBJECTS)
		ERRSTRING(DeviceErr_INCORRECTVERSION)
		ERRSTRING(DeviceErr_INITVBPOOL)
		ERRSTRING(DeviceErr_INITSHADERMANAGER)
		ERRSTRING(DeviceErr_INITTEXTUREMANAGER)
		ERRSTRING(DeviceErr_INITRTMANAGER)
		ERRSTRING(DeviceErr_SWITCHEDTOREF)

		ERRSTRING(TexFileErr_CannotOpenFile)
		ERRSTRING(TexFileErr_CannotInitD3D)
		ERRSTRING(TexFileErr_FailToLoadImage)
		ERRSTRING(TexFileErr_FailToConvertTexture)
		ERRSTRING(TexFileErr_FailToSave)
		ERRSTRING(TexFileErr_NotConsistency)
	}

	return "Error_Unknown";
}

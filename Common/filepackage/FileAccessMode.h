#pragma once

enum FileAccessMode
{
	FileAccessMode_None=0,
	FileAccessMode_Read=1,//only read
	FileAccessMode_Write=2,//create & write
	FileAccessMode_WritePackage=3,//create &write to package
	FileAccessMode_Modify=4,//read & write(will not create)
};

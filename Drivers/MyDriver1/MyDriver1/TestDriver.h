#pragma once

#include <ntddk.h>

class TestDriver
{
private:
	PDRIVER_OBJECT driverobject;
	PUNICODE_STRING registrypath;

public:
	TestDriver(PDRIVER_OBJECT, PUNICODE_STRING);

	static void DriverUnload(PDRIVER_OBJECT);
	
	RTL_OSVERSIONINFOW GetOsVersion();
	
	void SleepThread(short);

	

};


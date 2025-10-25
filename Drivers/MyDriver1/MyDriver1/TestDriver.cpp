#include "TestDriver.h"


void TestDriver::DriverUnload(PDRIVER_OBJECT driverobject) {
	UNREFERENCED_PARAMETER(driverobject);

	KdPrint(("DriverUnload function got called yipee2\n"));

}





TestDriver::TestDriver(PDRIVER_OBJECT driverobject, PUNICODE_STRING registrypath)
 {

	this->SleepThread(6);


	KdPrint(("DriverEntry function called yayyy!\n"));

	

	this->driverobject = driverobject;
	this->registrypath = registrypath;


	this->driverobject->DriverUnload =   (TestDriver::DriverUnload);

	

}


RTL_OSVERSIONINFOW TestDriver::GetOsVersion() {

	RTL_OSVERSIONINFOW osversion{ 0 };
	osversion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

	NTSTATUS status = RtlGetVersion(&osversion);
	if (!(NT_SUCCESS(status))) {
		KdPrint(("RtlGetVersion error: %d", status));
	}

	return osversion;

}


void TestDriver::SleepThread(short seconds) {

	LARGE_INTEGER li{ 0 };
	li.QuadPart = -seconds * 10 * 1000 * 1000;
	KeDelayExecutionThread(KernelMode, FALSE, &li);

}


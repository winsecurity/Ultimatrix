#include "utils.h"


void utils::SleepThread(short seconds) {

	LARGE_INTEGER li{ 0 };
	li.QuadPart = -seconds * 10 * 1000 * 1000;
	KeDelayExecutionThread(KernelMode, FALSE, &li);

}



RTL_OSVERSIONINFOW utils::GetOsVersion() {

	RTL_OSVERSIONINFOW osversion{ 0 };
	osversion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

	NTSTATUS status = RtlGetVersion(&osversion);
	if (!(NT_SUCCESS(status))) {
		KdPrint(("RtlGetVersion error: %d", status));
	}

	return osversion;

}







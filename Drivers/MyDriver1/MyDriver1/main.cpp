
#include <ntddk.h>

#include "utils.h"


#define IOCTL_METHOD_BUFFER_TEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_METHOD_IN_DIRECT_TEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_IN_DIRECT, FILE_ANY_ACCESS)



Result<RTL_OSVERSIONINFOW> GetOsVersion2() {


	RTL_OSVERSIONINFOW osversion{ 0 };
	osversion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);

	NTSTATUS status = RtlGetVersion(&osversion);
	if (!(NT_SUCCESS(status))) {
		Result<RTL_OSVERSIONINFOW> r = Result<RTL_OSVERSIONINFOW>(status);
		return r;
	}

	Result<RTL_OSVERSIONINFOW> r = Result<RTL_OSVERSIONINFOW>(osversion);

	

	return r;

}


void MyUnload(PDRIVER_OBJECT driverobject) {

	KdPrint(("Driver Unload function called: MyUnload \n"));

	UNICODE_STRING symname;
	RtlInitUnicodeString(&symname, L"\\??\\MyDevice1SymbolicLink");
	IoDeleteSymbolicLink(&symname);

	IoDeleteDevice(driverobject->DeviceObject);

}



NTSTATUS CreateHandler(PDEVICE_OBJECT , PIRP irp) {
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_SUCCESS;
	IofCompleteRequest(irp, 0);

	return STATUS_SUCCESS;
}


NTSTATUS CloseHandler(PDEVICE_OBJECT , PIRP irp) {
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_SUCCESS;
	IofCompleteRequest(irp, 0);

	return STATUS_SUCCESS;
}


NTSTATUS DeviceControlHandler(PDEVICE_OBJECT, PIRP irp) {


	PIO_STACK_LOCATION iostacklocation = IoGetCurrentIrpStackLocation(irp);
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	int len = 0;
	
	if (iostacklocation->Parameters.DeviceIoControl.IoControlCode == IOCTL_METHOD_BUFFER_TEST) {
		KdPrint(("METHOD_BUFFERED operation\n"));


		char buffer[1024];
		if (iostacklocation->Parameters.DeviceIoControl.InputBufferLength <= sizeof(buffer) / sizeof(char)) {
			RtlCopyMemory(&buffer, irp->AssociatedIrp.SystemBuffer, iostacklocation->Parameters.DeviceIoControl.InputBufferLength);
			KdPrint(("string from user: %s\n", &buffer));
		}
		else {
			status = STATUS_BUFFER_TOO_SMALL;
			goto completeirp;
		}
		
		

		const char* s = "Exit123\0";
		if (iostacklocation->Parameters.DeviceIoControl.OutputBufferLength >= strlen(s)) {
			RtlCopyMemory(irp->AssociatedIrp.SystemBuffer, s, strlen(s));
			status = STATUS_SUCCESS;
			len = (int)strlen(s);
			goto completeirp;
		}
		

	}
		
	if (iostacklocation->Parameters.DeviceIoControl.IoControlCode == IOCTL_METHOD_IN_DIRECT_TEST) {

		KdPrint(("METHOD_IN_DIRECT IO buffering mode\n"));

		// input buffer from user mode is written in to irp.AssociatedIrp.SystemBuffer
		// output buffer gets locked is accessible in irp.Mdladdress
		// we can use MmGetSystemAddressForMdlSafe() to get system pointer to the mapped output buffer
		char inputbuffer[512];
		if (iostacklocation->Parameters.DeviceIoControl.InputBufferLength<=sizeof(inputbuffer) &&
			irp->AssociatedIrp.SystemBuffer!=NULL) {
			
			RtlCopyMemory(&inputbuffer, irp->AssociatedIrp.SystemBuffer, iostacklocation->Parameters.DeviceIoControl.InputBufferLength);
			KdPrint(("Received from the user: %s\n", &inputbuffer));
		}
		else {
			status = STATUS_BUFFER_TOO_SMALL;
			goto completeirp;
		}
		

		PVOID outputbufferaddress = MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority);
		if (outputbufferaddress == NULL) {
			KdPrint(("MmGetSystemAddressForMdlSafe failed: "));
			status = STATUS_INSUFFICIENT_RESOURCES;
			goto completeirp;
		}


		char outputbuffer[512] = { 'A' };
		int outputbuffersize = (sizeof(outputbuffer) / sizeof(char));
		for (int i = 0;i < outputbuffersize;i++) {
			outputbuffer[i] = 'A';
		}
		outputbuffer[511] = 0;
		KdPrint(("output buffer length of user: %d\n", iostacklocation->Parameters.DeviceIoControl.OutputBufferLength));
		if (iostacklocation->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(outputbuffer)) {
			RtlCopyMemory(outputbufferaddress, &outputbuffer, 512);
			KdPrint(("sent %d A's to the user\n", outputbuffersize));
			status = STATUS_SUCCESS;
			len = outputbuffersize;
			goto completeirp;
		}
		else {
			status = STATUS_BUFFER_TOO_SMALL;
			goto completeirp;
		}

	

	}
	

completeirp:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = len;

	KdPrint(("iostatus information member value: %d\n", irp->IoStatus.Information));

	IofCompleteRequest(irp,IO_NO_INCREMENT);

	return status;
}


extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driverobject, PUNICODE_STRING registrypath) {

	
	
	driverobject->DriverUnload = MyUnload;

	UNREFERENCED_PARAMETER(registrypath);
	

	//utils u;
	//u.SleepThread(6);

	driverobject->MajorFunction[IRP_MJ_CREATE] = CreateHandler;
	driverobject->MajorFunction[IRP_MJ_CLOSE] = CloseHandler;
	driverobject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControlHandler;


	UNICODE_STRING us;
	RtlInitUnicodeString(&us, L"This is test unicode string");


	KdPrint(("Unicodestring: %wZ\n", us));

	KdPrint(("DriverEntry function got executed \n"));


	UNICODE_STRING devicename;
	RtlInitUnicodeString(&devicename, L"\\Device\\MyDevice1");
	PDEVICE_OBJECT deviceobject;
	NTSTATUS status = IoCreateDevice(driverobject, 0, &devicename, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceobject);
	if (!(NT_SUCCESS(status))) {
		KdPrint(("IoCreateDevice failed: %d\n", status));
		return status;
	}
	else {
		KdPrint(("Device object %wZ has been created\n", devicename));
	}



	// creating symbolic link
	UNICODE_STRING symname;
	RtlInitUnicodeString(&symname, L"\\??\\MyDevice1SymbolicLink");
	status = IoCreateSymbolicLink(&symname, &devicename);
	if (!(NT_SUCCESS(status))) {
		IoDeleteDevice(deviceobject);
		KdPrint(("IoCreateSymbolicLink failed: %X\n", status));
		return status;
	}
	else {
		KdPrint(("Symbolic link %wZ has been created\n", symname));
	}





	return STATUS_SUCCESS;

}






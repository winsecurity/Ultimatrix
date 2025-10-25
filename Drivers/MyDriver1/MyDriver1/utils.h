#pragma once
#include <ntddk.h>



class utils
{

public:
	void SleepThread(short);
	RTL_OSVERSIONINFOW GetOsVersion();

	
	
};



class MakeUnicodeString {
public:
	UNICODE_STRING us;

	MakeUnicodeString(PCWSTR s) {
		RtlInitUnicodeString(&(this->us), s);
	}


	~MakeUnicodeString() {
		KdPrint(("This is destructor"));
	}

};





template <typename T> 
class Result {
public:
	T value;
	NTSTATUS error;




	Result(T value) {
		this->value = value;
		this->error = STATUS_SUCCESS;
	}

	Result(NTSTATUS error) {
		this->error = error;
		
	}

	bool is_ok();

	bool is_err();

	T ok_unwrap();

	NTSTATUS err_unwrap();

};


template <typename T>
bool Result<T>::is_ok() {

	if (this->error==NULL) {
		return true;
	}

	else return false;
}

template <typename T>
bool Result<T>::is_err() {

	if (this->error == NULL) {
		return false;
	}

	else return true;
}

template <typename T>
T Result<T>::ok_unwrap() {
	return this->value;
}

template <typename T>
NTSTATUS Result<T>::err_unwrap() {
	return this->error;
}


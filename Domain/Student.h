#pragma once
#include "stdafx.h"
#include <windows.h> 
#include <stdio.h>
#include <vector>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost\serialization\vector.hpp>
#include <boost/serialization/assume_abstract.hpp>

#define BUFSIZES 4096

class Student
{
	friend std::ostream & operator<<(std::ostream &os, const Student &gp);
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int /* file_version */) {
		ar &code & name;
	}

private:
	int code;
	std::string name;
public:
	Student() = default;
	Student::~Student() {};
	Student(int argCode, std::string argName) : code(argCode), name(argName) { }

	int GetCode() { return this->code; }
	std::string GetName() { return this->name; }
};

typedef struct
{
	OVERLAPPED oOverlap;
	HANDLE hPipeInst;
	TCHAR chRequest[BUFSIZES];
	DWORD cbRead;
	TCHAR chReply[BUFSIZES];
	DWORD cbToWrite;
} PIPEINST, *LPPIPEINST;
#pragma once
#include "Commands.h"
#include "Student.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class WrapMessage
{

	friend std::ostream & operator<<(std::ostream &os, const Student &st);
	friend std::ostream & operator<<(std::ostream &os, const Commands &st);
	friend std::ostream & operator<<(std::ostream &os, const WrapMessage &wm);

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int /* file_version */) {
		ar &_command & _students &_data;
	}

public:
	WrapMessage() = default;
	WrapMessage(Commands command);
	WrapMessage(Student student , Commands command);
	WrapMessage(string data, Commands command);
	~WrapMessage();
	Commands GetCommand();
	vector<Student> GetStudents();
	vector<string> GetData();
	void SetListOfStudents(vector<Student> students);
	void SetListOfData(vector<string> data);
	void SetStudent(Student st);
	void SetCommand(Commands command);
	int GetStudentsCount();
	int GetDataCount();
	void SetSomeData(string data);
	static WrapMessage* DeserializeMessage(LPTSTR request);
	static std::wstring SerializeMessage(const WrapMessage *obj);
private:
	Commands _command;
	vector<Student> _students;
	vector<string> _data;
};


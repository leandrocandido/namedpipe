#include "stdafx.h"
#include "WrapMessage.h"

WrapMessage::WrapMessage(Commands command)
{
	this->_command = command;
}

WrapMessage::WrapMessage(Student student, Commands command)
{
	this->_students.push_back(student);
	this->_command = command;
}

WrapMessage::WrapMessage(string data, Commands command)
{
	this->_data.push_back(data);
	this->_command = command;
}


WrapMessage::~WrapMessage()
{
	this->_students.empty();
	this->_data.empty();
}
//get command inside wrap message
Commands WrapMessage::GetCommand()
{
	return this->_command;
}
//get the list of data in the message
vector<string> WrapMessage::GetData()
{
	return this->_data;
}
// set the command to be processed
void  WrapMessage::SetCommand(Commands command)
{
	this->_command = command;
}
//set the list of students in the wrap message
void WrapMessage::SetListOfStudents(vector<Student> students) 
{
	this->_students = students;
}
//set list of regualar data to be wrap in message
void WrapMessage::SetListOfData(vector<string> data)
{
	this->_data = data;
}
//set the student object in the list of students
void WrapMessage::SetStudent(Student st)
{
	this->_students.push_back(st);
}
//get the size of students elements
int WrapMessage::GetStudentsCount()
{
	return this->_students.size();
}
// get the size of data elements
int WrapMessage::GetDataCount()
{
	return this->_data.size();
}
//set the regular message in wrap message
void WrapMessage::SetSomeData(string data)
{
	this->_data.push_back(data);
}
//deserialize message using boost lib
WrapMessage* WrapMessage::DeserializeMessage(LPTSTR request)
{
	std::string val;
	int qtd = lstrlen(request);

	for (int i = 0; i < qtd; i++)
		val.push_back(request[i]);

	std::stringstream MyStringStream;
	MyStringStream << val;

	boost::archive::text_iarchive ia(MyStringStream);

	WrapMessage *result;
	ia >> result;
	return result;
}
//serialize message using boost lib
std::wstring WrapMessage::SerializeMessage(const WrapMessage *obj)
{
	std::stringstream MyStringStream;
	boost::archive::text_oarchive oa(MyStringStream);
	oa << obj;

	std::string data = MyStringStream.str();

	std::wstring stemp = std::wstring(data.begin(), data.end());
	return stemp;
}
//get the list of students in the message
vector<Student> WrapMessage::GetStudents()
{
	return this->_students;
}

std::ostream & operator<<(std::ostream & os, const Student & st)
{
	return os << ' ' << st.code << ' ' << st.name << '\'' << '"';
}

std::ostream & operator<<(std::ostream & os, const Commands & st)
{
	return os << ' ' << st << '\'' << '"';
}
std::ostream & operator<<(std::ostream & os, const WrapMessage & wm)
{
	//serial the list of students
	for (auto it : wm._students)
	{
		os << it;
	}
	//serializes the list of data
	for (auto dt : wm._data)
	{
		os << dt;
	}
	//serializes the command
	os << wm._command;
	
	return os;
}
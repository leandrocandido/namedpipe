#pragma once
#include <strsafe.h>
#include "BaseService.h"
#include "WrapMessage.h"

#define BUFSIZE 512

class NamedPipeServer : public BaseService
{
private:
	static vector<Student> _myStudents;
	static vector<string> _myData;
public:
	NamedPipeServer();
	~NamedPipeServer();
	virtual void Initialize() override;
	static VOID GetAnswerToRequest(LPTSTR, LPTSTR, WrapMessage*);
	static DWORD WINAPI InstanceThread(LPVOID);
};

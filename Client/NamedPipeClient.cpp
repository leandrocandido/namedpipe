#include "stdafx.h"
#include "NamedPipeClient.h"

using std::cout;
using std::endl;

NamedPipeClient::NamedPipeClient()
{
	//named fipe file
	lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");
}


NamedPipeClient::~NamedPipeClient()
{
}

//flag used to stop async thread
bool NamedPipeClient::_stopThread;

int NamedPipeClient::SendAsync(const WrapMessage *wrapmessage)
{
	BOOL   fSuccess = FALSE;
	DWORD  dwMode;

	NamedPipeClient::_stopThread = false;

	LPPIPEINST lpPipeInst;
	DWORD dwWait, cbRet;

	if (ConectToServer(1))
	{
		// Try to open a named pipe; wait for it, if necessary. 
		// The pipe connected; change to message-read mode. 
		dwMode = PIPE_READMODE_MESSAGE;
		fSuccess = SetNamedPipeHandleState(
			hPipe,    // pipe handle 
			&dwMode,  // new pipe mode 
			NULL,     // don't set maximum bytes 
			NULL);    // don't set maximum time 

		if (!fSuccess)
		{
			_tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
			return -1;
		}

		// Allocate storage for this instance. 

		lpPipeInst = (LPPIPEINST)GlobalAlloc(
			GPTR, sizeof(PIPEINST));
		if (lpPipeInst == NULL)
		{
			printf("GlobalAlloc failed (%d)\n", GetLastError());
			return 0;
		}

		//const Student student(100, "Leandro");
		//const WrapMessage wm(student, Commands::SendDataAsync);

		std::wstring stemp1 = WrapMessage::SerializeMessage(wrapmessage);
		LPCWSTR sw = stemp1.c_str();

		lpPipeInst->hPipeInst = hPipe;
		StringCchCopy(lpPipeInst->chReply, BUFSIZES, sw);

		lpPipeInst->cbToWrite = (lstrlen(lpPipeInst->chReply) + 1) * sizeof(TCHAR);


		//write data to named pipe server
		fSuccess = WriteFileEx(
			lpPipeInst->hPipeInst,
			lpPipeInst->chReply,
			lpPipeInst->cbToWrite,
			(LPOVERLAPPED)lpPipeInst,
			(LPOVERLAPPED_COMPLETION_ROUTINE)NamedPipeClient::CompletedWriteRoutine);

		if (!fSuccess)
		{
			_tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
			NamedPipeClient::DisconnectAndClose(lpPipeInst);
			return -1;
		}
			
		while (!NamedPipeClient::_stopThread)
		{
			dwWait = WaitForSingleObjectEx(
				hConnectEvent,  // event object to wait for 
				INFINITE,       // waits indefinitely 
				TRUE);

			//wait for async completion
			switch (dwWait)
			{
			case 0:
				fSuccess = GetOverlappedResult(
					hPipe,     // pipe handle 
					&oConnect, // OVERLAPPED structure 
					&cbRet,    // bytes transferred 
					FALSE);    // does not wait 
				if (!fSuccess)
				{
					printf("ConnectNamedPipe (%d)\n", GetLastError());
					return 0;
				}
				break;
			case WAIT_IO_COMPLETION:
				break;
			default:
				printf("WaitForSingleObjectEx (%d)\n", GetLastError());
				return 0;
			}
		}
	}

	return 0;
}

//send sync wrapmessage object to server
int NamedPipeClient::SendSync(const WrapMessage * wrapmessage)
{
	
	BOOL   fSuccess = FALSE;
	DWORD  cbRead, cbWritten, dwMode;
	TCHAR  chBuf[BUFSIZES];
	LPPIPEINST lpPipeInst;

	// Try to open a named pipe; wait for it, if necessary. 

	if (ConectToServer(2))
	{
		// The pipe connected; change to message-read mode. 

		dwMode = PIPE_READMODE_MESSAGE;
		fSuccess = SetNamedPipeHandleState(
			hPipe,    // pipe handle 
			&dwMode,  // new pipe mode 
			NULL,     // don't set maximum bytes 
			NULL);    // don't set maximum time 
		if (!fSuccess)
		{
			_tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
			return -1;
		}

		// Allocate storage for this instance. 

		lpPipeInst = (LPPIPEINST)GlobalAlloc(
			GPTR, sizeof(PIPEINST));
		if (lpPipeInst == NULL)
		{
			printf("GlobalAlloc failed (%d)\n", GetLastError());
			return 0;
		}

		// Send a message to the pipe server. 
		std::wstring stemp1 = WrapMessage::SerializeMessage(wrapmessage);
		//prepare the serialized message to be send
		LPCWSTR sw = stemp1.c_str();
		
		//stores the pipe handle
		lpPipeInst->hPipeInst = hPipe;
		StringCchCopy(lpPipeInst->chReply, BUFSIZES, sw);
		lpPipeInst->cbToWrite = (lstrlen(lpPipeInst->chReply) + 1) * sizeof(TCHAR);
		//write data to named pipe server
		fSuccess = WriteFile(
			lpPipeInst->hPipeInst,
			lpPipeInst->chReply,
			lpPipeInst->cbToWrite,
			&cbWritten,
			NULL
		);

		if (!fSuccess)
		{
			_tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
			return -1;
		}

		do
		{
			// Read from the pipe. 
			fSuccess = ReadFile(
				hPipe,    // pipe handle 
				chBuf,    // buffer to receive reply 
				BUFSIZES * sizeof(TCHAR),  // size of buffer 
				&cbRead,  // number of bytes read 
				NULL);    // not overlapped 

			if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
				break;

		} while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 

		if (!fSuccess)
		{
			_tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
			return -1;
		}
		//deserialize message to be processed
		auto resp = WrapMessage::DeserializeMessage(chBuf);
		//processes named pipe server response
		ProcessResponse(resp);
		//close the pipe handle
		CloseHandle(hPipe);
	}
	return 0;
}

//process the names pipe response
void NamedPipeClient::ProcessResponse(WrapMessage* wm)
{
	switch (wm->GetCommand())
	{
	case Commands::ResponseServerOk:
		cout << "Message was sucessfuly send to server" << endl;
		break;
	case Commands::ResponseListOfStudents:
		if (wm->GetStudentsCount() > 0)
		{
			cout << "Printing the list of students from server" << endl;
			for (auto item : wm->GetStudents())
			{
				cout << "Student name: " << item.GetName() << " Student Code : " << item.GetCode() << endl;
			}
		}
		else
		{
			cout << "No students stored on server.." << endl;
		}
		break;
	case Commands::ResponseListOfData :
			if (wm->GetDataCount() > 0)
			{
				cout << "Printing the list of data from server" << endl;
				for (auto item : wm->GetData())
				{
					cout << "Data stored on server : " << item << endl;
				}
			}
			break;
	default:
		break;
	}
}
//connect to named pipe server
bool NamedPipeClient::ConectToServer(int type)
{
	bool conn = false;

	//verify the connection type
	if(type == 1)
		conn = ConnectAsync();
	else
		conn = ConnectSync();

	while (!conn)
	{
		cout << "Was not possible connect to server - check if server is available press any key to try again..." << endl;
		_getch();

		if (type == 1)
			conn = ConnectAsync();
		else
			conn = ConnectSync();
	}

	return conn;
}

//makes the async connection to the named pipe server
bool NamedPipeClient::ConnectAsync()
{
	if (this->Initialize())
	{
		while (1)
		{
			hPipe = CreateFile(
				lpszPipename,   // pipe name 
				GENERIC_READ |  // read and write access 
				GENERIC_WRITE,
				0,              // no sharing 
				NULL,           // default security attributes
				OPEN_EXISTING,  // opens existing pipe 
				FILE_FLAG_OVERLAPPED,              // default attributes 
				NULL);          // no template file 

								// Break if the pipe handle is valid. 

			if (hPipe != INVALID_HANDLE_VALUE)
			{
				return true;
			}


			// Exit if an error other than ERROR_PIPE_BUSY occurs. 

			if (GetLastError() != ERROR_PIPE_BUSY)
			{
				_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
				return false;
			}

			// All pipe instances are busy, so wait for 20 seconds. 

			if (!WaitNamedPipe(lpszPipename, 20000))
			{
				printf("Could not open pipe: 20 second wait timed out.");
				return false;
			}
		}
	}
	else
		return false;
}
//makes the sync connection to named pipe server
bool NamedPipeClient::ConnectSync()
{
	while (1)
	{
		hPipe = CreateFile(
			lpszPipename,   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE,
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

							// Break if the pipe handle is valid. 

		if (hPipe != INVALID_HANDLE_VALUE)
			return true;

		// Exit if an error other than ERROR_PIPE_BUSY occurs. 

		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
			return false;
		}

		// All pipe instances are busy, so wait for 20 seconds. 

		if (!WaitNamedPipe(lpszPipename, 20000))
		{
			printf("Could not open pipe: 20 second wait timed out.");
			return false;
		}
	}
	return true;
}

//creates named pipe connection event 
bool NamedPipeClient::Initialize()
{
	bool res = true;
	// Create one event object for the connect operation. 
	hConnectEvent = CreateEvent(
		NULL,    // default security attribute
		TRUE,    // manual reset event 
		TRUE,    // initial state = signaled 
		NULL);   // unnamed event object 

	if (hConnectEvent == NULL)
	{
		printf("CreateEvent failed with %d.\n", GetLastError());
		res = false;
	}

	oConnect.hEvent = hConnectEvent;
	return res;
}
//callback method to be called when read process is finalized
VOID NamedPipeClient::CompletedReadRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap)
{

	LPPIPEINST lpPipeInst;
	lpPipeInst = (LPPIPEINST)lpOverLap;
	//_tprintf(TEXT("Message from Server String:\"%s\"\n"), lpPipeInst->chRequest);
	auto s = WrapMessage::DeserializeMessage(lpPipeInst->chRequest);
	ProcessResponse(s);
	NamedPipeClient::_stopThread = true;
	//_getch();
	NamedPipeClient::DisconnectAndClose(lpPipeInst);
}
//callback method to be called when write process is finalized
VOID NamedPipeClient::CompletedWriteRoutine(DWORD dwErr, DWORD cbWritten, LPOVERLAPPED lpOverLap)
{
	LPPIPEINST lpPipeInst;
	BOOL fRead = FALSE;

	// lpOverlap points to storage for this instance. 
	lpPipeInst = (LPPIPEINST)lpOverLap;

	if ((dwErr == 0) && (cbWritten == lpPipeInst->cbToWrite))
		fRead = ReadFileEx(
			lpPipeInst->hPipeInst,
			lpPipeInst->chRequest,
			BUFSIZES * sizeof(TCHAR),
			(LPOVERLAPPED)lpPipeInst,
			(LPOVERLAPPED_COMPLETION_ROUTINE)CompletedReadRoutine);
}

//disconnect and close to the named pipe server
VOID NamedPipeClient::DisconnectAndClose(LPPIPEINST lpPipeInst)
{
	FlushFileBuffers(lpPipeInst->hPipeInst);
	// Disconnect the pipe instance. 
	DisconnectNamedPipe(lpPipeInst->hPipeInst);

	// Close the handle to the pipe instance. 
	CloseHandle(lpPipeInst->hPipeInst);

	// Release the storage for the pipe instance. 
	if (lpPipeInst != NULL)
		GlobalFree(lpPipeInst);
}
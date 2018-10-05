#include "stdafx.h"
#include "NamedPipeServer.h"
#include <iostream>
#include <fstream>
#include <sstream>

vector<Student> NamedPipeServer::_myStudents;
vector<string> NamedPipeServer::_myData;

NamedPipeServer::NamedPipeServer()
{
}


NamedPipeServer::~NamedPipeServer()
{
}

void NamedPipeServer::Initialize()
{
	BOOL   fConnected = FALSE;
	DWORD  dwThreadId = 0;
	HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");

	// The main loop creates an instance of the named pipe and 
	// then waits for a client to connect to it. When the client 
	// connects, a thread is created to handle communications 
	// with that client, and this loop is free to wait for the
	// next client connect request. It is an infinite loop.

	for (;;)
	{
		_tprintf(TEXT("\nPipe Server: Main thread awaiting client connection on %s\n"), lpszPipename);
		hPipe = CreateNamedPipe(
			lpszPipename,             // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			BUFSIZE,                  // output buffer size 
			BUFSIZE,                  // input buffer size 
			0,                        // client time-out 
			NULL);                    // default security attribute 

		if (hPipe == INVALID_HANDLE_VALUE)
		{
			_tprintf(TEXT("CreateNamedPipe failed, GLE=%d.\n"), GetLastError());
			return;
		}

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 

		fConnected = ConnectNamedPipe(hPipe, NULL) ?
			TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (fConnected)
		{
			printf("Client connected, creating a processing thread.\n");

			// Create a thread for this client. 
			hThread = CreateThread(
				NULL,              // no security attribute 
				0,                 // default stack size 
				NamedPipeServer::InstanceThread,    // thread proc
				(LPVOID)hPipe,    // thread parameter 
				0,                 // not suspended 
				&dwThreadId);      // returns thread ID 

			if (hThread == NULL)
			{
				_tprintf(TEXT("CreateThread failed, GLE=%d.\n"), GetLastError());
				return;
			}
			else CloseHandle(hThread);
		}
		else
			// The client could not connect, so close the pipe. 
			CloseHandle(hPipe);
	}
}

// This routine is a thread processing function to read from and reply to a client
// via the open pipe connection passed from the main loop. Note this allows
// the main loop to continue executing, potentially creating more threads of
// of this procedure to run concurrently, depending on the number of incoming
// client connections.
DWORD WINAPI NamedPipeServer::InstanceThread(LPVOID lpvParam)
{
	HANDLE hHeap = GetProcessHeap();
	TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));
	TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR));

	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = NULL;

	// Do some extra error checking since the app will keep running even if this
	// thread fails.

	if (lpvParam == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL value in lpvParam.\n");
		printf("   InstanceThread exitting.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		return (DWORD)-1;
	}

	if (pchRequest == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL heap allocation.\n");
		printf("   InstanceThread exitting.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		return (DWORD)-1;
	}

	if (pchReply == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL heap allocation.\n");
		printf("   InstanceThread exitting.\n");
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		return (DWORD)-1;
	}

	// Print verbose messages. In production code, this should be for debugging only.
	printf("InstanceThread created, receiving and processing messages.\n");

	// The thread's parameter is a handle to a pipe object instance. 

	hPipe = (HANDLE)lpvParam;

	// Loop until done reading
	while (1)
	{
		// Read client requests from the pipe. This simplistic code only allows messages
		// up to BUFSIZE characters in length.
		fSuccess = ReadFile(
			hPipe,        // handle to pipe 
			pchRequest,    // buffer to receive data 
			BUFSIZE * sizeof(TCHAR), // size of buffer 
			&cbBytesRead, // number of bytes read 
			NULL);        // not overlapped I/O 

		if (!fSuccess || cbBytesRead == 0)
		{
			if (GetLastError() == ERROR_BROKEN_PIPE)
			{
				_tprintf(TEXT("InstanceThread: client disconnected.\n"), GetLastError());
			}
			else
			{
				_tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError());
			}
			break;
		}


		LPPIPEINST lpPipeInst = (LPPIPEINST)GlobalAlloc(
			GPTR, sizeof(PIPEINST));
		if (lpPipeInst == NULL)
		{
			printf("GlobalAlloc failed (%d)\n", GetLastError());
			return 0;
		}

		WrapMessage *response = new WrapMessage();
		// Process the incoming message.
		NamedPipeServer::GetAnswerToRequest(pchRequest, pchReply, response);

		std::wstring stemp1 = WrapMessage::SerializeMessage(response);
		LPCWSTR sw = stemp1.c_str();

		lpPipeInst->hPipeInst = hPipe;
		StringCchCopy(lpPipeInst->chReply, BUFSIZE, sw);
		lpPipeInst->cbToWrite = (lstrlen(lpPipeInst->chReply) + 1) * sizeof(TCHAR);

		fSuccess = WriteFile(
			lpPipeInst->hPipeInst,        // handle to pipe 
			lpPipeInst->chReply,     // buffer to write from 
			lpPipeInst->cbToWrite, // number of bytes to write 
			&cbWritten,   // number of bytes written 
			NULL);        // not overlapped I/O 

		if (!fSuccess || lpPipeInst->cbToWrite != cbWritten)
		{
			_tprintf(TEXT("InstanceThread WriteFile failed, GLE=%d.\n"), GetLastError());
			break;
		}
	}

	// Flush the pipe to allow the client to read the pipe's contents 
	// before disconnecting. Then disconnect the pipe, and close the 
	// handle to this pipe instance. 

	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);

	HeapFree(hHeap, 0, pchRequest);
	HeapFree(hHeap, 0, pchReply);

	printf("InstanceThread exitting.\n");
	return 1;
}

// This routine is a simple function to print the client request to the console
// and populate the reply buffer with a default data string. This is where you
// would put the actual client request processing code that runs in the context
// of an instance thread. Keep in mind the main thread will continue to wait for
// and receive other client connections while the instance thread is working.

VOID NamedPipeServer::GetAnswerToRequest(LPTSTR pchRequest,
	LPTSTR pchReply,
	WrapMessage* wm
	)
{
	auto s = WrapMessage::DeserializeMessage(pchRequest);

	switch (s->GetCommand())
	{
		case Commands::SendData :
		case Commands::SendDataAsync:
			for(auto el : s->GetData())
				NamedPipeServer::_myData.push_back(el);
			wm->SetCommand(Commands::ResponseServerOk);
			break;
		case Commands::SendObject:
		case Commands::SendObjectAsync:
			for (auto el : s->GetStudents())
				NamedPipeServer::_myStudents.push_back(el);
			wm->SetCommand(Commands::ResponseServerOk);
			break;
		case Commands::RequestStudentsAsync:
		case Commands::RequestStudentsSync:
			wm->SetListOfStudents(NamedPipeServer::_myStudents);
			wm->SetCommand(Commands::ResponseListOfStudents);
			break;
		case Commands::RequestDataAsync:
		case Commands::RequestDataSync:
			wm->SetListOfData(NamedPipeServer::_myData);
			wm->SetCommand(Commands::ResponseListOfData);
			break;
	default:
		break;
	}

	_tprintf(TEXT("Client Request String:\"%s\"\n"), pchRequest);

	// Check the outgoing message to make sure it's not too long for the buffer.
	if (FAILED(StringCchCopy(pchReply, BUFSIZE, TEXT("default answer from server"))))
	{
		pchReply[0] = 0;
		printf("StringCchCopy failed, no outgoing message.\n");
		return;
	}
}
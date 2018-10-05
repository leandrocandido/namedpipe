#pragma once
#include "WrapMessage.h"
#include "Commands.h"
#include <sstream>
#include <windows.h> 
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include "stdafx.h"
#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <memory>
#include <wchar.h>
#include <iostream>
#include <boost/archive/tmpdir.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost\serialization\vector.hpp>
#include <boost/serialization/assume_abstract.hpp>

#define PIPE_TIMEOUT 5000

class NamedPipeClient
{
private:
	HANDLE hPipe;
	HANDLE hConnectEvent;
	OVERLAPPED oConnect;
	static bool _stopThread;
	LPTSTR lpszPipename;
	bool ConnectAsync();
	bool ConnectSync();
	bool ConectToServer(int type);
	static void ProcessResponse(WrapMessage* wm);
private:
	bool Initialize();
public:
	NamedPipeClient();
	~NamedPipeClient();
	int SendAsync(const WrapMessage *wrapmessage);
	int SendSync(const WrapMessage *wrapmessage);
	static VOID DisconnectAndClose(LPPIPEINST);
	static VOID WINAPI CompletedReadRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap);
	static VOID WINAPI CompletedWriteRoutine(DWORD dwErr, DWORD cbWritten, LPOVERLAPPED lpOverLap);
};


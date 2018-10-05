// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <memory>
#include "NamedPipeServer.h"

int main()
{
	auto service = std::make_unique<NamedPipeServer>();
	service->Initialize();
	return 0;
}
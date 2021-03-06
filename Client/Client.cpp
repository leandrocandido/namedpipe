// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include "NamedPipeClient.h"
#include <memory>

//#define BUFSIZE 512

//Show submenu student option
void SubMenuStudent(WrapMessage * result)
{
	char name[100];
	int code = 0;
	bool flagStop = false;

	cout << "enter with student name :" << endl;
	cin.ignore();
	cin.getline(name,sizeof(name));
	cout << "enter with student code:" << endl;
	cin >> code;
	while (!cin.good())
	{
		cout << "Invalid values" <<endl;
		cin.clear();
		cin.ignore(INT32_MAX, '\n');
		cout << "enter with student code:" << endl;
		cin >> code;
	}
	
	const Student st(code, name);
	result->SetStudent(st);
}

//Show submenu to send some data to server
void SubMenuSomeData(WrapMessage * result)
{
	char data[100];
	cout << "write some data to send to server:" << endl;
	cin.ignore();
	cin.getline(data, sizeof(data));

	result->SetSomeData(data);
}

int main()
{
	//create a smart pointer for client
	auto client = std::make_unique<NamedPipeClient>();
	//flag to signalize when aplication should stop
	bool stop = false;
	//default value for option
	int option = 0;

	cout << "Connected to server please choose one opetion" << endl;
	while (!stop)
	{
		cout << "1 - to send async student object." << endl;
		cout << "2 - to send sync student object." << endl;
		cout << "3 - to send async data ." << endl;
		cout << "4 - to send sync data." << endl;
		cout << "5 - get student objects from server async" << endl;
		cout << "6 - get student objects from server sync" << endl;
		cout << "7 - get data from server async" << endl;
		cout << "8 - get data from server sync" << endl;
		cout << "9 - Quit " << endl;

		cin >> option ;

		while (!cin.good())
		{
			cin.clear();
			cin.ignore(INT32_MAX, '\n');
			option = 0;
		}
		WrapMessage *wm = new WrapMessage();
		switch (option)
		{
			//async student object to server
			case 1:
				wm->SetCommand(Commands::SendObjectAsync);
				SubMenuStudent(wm);
				client->SendAsync(wm);
				break;
			//sync student object to server
			case 2:
				wm->SetCommand(Commands::SendObject);
				SubMenuStudent(wm);
				client->SendSync(wm);
				break;
			//async data to server (string data)
			case 3:
				wm->SetCommand(Commands::SendDataAsync);
				SubMenuSomeData(wm);
				client->SendAsync(wm);
				break;
			//sync data to server (string data)
			case 4:
				wm->SetCommand(Commands::SendData);
				SubMenuSomeData(wm);
				client->SendSync(wm);
				break;
			//asynchronous call to retrieve students object from server
			case 5 :
				wm->SetCommand(Commands::RequestStudentsAsync);
				client->SendAsync(wm);
				break;
			//synchronous call to retrieve students object from server
			case 6:
				wm->SetCommand(Commands::RequestStudentsSync);
				client->SendSync(wm);
				break;
				//asynchronous call to retrieve data from server
			case 7:
				wm->SetCommand(Commands::RequestDataAsync);
				client->SendAsync(wm);
				break;
				//synchronous call to retrieve data from server
			case 8:
				wm->SetCommand(Commands::RequestDataSync);
				client->SendSync(wm);
				break;
			case 9:
				stop = true;
				break;
			default:
				cout << "Invalid option" << endl;
				break;

		}
		//release memory
		delete wm;
	}
}


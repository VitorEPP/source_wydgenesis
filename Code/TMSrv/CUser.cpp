/*
*   Copyright (C) {2015}  {Victor Klafke, Charles TheHouse}
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see [http://www.gnu.org/licenses/].
*
*   Contact at: victor.klafke@ecomp.ufsm.br
*/
#include <Windows.h>

#include "CUser.h"
#include "Server.h"


CUser::CUser()
{	
	Mode		     = USER_EMPTY;
	Unk3			 = 0;
	IsBillConnect    = 0;
	LastReceiveTime  = 0;

	memset(Cargo, 0, sizeof(Cargo));

	Admin			 = 0;
	CastleStatus	 = 0;
	MuteChat		 = 0;
	UseItemTime		 = 0;
	Message			 = 0;
	AttackTime		 = 0;
	LastClientTick	 = 0;
	PotionTime		 = 0;
	OnlyTrade		 = 0;

	WaitDB = 0;
}

CUser::~CUser()
{

}

BOOL CUser::AcceptUser(int ListenSocket)
{
	SOCKADDR_IN acc_sin;
	int Size = sizeof(acc_sin);

	int tSock = accept(ListenSocket, (struct sockaddr FAR*) & acc_sin, (int FAR*) & (Size));

	if (tSock < 0)
	{
		Mode = USER_EMPTY;
		return FALSE;
	}

	bool inList = false;
	auto TempIP = acc_sin.sin_addr.S_un.S_addr;

	for (auto& i : BlackList)
	{
		if (i.IP == TempIP)
		{
			if (i.BlackList)
			{
				closesocket(tSock);
				char msg[4096] = { 0, };
				char* tmp = inet_ntoa(acc_sin.sin_addr);
				sprintf(msg, "netsh advfirewall firewall show rule name = ""BLACKLIST-%d"" > null || netsh advfirewall firewall add rule name = ""BLACKLIST-%d"" dir = in action = block remoteip = %s", TempIP, TempIP, tmp);
				system(msg);
				return FALSE;
			}

			if ((CurrentTime - i.LastTimer) < 1000)
			{
				if (i.Warning >= 25)
				{
					i.Minutes = 10;
					i.BlackList = true;
					closesocket(tSock);
					return FALSE;
				}
				i.Warning++;
			}
			else
				i.LastTimer = CurrentTime;

			inList = true;
		}
	}
	if (!inList)
	{
		STRUCT_RECENTCONNECTION Temp;

		Temp.IP = TempIP;
		Temp.LastTimer = CurrentTime;
		Temp.Minutes = 0;
		Temp.Warning = 0;
		Temp.BlackList = false;

		BlackList.push_back(Temp);
	}

	if (WSAAsyncSelect(tSock, hWndMain, WSA_READ, FD_READ | FD_CLOSE) > 0)
	{
		closesocket(tSock);
		return FALSE;
	}

	cSock.Sock = tSock;
	cSock.nRecvPosition = 0;
	cSock.nProcPosition = 0;
	cSock.nSendPosition = 0;
	Mode = USER_ACCEPT;


	char* tmp = inet_ntoa(acc_sin.sin_addr);
	IP = acc_sin.sin_addr.S_un.S_addr;

	Unk3 = 0;

	/*sendBuffer = new BYTE[MAX_BUFFER];
	recvBuffer = new BYTE[MAX_BUFFER];*/

	return TRUE;
}

//BOOL CUser::AcceptUser(int ListenSocket)
//{
//	SOCKADDR_IN acc_sin; 
//	int Size = sizeof(acc_sin);
//
//	int tSock = accept(ListenSocket, (struct sockaddr FAR *)&acc_sin, (int FAR *)&(Size));
//
//	if(tSock < 0) 
//		return FALSE;
//
//	if(WSAAsyncSelect(tSock, hWndMain, WSA_READ, FD_READ | FD_CLOSE ) > 0) 
//	{	
//		closesocket(tSock);
//
//		return FALSE;
//	}
//
//	cSock.Sock          = tSock;
//	cSock.nRecvPosition = 0;
//	cSock.nProcPosition = 0;
//	cSock.nSendPosition = 0;
//
//	char *tmp  = inet_ntoa(acc_sin.sin_addr);
//	IP		   = acc_sin.sin_addr.S_un.S_addr;
//	Mode	   = USER_ACCEPT;
//	Unk3	   = 0;
//
//	return TRUE;
//}

int CUser::CloseUser()
{
     cSock.CloseSocket();
	 cSock.Sock		= 0;
	 IsBillConnect  = 0;	 
	 Mode = USER_EMPTY;
	 AccountName[0] = 0;
	 return TRUE;
}
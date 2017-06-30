#include<iostream>
#include<string>
#include<sstream>
#include"winsock2.h"
#include<iomanip>
#include<time.h>
#include<stdlib.h>
#include"Order.h"
#include<list>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
map<string,list<Order> >buyOrderList;
map<string,list<Order> >saleOrderList;
int main()
{
	WSADATA wsd;
	WSAStartup(MAKEWORD(2,2),&wsd);
	SOCKET m_ClientServer;
	sockaddr_in serveraddr;
	sockaddr_in myaddr;

	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(4600);
	serveraddr.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");

	m_ClientServer=socket(AF_INET,SOCK_STREAM,0);

	char buffer[1024];
	char sendBuffer[1024];
	char recvbuffer[1024];
	string user_name="monitor",pass_word="monitor";
	memset(buffer,'\0',1024);
	memset(sendBuffer,'\0',1024);
	memset(recvbuffer,'\0',1024);
	int i=connect(m_ClientServer,(sockaddr*)&serveraddr,sizeof(serveraddr));
	stringstream stream;
	stream<<user_name<<";"<<pass_word;
	stream>>sendBuffer;
	send(m_ClientServer,sendBuffer,sizeof(recvbuffer),0);
	while(recv(m_ClientServer,recvbuffer,sizeof(recvbuffer),0))
	{
		cout<<recvbuffer<<endl;
	}
	int res=recv(m_ClientServer,recvbuffer,sizeof(recvbuffer),0);
	cout<<recvbuffer<<endl;
	string stock_name;
	cout<<"请输入你想查看的股票名称：";
	cin>>stock_name;
	if(i==INVALID_SOCKET){
	}else{
		while(true)
	    {
			res=recv(m_ClientServer,recvbuffer,sizeof(recvbuffer),0);
			if(res>0)
			{
				FixMessage message(recvbuffer);
				cout<<setw(20)<<"BuyOrder"<<" "<<setw(20)<<"SaleOrder"<<endl;
				cout<<setw(10)<<"shares"<<" "<<setw(10)<<"price"<<setw(10)<<"shares"<<" "<<setw(10)<<"price"<<endl;
				stringstream stream;
				stream<<"35=Q;54=0;1="<<stock_name<<";";
				stream>>sendBuffer;
				send(m_ClientServer,sendBuffer,sizeof(sendBuffer),0);
				res=recv(m_ClientServer,recvbuffer,sizeof(recvbuffer),0);
				cout<<recvbuffer<<endl;
			}
			system("cls");
		}
	}
	closesocket(m_ClientServer);
	system("pause");
	return 0;
} 
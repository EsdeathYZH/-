#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include"winsock2.h"
#include<time.h>
#include<stdlib.h>
#include"Order.h"
#include<time.h>
#include<list>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
CRITICAL_SECTION  io_section;     //������Ŀ��Ҫ�õ�ȫ�ֱ���,cout�ؼ���
map<string,list<Order> >buyOrderList;
map<string,list<Order> >saleOrderList;
DWORD WINAPI threadpro1(LPVOID pParam)
{
	SOCKET m_ClientServer=(SOCKET)pParam;
	char sendBuffer[1024];
	char recvbuffer[1024];
	memset(sendBuffer,'\0',1024);
	memset(recvbuffer,'\0',1024);
	int res;
	while(true)
	{
		int choice;
		EnterCriticalSection(&io_section);
		cout<<"���ѽ��뽻��������ѡ����ķ�����Ŀ:"<<endl;
		cout<<"�����Ʊ�������롮1��"<<endl;
		cout<<"������Ʊ�������롮2��"<<endl;
		cout<<"ȡ�������������롮3��"<<endl;
		cout<<"��ѯ��ǰ���������������롮4��"<<endl;
		LeaveCriticalSection(&io_section);
		cin>>choice;
		switch(choice)
		{
		case 1:
			{
				double time_num;
				time_t tt = time(NULL);//��䷵�ص�ֻ��һ��ʱ��cuo
                tm* t= localtime(&tt);
				time_num=t->tm_hour+(double)t->tm_min/100;
				if(!(time_num>=9.3&&time_num<=11.3))
				{
					cout<<"������δ���ţ���ע�⽻��ʱ�Σ�"<<endl;
					break;
				}
				string stock_name;
				double price;
				int amount;
				EnterCriticalSection(&io_section);
				cout<<"��������Ҫ��Ĺ�Ʊ:";
				cin>>stock_name;
				cout<<"����������ļ۸�(��ο���ǰ����):";
				cin>>price;
				cout<<"��������������:";
				cin>>amount;
				LeaveCriticalSection(&io_section);
				stringstream stream;
				stream<<"35=D;38="<<amount<<";54=0;44="<<price<<";1="<<stock_name;
				stream>>sendBuffer;
				send(m_ClientServer,sendBuffer,sizeof(sendBuffer),0);
				break;
			}
		case 2:
			{
				double time_num;
				time_t tt = time(NULL);//��䷵�ص�ֻ��һ��ʱ��cuo
                tm* t= localtime(&tt);
				time_num=t->tm_hour+(double)t->tm_min/100;
				if(!(time_num>=9.3&&time_num<=11.3))
				{
					cout<<"������δ���ţ���ע�⽻��ʱ�Σ�"<<endl;
					break;
				}
				string stock_name;
				double price;
				int amount;
				EnterCriticalSection(&io_section);
				while(true)
				{
					cout<<"��������Ҫ���Ĺ�Ʊ:";
					cin>>stock_name;
					if(buyOrderList.count(stock_name)>0)
						break;
					cout<<"�ù�Ʊ���Ʋ����ڣ�������ѡ��"<<endl;
				}
				while(true)
				{
					cout<<"�����������۸�(��ο���ǰ����):";
					cin>>price;
					cout<<"��������������:";
					cin>>amount;
					if(amount<=0||amount>500)
					{
						cout<<"����������Ϲ涨����ı���Ĺ���������"<<endl;
						continue;
					}
				}
				LeaveCriticalSection(&io_section);
				stringstream stream;
				stream<<"35=D;38="<<amount<<";54=1;44="<<price<<";1="<<stock_name;
				stream>>sendBuffer;
				send(m_ClientServer,sendBuffer,sizeof(sendBuffer),0);
				break;
			}
		case 3:
			{
				string stock_name;
				int choice;
				EnterCriticalSection(&io_section);
				cout<<"����������Ҫȡ���Ķ�����Ʊ���ƣ�";
				cin>>stock_name;
				cout<<"ȡ�����򶩵�������������������ǹ���������0�����������������1��";
				cin>>choice;
				LeaveCriticalSection(&io_section);
				switch(choice)
				{
				case 0:
					{
						for(list<Order>::iterator iter=buyOrderList[stock_name].begin();iter!=buyOrderList[stock_name].end();iter++)
						{
							cout<<stock_name<<" id:"<<iter->getId()<<" price:"<<iter->getPrice();
							cout<<" executed amount:"<<iter->getFilledAmt()<<" total amount:"<<iter->getAmount()<<endl;
						}
						int id;
						while(1)//���id�Ϸ���
						{
							cout<<"��������Ҫȡ���Ķ���id��";
							cin>>id;
							for(list<Order>::iterator iter=buyOrderList[stock_name].begin();iter!=buyOrderList[stock_name].end();iter++)
							{
								if(iter->getId()==id) break;
							}
							cout<<"�ö�����������Ľ����б��У�"<<endl;
						}
						stringstream stream;
						stream<<"35=F;54=0;11="<<id<<";1="<<stock_name<<";";
						stream>>sendBuffer;
						send(m_ClientServer,sendBuffer,sizeof(sendBuffer),0);
						break;
					}
				case 1:
					{
						for(list<Order>::iterator iter=saleOrderList[stock_name].begin();iter!=saleOrderList[stock_name].end();iter++)
						{
							cout<<stock_name<<" id:"<<iter->getId()<<" price:"<<iter->getPrice();
							cout<<" executed amount:"<<iter->getFilledAmt()<<" total amount:"<<iter->getAmount()<<endl;
						}
						int id;
						while(1)//���id�Ϸ���
						{
							cout<<"��������Ҫȡ���Ķ���id��";
							cin>>id;
							for(list<Order>::iterator iter=buyOrderList[stock_name].begin();iter!=buyOrderList[stock_name].end();iter++)
							{
								if(iter->getId()==id) break;
							}
							cout<<"�ö�����������Ľ����б��У�"<<endl;
						}
						stringstream stream;
						stream<<"35=F;54=1;11="<<id<<";1="<<stock_name<<";";
						stream>>sendBuffer;
						send(m_ClientServer,sendBuffer,sizeof(sendBuffer),0);
						break;
					}
				}
				break;
			}
		case 4:
			{
				EnterCriticalSection(&io_section);
				cout<<"��ǰ����Ķ����У�"<<endl;
				for(map<string,list<Order> >::iterator iter1=buyOrderList.begin();iter1!=buyOrderList.end();iter1++)
				{
					for(list<Order>::iterator iter2=buyOrderList[iter1->first].begin();iter2!=buyOrderList[iter1->first].end();iter2++)
					{
						cout<<iter1->first<<" id:"<<iter2->getId()<<" price:"<<iter2->getPrice();
						cout<<" executed amount:"<<iter2->getFilledAmt()<<" total amount:"<<iter2->getAmount()<<endl;
					}
				}
				cout<<"��ǰ�����Ķ����У�"<<endl;
				for(map<string,list<Order> >::iterator iter1=saleOrderList.begin();iter1!=saleOrderList.end();iter1++)
				{
					for(list<Order>::iterator iter2=saleOrderList[iter1->first].begin();iter2!=saleOrderList[iter1->first].end();iter2++)
					{
						cout<<iter1->first<<" id:"<<iter2->getId()<<" price:"<<iter2->getPrice();
						cout<<" executed amount:"<<iter2->getFilledAmt()<<" total amount:"<<iter2->getAmount()<<endl;
					}
				}
				LeaveCriticalSection(&io_section);
				break;
			}
		default:
			cout<<"�޴˹��ܣ�����������.."<<endl;
		}
				
	}
	
}

DWORD WINAPI threadpro2(LPVOID pParam)
{
	SOCKET hsock=(SOCKET)pParam;
	int res;
	char recvbuffer[1024];
	while(res=recv(hsock,recvbuffer,sizeof(recvbuffer),0))
	{
		FixMessage message(recvbuffer);
		if(message.getType()=="8")
		{
			string back=message.getOrderStatus();
			if(back=="0")
			{
				EnterCriticalSection(&io_section);
				cout<<"�����ɹ�����ȴ�����"<<endl;
				LeaveCriticalSection(&io_section);
				Order order(message.getId(),message.getSide(),message.getName(),message.getPrice(),
					message.getAmount(),"","",0);
				EnterCriticalSection(&io_section);
				if(message.getSide()==0)
				{
					buyOrderList[message.getName()].push_back(order);
				}
				else
				{
					saleOrderList[message.getName()].push_back(order);
				}
				LeaveCriticalSection(&io_section);
			}
			else if(back=="1")
			{
				EnterCriticalSection(&io_section);
				cout<<"��Ķ������ֽ��׳ɹ�!ƽ�����׼۸�Ϊ:"<<message.getAvgPce()<<" ��������Ϊ"<<message.getFilledAmt()<<endl;
				LeaveCriticalSection(&io_section);
			}
			else if(back=="2")
			{
				EnterCriticalSection(&io_section);
				cout<<"��Ķ�����ȫ������!ƽ�����׼۸�Ϊ:"<<message.getAvgPce()<<" ��������Ϊ"<<message.getFilledAmt()<<endl;
				LeaveCriticalSection(&io_section);
			}
			else if(back=="8")
			{
				EnterCriticalSection(&io_section);
				cout<<"��Ķ��������Ϲ涨���ѱ��ܾ�!"<<endl;
				LeaveCriticalSection(&io_section);
			}
			else if(back=="4")
			{
				EnterCriticalSection(&io_section);
				cout<<"�ɹ�ȡ��"<<message.getName()<<"����"<<message.getId()<<"δ���׵�"<<message.getOpenAmt()<<"��"<<endl;
				LeaveCriticalSection(&io_section);
			}
		}else{
			EnterCriticalSection(&io_section);
		    cout<<"���ȡ�������ѱ��ܾ����ö����ѱ���ȫ����!"<<endl;
			LeaveCriticalSection(&io_section);
		}
	}
	return 0;
}
void initShare()
{
	fstream file("share_imformation.txt",ios::in|ios::out);    //���ļ���ȡ��Ʊ��Ϣ
	string imformation;
	while(getline(file,imformation))
	{
		string share_name;
		string share_id;
		double start_price;
		stringstream stream(imformation);
		stream>>share_id>>share_name>>start_price;
		cout<<share_id<<" "<<share_name<<" "<<start_price<<endl;   
		buyOrderList[share_name]=list<Order>();             //��ʼ��Orderlist
		saleOrderList[share_name]=list<Order>();
	}
	file.close();
}

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
	string user_name,pass_word;
	memset(buffer,'\0',1024);
	memset(sendBuffer,'\0',1024);
	memset(recvbuffer,'\0',1024);
	InitializeCriticalSection(&io_section);
	initShare();
	while(true)
	{
		cout<<"�������û�����";
		cin>>user_name;
		cout<<"������������룺";
		cin>>pass_word;
		break;
	}
	int i=connect(m_ClientServer,(sockaddr*)&serveraddr,sizeof(serveraddr));
	if(i!=INVALID_SOCKET)
		cout<<"���ӳɹ���";
	int res=recv(m_ClientServer,recvbuffer,sizeof(recvbuffer),0);
	cout<<recvbuffer;
	stringstream stream;
	stream<<user_name<<";"<<pass_word;
	stream>>sendBuffer;
	send(m_ClientServer,sendBuffer,sizeof(sendBuffer),0);
	if(i==INVALID_SOCKET){
	}else{
		HANDLE send_Handle;
		DWORD sendThreadld=0;
		send_Handle =(HANDLE)::CreateThread(NULL,0,threadpro1,(LPVOID)m_ClientServer,0,&sendThreadld);
		HANDLE recv_Handle;
		DWORD recvThreadld=0;
		recv_Handle =(HANDLE)::CreateThread(NULL,0,threadpro2,(LPVOID)m_ClientServer,0,&recvThreadld);
	}
	while(1){}
	closesocket(m_ClientServer);
	system("pause");
	return 0;
} 

#include<iostream>
#include<string>
#include<sstream>
#include<fstream>
#include<algorithm>
#include<iomanip>
#include"Order.h"
#include"winsock2.h"
#include<time.h>
#include<stdlib.h>
#include<list>
#include<map>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
SOCKET m_Server[20];
SOCKET monitor_client;
CRITICAL_SECTION  queue_section;     //������Ŀ��Ҫ�õ�ȫ�ֱ���
CRITICAL_SECTION  id_section;
int order_id=0;
map<string,Stock>share_map;
map<string,list<Order> >buyOrderList;
map<string,list<Order> >endBuyOrder;
map<string,list<Order> >saleOrderList;
map<string,list<Order> >endSaleOrder;

bool myCmp(Order&order1,Order&order2)
{
	if(order1.getSide()==0)
	{
		return order1.getPrice()>order2.getPrice();
	}
	else
	{
		return order1.getPrice()<order2.getPrice();
	}
}
list<Order>::iterator findInList(int order_id,list<Order>&order_list)
{
	for(list<Order>::iterator iter=order_list.begin();iter!=order_list.end();iter++)
	{
		if(iter->getId()==order_id)
		{
			return iter;
		}
	}
	return order_list.end();
}
DWORD WINAPI threadpro(LPVOID pParam)try
{
	User* user=(User*)pParam;
	SOCKET hsock=m_Server[user->getIndex()];
	string username=user->getName(),password=user->getPassWord();
	int socket_index=user->getIndex();
	if(hsock!=INVALID_SOCKET)
		cout<<username<<"�û����ӳɹ�!"<<endl;
	char buffer[1024];
	int num=recv(hsock,buffer,sizeof(buffer),0);//���ܿͻ�����Ϣ
	while(1)
	{
		if(num<0)//��client����ر�ʱ����Ͽ�����
		{
			cout<<"�û�"<<username<<"�Ͽ����ӣ�"<<endl;
			break;
		}
		FixMessage message(buffer);
		EnterCriticalSection(&queue_section);//����ؼ���
		if(message.getSide()==0)      //�򷽶���
		{
			if(message.getType()=="D")
			{
				if(message.getPrice()>share_map[message.getName()].getAvgPrice()*1.2||message.getPrice()<share_map[message.getName()].getAvgPrice()*0.8)
				{
					char sendBuffer[1024]="35=8;39=8;";
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					//�ܾ��������
				}else{
					Order order(order_id,message.getSide(),message.getName(),message.getPrice(),
					message.getAmount(),username,password,socket_index);
					stringstream stream;
					stream<<"35=8;39=0;150=0;54=0;14=0;44="<<message.getPrice()<<";11="<<order_id<<";1="<<message.getName()<<";38="<<message.getAmount()<<";";
					char sendBuffer[1024];
					stream>>sendBuffer;
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					char sendBuffer1[1024];//��monitor_client������Ϣ
					stream.clear();
					stream.str("");
					stream<<"�û�"<<order.user.getName()<<"�Ĺ��򶩵��ɹ���������Ϊ"<<order.getId()<<",��Ʊ����Ϊ"<<order.getStockName()<<",�����۸�Ϊ"<<order.getPrice()<<",����Ϊ"<<order.getAmount()<<".";
					stream>>sendBuffer1;
					send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
					cout<<sendBuffer1<<endl;
					if(saleOrderList[message.getName()].empty())//�����ʱû������������ֱ�ӽ������������б�
					{
						buyOrderList[message.getName()].push_back(order);
					}
					else if(saleOrderList[message.getName()].begin()->getPrice()<=message.getPrice())//������Խ���
					{
						list<Order>::iterator iter=saleOrderList[message.getName()].begin();//��ʼ��ÿ�εĽ��׶���iter
                        //��������һ���������׵Ĺ��� 
						while(order.getAmount()>0)
						{
							if(iter->getPrice()>message.getPrice()||iter==saleOrderList[message.getName()].end())//�������������ͼ��޷�����Ԥ�ڼ۸�
							{
								stringstream stream;                                 //����partial fill���
								stream<<"35=8;39=1;54=0;150=0;11="<<order_id<<";6="<<order.getAvgPrice()<<";14="<<order.getFilledAmt()<<";";
								char sendBuffer[1024];
								stream>>sendBuffer;
								send(hsock,sendBuffer,sizeof(sendBuffer),0);
								char sendBuffer1[1024];//��monitor_client������Ϣ
								stream.clear();
								stream.str("");
								stream<<"�û�"<<order.user.getName()<<"������Ϊ"<<order.getId()<<"�Ĺ��򶩵�"<<"���ֽ��׳ɹ����ɽ���Ϊ"<<order.getFilledAmt();
								stream>>sendBuffer1;
								send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
								buyOrderList[message.getName()].push_back(order);//�������б����۸�����
						        buyOrderList[message.getName()].sort(myCmp);
								break;
							}
							if(iter->getAmount()>order.getAmount())
							{
								order.writeResult(iter->getUsername(),order.getAmount(),iter->getPrice());//д�뽻�׽��
								iter->writeResult(order.getUsername(),order.getAmount(),iter->getPrice());//д�뽻�׽��
								share_map[message.getName()].upgradePrice(iter->getPrice());//���¼۸�
								stringstream stream;
								stream<<"35=8;39=2;54=0;150=0;11="<<order_id<<";6="<<order.getAvgPrice()<<";14="<<order.getFilledAmt()<<";";           
								char sendBuffer[1024];   //����fullfill���
								stream>>sendBuffer;
								send(hsock,sendBuffer,sizeof(sendBuffer),0);
								char sendBuffer1[1024];//��monitor_client������Ϣ
								stream.clear();
								stream.str("");
								stream<<"�û�"<<order.user.getName()<<"������Ϊ"<<order.getId()<<"�Ĺ��򶩵�"<<"ȫ�����׳ɹ���";
								stream>>sendBuffer1;
								send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
							}
							else if(iter->getAmount()==order.getAmount())
							{
								stringstream stream; 
								order.writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//д����
								iter->writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//д�뽻�׽��,��list�н�����������ȥ��
								share_map[message.getName()].upgradePrice(iter->getPrice());//���¼۸�
                                //����full fill���
								stream<<"35=8;39=2;150=0;11="<<order_id<<";6="<<order.getAvgPrice()<<";14="<<order.getFilledAmt()<<";";
								char sendBuffer[1024],sendBuffer1[1024];
								stream>>sendBuffer;
								send(hsock,sendBuffer,sizeof(sendBuffer),0);
								stream.str("");//�򱻽��׷�����fullfill���
								stream<<"35=8;39=2;54=1;150=0;11="<<iter->getId()<<";6="<<iter->getAvgPrice()<<";14="<<iter->getFilledAmt()<<";";
								stream>>sendBuffer1;
								send(m_Server[iter->user.getIndex()],sendBuffer1,sizeof(sendBuffer1),0);
								char sendBuffer2[1024];//��monitor_client������Ϣ
								stream.clear();
								stream.str("");
								stream<<"�û�"<<iter->user.getName()<<"������Ϊ"<<iter->getId()<<"����������"<<"ȫ�����׳ɹ���";
								stream>>sendBuffer2;
								send(monitor_client,sendBuffer2,sizeof(sendBuffer2),0);
								stream.clear();
								stream.str("");//��monitor_client������Ϣ
								stream<<"�û�"<<message.getName()<<"������Ϊ"<<iter->getId()<<"�Ĺ��򶩵�"<<"ȫ�����׳ɹ���";
								stream>>sendBuffer2;
								send(monitor_client,sendBuffer2,sizeof(sendBuffer2),0);
								endSaleOrder[message.getName()].push_back(*iter);
								saleOrderList[message.getName()].pop_front();
								iter=saleOrderList[message.getName()].begin();
								break;
							}
							else
							{
								stringstream stream; 
								order.writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//д����
								iter->writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//д�뽻�׽��,��list�н�����������ȥ��
								share_map[message.getName()].upgradePrice(iter->getPrice());//���¼۸�
								char sendBuffer[1024];
								stream<<"35=8;54=1;39=2;150=0;11="<<iter->getId()<<";6="<<iter->getAvgPrice()<<";14="<<iter->getFilledAmt()<<";";
								stream>>sendBuffer;
								send(m_Server[iter->user.getIndex()],sendBuffer,sizeof(sendBuffer),0);//�򱻽��׷�����fullfill���
								char sendBuffer1[1024];//��monitor_client������Ϣ
								stream.clear();
								stream.str("");
								stream<<"�û�"<<iter->user.getName()<<"������Ϊ"<<iter->getId()<<"����������ȫ�����׳ɹ���";
								stream>>sendBuffer1;
								send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
								endSaleOrder[message.getName()].push_back(*iter);
								saleOrderList[message.getName()].pop_front();
								iter=saleOrderList[message.getName()].begin();
							}
						}
					}
					else          //����۸��޷����׾ͷŽ�buyOrderList��
					{
						buyOrderList[message.getName()].push_back(order);
						buyOrderList[message.getName()].sort(myCmp);
					}
					order_id++;
				}
			}else if(message.getType()=="F"){
				int order_id=message.getId();
				list<Order>::iterator find_iter=findInList(order_id,buyOrderList[message.getName()]);
				if(find_iter==buyOrderList[message.getName()].end())
				{
					find_iter=findInList(order_id,endBuyOrder[message.getName()]);//�ܾ�ȡ������Ϊ������������ڻ����Ѿ����׽���
					stringstream stream;
					stream<<"35=9;39=8;";
					char sendBuffer[1024];
					stream>>sendBuffer;
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					char sendBuffer1[1024];//��monitor_client������Ϣ
					stream.clear();
					stream.str("");
					stream<<"�û�"<<find_iter->user.getName()<<"���ڶ�����Ϊ"<<find_iter->getId()<<"�Ĺ��򶩵���ȡ�����󱻾ܾ���";
					stream>>sendBuffer1;
					send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
				}
				else
				{
					stringstream stream;                     //����ȡ��
					stream<<"35=8;39=4;54=0;11="<<find_iter->getId()<<";14="<<find_iter->getFilledAmt()<<";38="<<find_iter->getFilledAmt()+find_iter->getAmount()<<";";
					char sendBuffer[1024];
					stream>>sendBuffer;
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					char sendBuffer1[1024];//��monitor_client������Ϣ
					stream.clear();
					stream.str("");
					stream<<"�û�"<<find_iter->user.getName()<<"ȡ����δ�������"<<find_iter->getId()<<"����"<<"�е�"<<find_iter->getAmount()<<"��"<<find_iter->getStockName();
					stream>>sendBuffer1;
					send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
					find_iter->orderStatus=4;
					endBuyOrder[message.getName()].push_back(*find_iter); //�����Ѿ������Ķ���list
					buyOrderList[message.getName()].erase(find_iter);     //��orderlist���Ƴ�
				}
			}else if(message.getType()=="Q"){//�ͻ���ѯ��Ϣ
				list<Order>::iterator buyIter=buyOrderList[message.getName()].begin();
				list<Order>::iterator saleIter=saleOrderList[message.getName()].begin();
				stringstream stream;
				for(int i=0;i<10;i++)
				{
					if(buyIter!=buyOrderList[message.getName()].end())
					{
					    stream<<setw(10)<<buyIter->getAmount()<<" "<<setw(10)<<buyIter->getPrice();
						buyIter++;
					}else
					{
						stream<<setw(10)<<"---"<<" "<<setw(10)<<"---";
					}
					if(saleIter!=saleOrderList[message.getName()].end())
					{
					    stream<<setw(10)<<saleIter->getAmount()<<" "<<setw(10)<<saleIter->getPrice()<<"\n";
						saleIter++;
					}else
					{
						stream<<setw(10)<<"---"<<" "<<setw(10)<<"---"<<"\n";
					}
				}
				char sendBuffer[1024];
				strcpy(sendBuffer,stream.str().c_str());
				send(hsock,sendBuffer,sizeof(sendBuffer),0);
				//��ѯ�������ص�OrderBook��Ϣ
			}
		}else{                           //��������
			if(message.getType()=="D")
			{
				if(message.getPrice()>share_map[message.getName()].getAvgPrice()*1.2||message.getPrice()<share_map[message.getName()].getAvgPrice()*0.8)
				{
					char sendBuffer[1024]="35=8;39=8;";
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					//�ܾ��������
				}else{
					Order order(order_id,message.getSide(),message.getName(),message.getPrice(),
					message.getAmount(),username,password,socket_index);
					stringstream stream;
					stream<<"35=8;39=0;150=0;54=1;14=0;44="<<message.getPrice()<<";11="<<order_id<<";1="<<message.getName()<<";38="<<message.getAmount()<<";";
					char sendBuffer[1024];
					stream>>sendBuffer;
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					char sendBuffer1[1024];//��monitor_client������Ϣ
					stream.clear();
					stream.str("");
					stream<<"�û�"<<order.user.getName()<<"�����������ɹ���������Ϊ"<<order.getId()<<",��Ʊ����Ϊ"<<order.getStockName()<<",�������Ϊ"<<order.getPrice()<<",����Ϊ"<<order.getAmount()<<".";
					stream>>sendBuffer1;
					send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
					if(buyOrderList[message.getName()].empty())
					{
						saleOrderList[message.getName()].push_back(order);
						saleOrderList[message.getName()].sort(myCmp);
					}
					else if(buyOrderList[message.getName()].begin()->getPrice()>=message.getPrice())
					{
						list<Order>::iterator iter=buyOrderList[message.getName()].begin();
						int amount=message.getAmount();                     //��������һ���������׵Ĺ��� 
						while(order.getAmount()>0)
						{
							if(iter->getPrice()<message.getPrice()||iter==buyOrderList[message.getName()].end())//�����������߼��޷�����Ԥ�ڼ۸�
							{
								stringstream stream;                                 //����partial fill���
								stream<<"35=8;39=1;54=1;150=0;11="<<order_id<<";6="<<order.getAvgPrice()<<";14="<<order.getFilledAmt()<<";";
								char sendBuffer[1024];
								stream>>sendBuffer;
								send(hsock,sendBuffer,sizeof(sendBuffer),0);
								char sendBuffer1[1024];//��monitor_client������Ϣ
								stream.clear();
								stream.str("");
								stream<<"�û�"<<order.user.getName()<<"�Ķ�����Ϊ"<<order.getId()<<"����������"<<"���ֽ��׳ɹ����ɽ���Ϊ"<<order.getFilledAmt();
								stream>>sendBuffer1;
								send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
								saleOrderList[message.getName()].push_back(order);
						        saleOrderList[message.getName()].sort(myCmp);
								break;
							}
							if(iter->getAmount()>order.getAmount())
							{
								order.writeResult(iter->getUsername(),order.getAmount(),iter->getPrice());//д�뽻�׽��
								iter->writeResult(order.getUsername(),order.getAmount(),iter->getPrice());//д�뽻�׽��
								share_map[message.getName()].upgradePrice(iter->getPrice());//���¼۸�
								stringstream stream;
								stream<<"35=8;39=2;54=1;150=0;11="<<order_id<<";6="<<order.getAvgPrice()<<";14="<<order.getFilledAmt()<<";";           
								char sendBuffer[1024];   //����fullfill���
								stream>>sendBuffer;
								send(hsock,sendBuffer,sizeof(sendBuffer),0);
								char sendBuffer1[1024];//��monitor_client������Ϣ
								stream.clear();
								stream.str("");
								stream<<"�û�"<<order.user.getName()<<"�Ķ�����Ϊ"<<order.getId()<<"����������"<<"ȫ�����׳ɹ���";
								stream>>sendBuffer1;
								send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
							}
							else if(iter->getAmount()==order.getAmount())
							{
								stringstream stream;
								order.writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//д����
								iter->writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//д�뽻�׽��,��list�н�����������ȥ��
								share_map[message.getName()].upgradePrice(iter->getPrice());//���¼۸�
                                 //����full fill���
								stream<<"35=8;39=2;54=1;150=0;11="<<order_id<<";6="<<order.getAvgPrice()<<";14="<<order.getFilledAmt()<<";";
								char sendBuffer[1024],sendBuffer1[1024];
								stream>>sendBuffer;
								send(hsock,sendBuffer,sizeof(sendBuffer),0);
								stream.str("");
								stream<<"35=8;39=2;54=0;150=0;11="<<iter->getId()<<";6="<<iter->getAvgPrice()<<";14="<<iter->getFilledAmt()<<";";
								stream>>sendBuffer1;
								send(m_Server[iter->user.getIndex()],sendBuffer1,sizeof(sendBuffer1),0);//�򱻽��׷�����fullfill���
								char sendBuffer2[1024];//��monitor_client������Ϣ
								stream.clear();
								stream.str("");
								stream<<"�û�"<<iter->user.getName()<<"�Ķ�����Ϊ"<<iter->getId()<<"�Ĺ��򶩵�"<<"ȫ�����׳ɹ���";
								stream>>sendBuffer2;
								send(monitor_client,sendBuffer2,sizeof(sendBuffer2),0);
								stream.clear();
								stream.str("");//��monitor_client������Ϣ
								stream<<"�û�"<<message.getName()<<"�Ķ�����Ϊ"<<message.getId()<<"����������"<<"ȫ�����׳ɹ���";
								stream>>sendBuffer2;
								send(monitor_client,sendBuffer2,sizeof(sendBuffer2),0);
								endBuyOrder[message.getName()].push_back(*iter);
								buyOrderList[message.getName()].pop_front();
								iter=buyOrderList[message.getName()].begin();
								break;
							}
							else
							{
								stringstream stream;
								order.writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//д�뽻�׽��
								iter->writeResult(order.getUsername(),iter->getAmount(),iter->getPrice());//д�뽻�׽��,��list�н�����������ȥ��
								share_map[message.getName()].upgradePrice(iter->getPrice());//���¼۸�
								char sendBuffer[1024];
								stream<<"35=8;54=0;39=2;150=0;11="<<iter->getId()<<";6="<<iter->getAvgPrice()<<";14="<<iter->getFilledAmt()<<";";
								stream>>sendBuffer;
								send(m_Server[iter->user.getIndex()],sendBuffer,sizeof(sendBuffer),0);//�򱻽��׷�����fullfill���
								char sendBuffer1[1024];//��monitor_client������Ϣ
								stream.clear();
								stream.str("");
								stream<<"�û�"<<iter->user.getName()<<"�Ķ�����Ϊ"<<iter->getId()<<"�Ĺ��򶩵�ȫ�����׳ɹ���";
								stream>>sendBuffer1;
								send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
								endBuyOrder[message.getName()].push_back(*iter);
								buyOrderList[message.getName()].pop_front();
								iter=buyOrderList[message.getName()].begin();
							}
						}
					}
					else          //����۸��޷����׾ͷŽ�saleOrderList��
					{
						saleOrderList[message.getName()].push_back(order);
						saleOrderList[message.getName()].sort(myCmp);
					}
					order_id++;
				}
			}else if(message.getType()=="F"){//ȡ������
				int order_id=message.getId();
				list<Order>::iterator find_iter=findInList(order_id,saleOrderList[message.getName()]);
				if(find_iter==saleOrderList[message.getName()].end())
				{
					find_iter=findInList(order_id,endSaleOrder[message.getName()]);//�ܾ�ȡ��
					stringstream stream;
					stream<<"35=9;39=8;";
					char sendBuffer[1024];
					stream>>sendBuffer;
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					char sendBuffer1[1024];//��monitor_client������Ϣ
					stream.clear();
					stream.str("");
					stream<<"�û�"<<find_iter->user.getName()<<"���ڶ�����Ϊ"<<find_iter->getId()<<"�Ķ���"<<"��ȡ�����󱻾ܾ���";
					stream>>sendBuffer1;
					send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
				}
				else
				{
					stringstream stream;                     //����ȡ��
					stream<<"35=8;39=4;11="<<find_iter->getId()<<";14="<<find_iter->getFilledAmt()<<";38="<<find_iter->getFilledAmt()+find_iter->getAmount()<<";";
					char sendBuffer[1024];
					stream>>sendBuffer;
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					char sendBuffer1[1024];//��monitor_client������Ϣ
					stream.clear();
					stream.str("");
					stream<<"�û�"<<find_iter->user.getName()<<"ȡ����δ���׵�"<<find_iter->getId()<<"����"<<"�е�"<<find_iter->getAmount()<<"��"<<find_iter->getStockName();
					stream>>sendBuffer1;
					send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
					find_iter->orderStatus=4;
					endSaleOrder[message.getName()].push_back(*find_iter); //�����Ѿ������Ķ���list
					saleOrderList[message.getName()].erase(find_iter);     //��orderlist���Ƴ�
				}
			}else if(message.getType()=="Q"){
				list<Order>::iterator buyIter=buyOrderList[message.getName()].begin();
				list<Order>::iterator saleIter=saleOrderList[message.getName()].begin();
				stringstream stream;
				for(int i=0;i<10;i++)
				{
					if(buyIter!=buyOrderList[message.getName()].end())
					{
					    stream<<setw(10)<<buyIter->getAmount()<<" "<<setw(10)<<buyIter->getPrice();
						buyIter++;
					}else
					{
						stream<<setw(10)<<"---"<<" "<<setw(10)<<"---";
					}
					if(saleIter!=saleOrderList[message.getName()].end())
					{
					    stream<<setw(10)<<saleIter->getAmount()<<" "<<setw(10)<<saleIter->getPrice()<<"\n";
						saleIter++;
					}else
					{
						stream<<setw(10)<<"---"<<" "<<setw(10)<<"---"<<"\n";
					}
				}
				char sendBuffer[1024];
				strcpy(sendBuffer,stream.str().c_str());
				send(hsock,sendBuffer,sizeof(sendBuffer),0);
				//��ѯ�������ص�OrderBook��Ϣ
			}
		}
		LeaveCriticalSection(&queue_section);//�뿪�ؼ���
		num=recv(hsock,buffer,sizeof(buffer),0);//����ѭ��
	}
}catch(...){cout<<"һλ�û��Ͽ����ӣ�"<<endl;return 1;}
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
		share_map[share_name]=Stock(share_id,share_name,start_price);   
		buyOrderList[share_name]=list<Order>();             //��ʼ��Orderlist
		saleOrderList[share_name]=list<Order>();
	}
	file.close();
}
int main()
{
	WSADATA wsd;
	WSAStartup(MAKEWORD(2,2),&wsd);
	SOCKET m_SockServer;
	sockaddr_in serveraddr;
	sockaddr_in serveraddrfrom;

	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(4600);
	serveraddr.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");

	m_SockServer=socket(AF_INET,SOCK_STREAM,0);

	int i=bind(m_SockServer,(sockaddr*)&serveraddr,sizeof(serveraddr));
	cout<<"�󶨵�ַ�ɹ�����������������"<<endl;

	int iMaxConnect=20;
	int iConnect=0;
	int iLisRet;
	char buf[]="��ã���ӭ����ģ�⽻������\n";
	char recvbuffer[1024];
	char WarnBuf[]="������ӵ�£����Ժ����ԣ�\n";
	int len=sizeof(sockaddr);
	initShare();
	
	InitializeCriticalSection(&queue_section);

	while(1)
	{
		iLisRet=listen(m_SockServer,0);
		m_Server[iConnect]=accept(m_SockServer,(sockaddr*)&serveraddrfrom,&len);
		if(m_Server[iConnect]!=INVALID_SOCKET)
		{
			string user_name,pass_word;
			int ires=send(m_Server[iConnect],buf,sizeof(buf),0);
			cout<<"�¿ͻ�ip��ַ:"<<inet_ntoa(serveraddrfrom.sin_addr)<<endl;
			int res=recv(m_Server[iConnect],recvbuffer,sizeof(recvbuffer),0);
			stringstream stream;
			stream<<recvbuffer;
			string temp=stream.str();
			int split=temp.find(';');
			user_name=temp.substr(0,split);
			pass_word=temp.substr(split+1);
			cout<<"�û�����:"<<user_name<<endl;
			if(iConnect>iMaxConnect)
			{
				int ires=send(m_Server[iConnect],WarnBuf,sizeof(WarnBuf),0);
			}
			else if(user_name=="monitor")
			{
				monitor_client=m_Server[iConnect];
				iConnect++;
			}
			else{
				HANDLE m_Handle;
				DWORD nThreadld=0;
				m_Handle =(HANDLE)::CreateThread(NULL,0,threadpro,(LPVOID)&User(user_name,pass_word,iConnect),0,&nThreadld);
				iConnect++;
			}
		}
	}
	ofstream ofile("share_imformation.txt",ios::out);
	for(map<string,Stock>::iterator iter=share_map.begin();iter!=share_map.end();iter++)
	{
		ofile<<iter->second.getId()<<" "<<iter->second.getStockName()<<" "<<iter->second.getEndPrice();
		ofile<<" "<<iter->second.getMaxPrice()<<" "<<iter->second.getMinPrice()<<endl;
	}
	ofile.close();
	WSACleanup();
	system("pause");
	return 0;
}


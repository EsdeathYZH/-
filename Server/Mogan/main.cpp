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
CRITICAL_SECTION  queue_section;     //这是项目中要用的全局变量
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
		cout<<username<<"用户连接成功!"<<endl;
	char buffer[1024];
	int num=recv(hsock,buffer,sizeof(buffer),0);//接受客户的信息
	while(1)
	{
		if(num<0)//当client程序关闭时，会断开连接
		{
			cout<<"用户"<<username<<"断开连接！"<<endl;
			break;
		}
		FixMessage message(buffer);
		EnterCriticalSection(&queue_section);//进入关键段
		if(message.getSide()==0)      //买方订单
		{
			if(message.getType()=="D")
			{
				if(message.getPrice()>share_map[message.getName()].getAvgPrice()*1.2||message.getPrice()<share_map[message.getName()].getAvgPrice()*0.8)
				{
					char sendBuffer[1024]="35=8;39=8;";
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					//拒绝这个订单
				}else{
					Order order(order_id,message.getSide(),message.getName(),message.getPrice(),
					message.getAmount(),username,password,socket_index);
					stringstream stream;
					stream<<"35=8;39=0;150=0;54=0;14=0;44="<<message.getPrice()<<";11="<<order_id<<";1="<<message.getName()<<";38="<<message.getAmount()<<";";
					char sendBuffer[1024];
					stream>>sendBuffer;
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					char sendBuffer1[1024];//向monitor_client发送消息
					stream.clear();
					stream.str("");
					stream<<"用户"<<order.user.getName()<<"的购买订单成功！订单号为"<<order.getId()<<",股票名称为"<<order.getStockName()<<",订单价格为"<<order.getPrice()<<",数量为"<<order.getAmount()<<".";
					stream>>sendBuffer1;
					send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
					cout<<sendBuffer1<<endl;
					if(saleOrderList[message.getName()].empty())//如果此时没有卖方订单，直接将订单放入买方列表
					{
						buyOrderList[message.getName()].push_back(order);
					}
					else if(saleOrderList[message.getName()].begin()->getPrice()<=message.getPrice())//如果可以交易
					{
						list<Order>::iterator iter=saleOrderList[message.getName()].begin();//初始化每次的交易对象iter
                        //接下来是一个持续交易的过程 
						while(order.getAmount()>0)
						{
							if(iter->getPrice()>message.getPrice()||iter==saleOrderList[message.getName()].end())//如果现有卖方最低价无法到达预期价格
							{
								stringstream stream;                                 //发送partial fill结果
								stream<<"35=8;39=1;54=0;150=0;11="<<order_id<<";6="<<order.getAvgPrice()<<";14="<<order.getFilledAmt()<<";";
								char sendBuffer[1024];
								stream>>sendBuffer;
								send(hsock,sendBuffer,sizeof(sendBuffer),0);
								char sendBuffer1[1024];//向monitor_client发送消息
								stream.clear();
								stream.str("");
								stream<<"用户"<<order.user.getName()<<"订单号为"<<order.getId()<<"的购买订单"<<"部分交易成功！成交量为"<<order.getFilledAmt();
								stream>>sendBuffer1;
								send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
								buyOrderList[message.getName()].push_back(order);//放入买方列表并按价格排序
						        buyOrderList[message.getName()].sort(myCmp);
								break;
							}
							if(iter->getAmount()>order.getAmount())
							{
								order.writeResult(iter->getUsername(),order.getAmount(),iter->getPrice());//写入交易结果
								iter->writeResult(order.getUsername(),order.getAmount(),iter->getPrice());//写入交易结果
								share_map[message.getName()].upgradePrice(iter->getPrice());//更新价格
								stringstream stream;
								stream<<"35=8;39=2;54=0;150=0;11="<<order_id<<";6="<<order.getAvgPrice()<<";14="<<order.getFilledAmt()<<";";           
								char sendBuffer[1024];   //发送fullfill结果
								stream>>sendBuffer;
								send(hsock,sendBuffer,sizeof(sendBuffer),0);
								char sendBuffer1[1024];//向monitor_client发送消息
								stream.clear();
								stream.str("");
								stream<<"用户"<<order.user.getName()<<"订单号为"<<order.getId()<<"的购买订单"<<"全部交易成功！";
								stream>>sendBuffer1;
								send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
							}
							else if(iter->getAmount()==order.getAmount())
							{
								stringstream stream; 
								order.writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//写入结果
								iter->writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//写入交易结果,在list中将这个交易完的去掉
								share_map[message.getName()].upgradePrice(iter->getPrice());//更新价格
                                //发送full fill结果
								stream<<"35=8;39=2;150=0;11="<<order_id<<";6="<<order.getAvgPrice()<<";14="<<order.getFilledAmt()<<";";
								char sendBuffer[1024],sendBuffer1[1024];
								stream>>sendBuffer;
								send(hsock,sendBuffer,sizeof(sendBuffer),0);
								stream.str("");//向被交易方发送fullfill结果
								stream<<"35=8;39=2;54=1;150=0;11="<<iter->getId()<<";6="<<iter->getAvgPrice()<<";14="<<iter->getFilledAmt()<<";";
								stream>>sendBuffer1;
								send(m_Server[iter->user.getIndex()],sendBuffer1,sizeof(sendBuffer1),0);
								char sendBuffer2[1024];//向monitor_client发送消息
								stream.clear();
								stream.str("");
								stream<<"用户"<<iter->user.getName()<<"订单号为"<<iter->getId()<<"的售卖订单"<<"全部交易成功！";
								stream>>sendBuffer2;
								send(monitor_client,sendBuffer2,sizeof(sendBuffer2),0);
								stream.clear();
								stream.str("");//向monitor_client发送消息
								stream<<"用户"<<message.getName()<<"订单号为"<<iter->getId()<<"的购买订单"<<"全部交易成功！";
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
								order.writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//写入结果
								iter->writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//写入交易结果,在list中将这个交易完的去掉
								share_map[message.getName()].upgradePrice(iter->getPrice());//更新价格
								char sendBuffer[1024];
								stream<<"35=8;54=1;39=2;150=0;11="<<iter->getId()<<";6="<<iter->getAvgPrice()<<";14="<<iter->getFilledAmt()<<";";
								stream>>sendBuffer;
								send(m_Server[iter->user.getIndex()],sendBuffer,sizeof(sendBuffer),0);//向被交易方发送fullfill结果
								char sendBuffer1[1024];//向monitor_client发送消息
								stream.clear();
								stream.str("");
								stream<<"用户"<<iter->user.getName()<<"订单号为"<<iter->getId()<<"的售卖订单全部交易成功！";
								stream>>sendBuffer1;
								send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
								endSaleOrder[message.getName()].push_back(*iter);
								saleOrderList[message.getName()].pop_front();
								iter=saleOrderList[message.getName()].begin();
							}
						}
					}
					else          //如果价格无法交易就放进buyOrderList中
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
					find_iter=findInList(order_id,endBuyOrder[message.getName()]);//拒绝取消，因为这个订单不存在或者已经交易结束
					stringstream stream;
					stream<<"35=9;39=8;";
					char sendBuffer[1024];
					stream>>sendBuffer;
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					char sendBuffer1[1024];//向monitor_client发送消息
					stream.clear();
					stream.str("");
					stream<<"用户"<<find_iter->user.getName()<<"对于订单号为"<<find_iter->getId()<<"的购买订单的取消请求被拒绝！";
					stream>>sendBuffer1;
					send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
				}
				else
				{
					stringstream stream;                     //部分取消
					stream<<"35=8;39=4;54=0;11="<<find_iter->getId()<<";14="<<find_iter->getFilledAmt()<<";38="<<find_iter->getFilledAmt()+find_iter->getAmount()<<";";
					char sendBuffer[1024];
					stream>>sendBuffer;
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					char sendBuffer1[1024];//向monitor_client发送消息
					stream.clear();
					stream.str("");
					stream<<"用户"<<find_iter->user.getName()<<"取消了未交易完的"<<find_iter->getId()<<"订单"<<"中的"<<find_iter->getAmount()<<"股"<<find_iter->getStockName();
					stream>>sendBuffer1;
					send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
					find_iter->orderStatus=4;
					endBuyOrder[message.getName()].push_back(*find_iter); //加入已经结束的订单list
					buyOrderList[message.getName()].erase(find_iter);     //从orderlist中移除
				}
			}else if(message.getType()=="Q"){//客户查询消息
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
				//查询订单返回的OrderBook信息
			}
		}else{                           //卖方订单
			if(message.getType()=="D")
			{
				if(message.getPrice()>share_map[message.getName()].getAvgPrice()*1.2||message.getPrice()<share_map[message.getName()].getAvgPrice()*0.8)
				{
					char sendBuffer[1024]="35=8;39=8;";
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					//拒绝这个订单
				}else{
					Order order(order_id,message.getSide(),message.getName(),message.getPrice(),
					message.getAmount(),username,password,socket_index);
					stringstream stream;
					stream<<"35=8;39=0;150=0;54=1;14=0;44="<<message.getPrice()<<";11="<<order_id<<";1="<<message.getName()<<";38="<<message.getAmount()<<";";
					char sendBuffer[1024];
					stream>>sendBuffer;
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					char sendBuffer1[1024];//向monitor_client发送消息
					stream.clear();
					stream.str("");
					stream<<"用户"<<order.user.getName()<<"的售卖订单成功！订单号为"<<order.getId()<<",股票名称为"<<order.getStockName()<<",订单金额为"<<order.getPrice()<<",数量为"<<order.getAmount()<<".";
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
						int amount=message.getAmount();                     //接下来是一个持续交易的过程 
						while(order.getAmount()>0)
						{
							if(iter->getPrice()<message.getPrice()||iter==buyOrderList[message.getName()].end())//如果现有买方最高价无法到达预期价格
							{
								stringstream stream;                                 //发送partial fill结果
								stream<<"35=8;39=1;54=1;150=0;11="<<order_id<<";6="<<order.getAvgPrice()<<";14="<<order.getFilledAmt()<<";";
								char sendBuffer[1024];
								stream>>sendBuffer;
								send(hsock,sendBuffer,sizeof(sendBuffer),0);
								char sendBuffer1[1024];//向monitor_client发送消息
								stream.clear();
								stream.str("");
								stream<<"用户"<<order.user.getName()<<"的订单号为"<<order.getId()<<"的售卖订单"<<"部分交易成功！成交量为"<<order.getFilledAmt();
								stream>>sendBuffer1;
								send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
								saleOrderList[message.getName()].push_back(order);
						        saleOrderList[message.getName()].sort(myCmp);
								break;
							}
							if(iter->getAmount()>order.getAmount())
							{
								order.writeResult(iter->getUsername(),order.getAmount(),iter->getPrice());//写入交易结果
								iter->writeResult(order.getUsername(),order.getAmount(),iter->getPrice());//写入交易结果
								share_map[message.getName()].upgradePrice(iter->getPrice());//更新价格
								stringstream stream;
								stream<<"35=8;39=2;54=1;150=0;11="<<order_id<<";6="<<order.getAvgPrice()<<";14="<<order.getFilledAmt()<<";";           
								char sendBuffer[1024];   //发送fullfill结果
								stream>>sendBuffer;
								send(hsock,sendBuffer,sizeof(sendBuffer),0);
								char sendBuffer1[1024];//向monitor_client发送消息
								stream.clear();
								stream.str("");
								stream<<"用户"<<order.user.getName()<<"的订单号为"<<order.getId()<<"的售卖订单"<<"全部交易成功！";
								stream>>sendBuffer1;
								send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
							}
							else if(iter->getAmount()==order.getAmount())
							{
								stringstream stream;
								order.writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//写入结果
								iter->writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//写入交易结果,在list中将这个交易完的去掉
								share_map[message.getName()].upgradePrice(iter->getPrice());//更新价格
                                 //发送full fill结果
								stream<<"35=8;39=2;54=1;150=0;11="<<order_id<<";6="<<order.getAvgPrice()<<";14="<<order.getFilledAmt()<<";";
								char sendBuffer[1024],sendBuffer1[1024];
								stream>>sendBuffer;
								send(hsock,sendBuffer,sizeof(sendBuffer),0);
								stream.str("");
								stream<<"35=8;39=2;54=0;150=0;11="<<iter->getId()<<";6="<<iter->getAvgPrice()<<";14="<<iter->getFilledAmt()<<";";
								stream>>sendBuffer1;
								send(m_Server[iter->user.getIndex()],sendBuffer1,sizeof(sendBuffer1),0);//向被交易方发送fullfill结果
								char sendBuffer2[1024];//向monitor_client发送消息
								stream.clear();
								stream.str("");
								stream<<"用户"<<iter->user.getName()<<"的订单号为"<<iter->getId()<<"的购买订单"<<"全部交易成功！";
								stream>>sendBuffer2;
								send(monitor_client,sendBuffer2,sizeof(sendBuffer2),0);
								stream.clear();
								stream.str("");//向monitor_client发送消息
								stream<<"用户"<<message.getName()<<"的订单号为"<<message.getId()<<"的售卖订单"<<"全部交易成功！";
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
								order.writeResult(iter->getUsername(),iter->getAmount(),iter->getPrice());//写入交易结果
								iter->writeResult(order.getUsername(),iter->getAmount(),iter->getPrice());//写入交易结果,在list中将这个交易完的去掉
								share_map[message.getName()].upgradePrice(iter->getPrice());//更新价格
								char sendBuffer[1024];
								stream<<"35=8;54=0;39=2;150=0;11="<<iter->getId()<<";6="<<iter->getAvgPrice()<<";14="<<iter->getFilledAmt()<<";";
								stream>>sendBuffer;
								send(m_Server[iter->user.getIndex()],sendBuffer,sizeof(sendBuffer),0);//向被交易方发送fullfill结果
								char sendBuffer1[1024];//向monitor_client发送消息
								stream.clear();
								stream.str("");
								stream<<"用户"<<iter->user.getName()<<"的订单号为"<<iter->getId()<<"的购买订单全部交易成功！";
								stream>>sendBuffer1;
								send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
								endBuyOrder[message.getName()].push_back(*iter);
								buyOrderList[message.getName()].pop_front();
								iter=buyOrderList[message.getName()].begin();
							}
						}
					}
					else          //如果价格无法交易就放进saleOrderList中
					{
						saleOrderList[message.getName()].push_back(order);
						saleOrderList[message.getName()].sort(myCmp);
					}
					order_id++;
				}
			}else if(message.getType()=="F"){//取消订单
				int order_id=message.getId();
				list<Order>::iterator find_iter=findInList(order_id,saleOrderList[message.getName()]);
				if(find_iter==saleOrderList[message.getName()].end())
				{
					find_iter=findInList(order_id,endSaleOrder[message.getName()]);//拒绝取消
					stringstream stream;
					stream<<"35=9;39=8;";
					char sendBuffer[1024];
					stream>>sendBuffer;
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					char sendBuffer1[1024];//向monitor_client发送消息
					stream.clear();
					stream.str("");
					stream<<"用户"<<find_iter->user.getName()<<"对于订单号为"<<find_iter->getId()<<"的订单"<<"的取消请求被拒绝！";
					stream>>sendBuffer1;
					send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
				}
				else
				{
					stringstream stream;                     //部分取消
					stream<<"35=8;39=4;11="<<find_iter->getId()<<";14="<<find_iter->getFilledAmt()<<";38="<<find_iter->getFilledAmt()+find_iter->getAmount()<<";";
					char sendBuffer[1024];
					stream>>sendBuffer;
					send(hsock,sendBuffer,sizeof(sendBuffer),0);
					char sendBuffer1[1024];//向monitor_client发送消息
					stream.clear();
					stream.str("");
					stream<<"用户"<<find_iter->user.getName()<<"取消了未交易的"<<find_iter->getId()<<"订单"<<"中的"<<find_iter->getAmount()<<"股"<<find_iter->getStockName();
					stream>>sendBuffer1;
					send(monitor_client,sendBuffer1,sizeof(sendBuffer1),0);
					find_iter->orderStatus=4;
					endSaleOrder[message.getName()].push_back(*find_iter); //加入已经结束的订单list
					saleOrderList[message.getName()].erase(find_iter);     //从orderlist中移除
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
				//查询订单返回的OrderBook信息
			}
		}
		LeaveCriticalSection(&queue_section);//离开关键段
		num=recv(hsock,buffer,sizeof(buffer),0);//接上循环
	}
}catch(...){cout<<"一位用户断开连接！"<<endl;return 1;}
void initShare()
{
	fstream file("share_imformation.txt",ios::in|ios::out);    //从文件读取股票信息
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
		buyOrderList[share_name]=list<Order>();             //初始化Orderlist
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
	cout<<"绑定地址成功！服务器可以运行"<<endl;

	int iMaxConnect=20;
	int iConnect=0;
	int iLisRet;
	char buf[]="你好！欢迎来到模拟交易所！\n";
	char recvbuffer[1024];
	char WarnBuf[]="服务器拥堵，请稍后再试！\n";
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
			cout<<"新客户ip地址:"<<inet_ntoa(serveraddrfrom.sin_addr)<<endl;
			int res=recv(m_Server[iConnect],recvbuffer,sizeof(recvbuffer),0);
			stringstream stream;
			stream<<recvbuffer;
			string temp=stream.str();
			int split=temp.find(';');
			user_name=temp.substr(0,split);
			pass_word=temp.substr(split+1);
			cout<<"用户名称:"<<user_name<<endl;
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


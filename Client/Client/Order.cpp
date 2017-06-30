#include"Order.h"
#include<fstream>
#include<sstream>
#include<list>
#include<map>
Order::Order(int order_id,int exchangeSide,string stockname,double givePrice,int amount,string username,string password,int socket_index):
id(order_id),side(exchangeSide),givePrice(givePrice),amount(amount),
stock_name(stockname),user(username,password,socket_index),execute_amount(0),orderStatus(EXECUTING)
{
	
}
string Order::getStockName()
{
	return this->stock_name;
}
double Order::getPrice()
{
	return this->givePrice;
}
double Order::getAvgPrice()
{
	return this->avg_price;
}
int Order::getAmount()
{
	return this->amount;
}
int Order::getFilledAmt()
{
	return this->execute_amount;
}
int Order::getId()
{
	return this->id;
}
int Order::getSide()
{
	return this->side;
}
string Order::getUsername()
{
	return user.getName();
}
void Order::writeResult(string opposite_name,int ex_amount,double ex_price)      //这个函数需要实现的功能：写入交易记录，改变order状态，改变现在股票价格，改变amount与execAmount
{
	map<int,string>side_map;
	side_map[0]="买进";
	side_map[1]="卖出";
	ofstream ofile;
	stringstream  filename;
	filename<<stock_name<<"record.txt";
	ofile.open(filename.str(),ios::app);
	ofile<<this->user.getName()<<side_map[side]<<" "<<opposite_name<<" "<<stock_name<<" "<<ex_amount<<"股"<<endl;
	ofile.close();
	this->avg_price=(double)(this->avg_price*this->execute_amount+ex_price*ex_amount)/(this->execute_amount+ex_amount);
	this->amount-=ex_amount;
	this->execute_amount+=ex_amount;
	this->orderStatus=1;//将orderstatus改为partialfill
	if(amount=0)  this->orderStatus=2;//如果已经全部交易，则改为fullfill
}

FixMessage::FixMessage(const char* message)
{
	string string_message(message);
	stringstream stream(string_message);
	list<string>message_list;
	string buffer;
	while(getline(stream,buffer,';'))
	{
		message_list.push_back(buffer);
	}
	for(list<string>::iterator iter=message_list.begin();iter!=message_list.end();iter++)
	{
		string temp_string=*iter;
		int split_pos=temp_string.find('=');
		if(split_pos==string::npos)
			throw"invalid message!";
		message_map.insert(make_pair<string,string>(temp_string.substr(0,split_pos),temp_string.substr(split_pos+1)));
	}
	
}
int FixMessage::getSide()
{
	string str=message_map["54"];
	stringstream stream(str);      //user side
	int int_side;
	stream>>int_side;
	return int_side;
}
double FixMessage::getAvgPce()
{
	string str=message_map["6"];//average price
	stringstream stream(str);
	stream>>avg_price;
	return price;
}
double FixMessage::getPrice()
{
	string str=message_map["44"];//price
	stringstream stream(str);
	stream>>price;
	return price;
}
int FixMessage::getFilledAmt()
{
	string str=message_map["14"];   //filled amount
	stringstream stream(str);
	stream>>amount;
	return amount;
}
int FixMessage::getId()
{
	string str=message_map["11"];   //orderid
	stringstream stream(str);
	int order_id;
	stream>>order_id;
	return order_id;
}
string FixMessage::getUsername()
{
	return message_map["49"];     //username
}
int FixMessage::getOpenAmt()
{
	string str=message_map["151"]; //not been filled order
	stringstream stream(str);
	stream>>amount;
	return amount;
}
int FixMessage::getAmount()
{
	string str=message_map["38"];  //order amount
	stringstream stream(str);
	stream>>amount;
	return amount;
}
string FixMessage::getName()
{
	return message_map["1"];
}
string FixMessage::getType()    //orderType
{
	return message_map["35"];
}
string FixMessage::getExecType()    //execution type
{
	return message_map["150"];    
}
string FixMessage::getOrderStatus()
{
	return message_map["39"];   //0=new,1=partial filled,2=filled,4=canceled,8=rejected
}
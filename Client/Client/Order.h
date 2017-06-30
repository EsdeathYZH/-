#include<iostream>
#include<string>
#include<vector>
#include<map>
#include"Stock.h"
#include"User.h"
using namespace std;
class Order{
public:   //���׷�
	enum Tif{DAY=1,IOC,OPG,GTC,GTX};   //����ʱ������
	enum Status{EXECUTING=0,SUCCESS=1,PENDING=2,CANCELLED=3,REJECTED=4};
	Order(int,int,string,double,int,string,string,int);
	void acceptResult(string);
	void writeResult(string,int,double);
	int orderStatus;
	double getPrice();
	double getAvgPrice();
	int getAmount();
	int getId();
	int getSide();
	int getFilledAmt();
	string getUsername();
	string getStockName();
	User user;       //ʵ���û�
private:
	int id;
	int side;       //���׷�
	int amount;
	double avg_price;     //��ǰƽ�����׼۸�
	double givePrice;     //orderӵ�з�����
	string stock_name;          //��Ʊ��Ʒ
	//Tif tifKind;             //����ʱ������
	string information;
	int execute_amount;
};
class FixMessage{
public:
	FixMessage(const char*);
	//������Ϣ����
	int getSide();
	//Order::Tif getTif();
	double getPrice();
	int getAmount();
	string getName();
	bool ifLegal();
	string getType();
	string getExecType();
	string getOrderStatus();
	int getFilledAmt();
	int getOpenAmt();
	double getAvgPce();
	int getId();
	string getUsername();
private:
	string message_type;
	map<string,string>message_map;
	int side;
	//Order::Tif tif;
	double price;
	double avg_price;
	int amount;
	int order_id;
	string stock_name;
	string user_name;
};
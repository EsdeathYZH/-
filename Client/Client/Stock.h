#include<iostream>
#include<string>
using namespace std;
class Stock{
public:
	Stock();
	Stock(string,string,double);
	void upgradePrice(double price);
	double getAvgPrice();
	double getMaxPrice();
	double getMinPrice();
	double getStaPrice();
	double getEndPrice();
	string getStockName();
	string getId();
private:
	string stock_id;
	string stock_name;
	double average_price;  //��Ʒʵʱ�۸�
	double start_price;    //���̼�
	double end_price;   //���̼ۣ����գ�
	double max_price;   //��߼�
	double min_price;   //��ͼ�
	
};
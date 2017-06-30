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
	double average_price;  //商品实时价格
	double start_price;    //开盘价
	double end_price;   //闭盘价（昨日）
	double max_price;   //最高价
	double min_price;   //最低价
	
};
#include<iostream>
#include<string>
using namespace std;
class Stock{
public:
	Stock();
	Stock(string,double);
	double upgradePrice();
	double getAvgPrice();
	double getMaxPrice();
	double getMinPrice();
	double getStaPrice();
	double getEndPrice();
	string getStockName();
private:
	string stockName;
	double averagePrice;  //商品实时价格
	double startPrice;    //开盘价
	double endPrice;   //闭盘价（昨日）
	double maxPrice;   //涨停板
	double minPrice;   //跌停板
	
};
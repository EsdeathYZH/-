#include"Stock.h"
#include<sstream>
using namespace std;
Stock::Stock()
{
	stockName="�����ڵġ�";
}
Stock::Stock(string stock_name,double start_price):stockName(stock_name),startPrice(start_price)
{
	averagePrice=startPrice;
	endPrice=startPrice;
	maxPrice=startPrice*1.2;
	minPrice=startPrice*0.8;
}
double Stock::getStaPrice()
{
	return startPrice;
	//�������ǻ�ȡ�����̼۸�Ĵ���
}
double Stock::getEndPrice()
{
	return averagePrice;
}
string Stock::getStockName()
{
	return this->stockName;
}
double Stock::getAvgPrice()
{
	return averagePrice;
}
double Stock::upgradePrice()
{
	return averagePrice;
}
double Stock::getMaxPrice()
{
	return maxPrice;
}
double Stock::getMinPrice()
{
	return minPrice;
}
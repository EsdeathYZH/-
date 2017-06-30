#include"Stock.h"
#include<sstream>
using namespace std;
Stock::Stock()
{
	stockName="不存在的。";
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
	//接下来是获取到开盘价格的代码
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
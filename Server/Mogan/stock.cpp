#include"Stock.h"
#include<sstream>
using namespace std;
Stock::Stock()
{
	stock_name="";
}
Stock::Stock(string id,string stock_name,double start_price):stock_id(id),stock_name(stock_name),start_price(start_price)
{
	this->average_price=start_price;
	this->end_price=start_price;
	this->max_price=start_price;
	this->min_price=start_price;
}
double Stock::getStaPrice()
{
	return this->start_price;
}
double Stock::getEndPrice()
{
	return this->end_price;
}
string Stock::getId()
{
	return this->stock_id;
}
string Stock::getStockName()
{
	return this->stock_name;
}
double Stock::getAvgPrice()
{
	return this->average_price;
}
double Stock::getMaxPrice()
{
	return this->max_price;
}
double Stock::getMinPrice()
{
	return this->min_price;
}
void Stock::upgradePrice(double price)
{
	this->average_price=price;
	this->max_price=max(this->max_price,this->average_price);
	this->min_price=min(this->min_price,this->average_price);
}
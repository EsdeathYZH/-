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
	double averagePrice;  //��Ʒʵʱ�۸�
	double startPrice;    //���̼�
	double endPrice;   //���̼ۣ����գ�
	double maxPrice;   //��ͣ��
	double minPrice;   //��ͣ��
	
};
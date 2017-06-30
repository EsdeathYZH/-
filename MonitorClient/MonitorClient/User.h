#include<iostream>
#include<string>
using namespace std;
class User{
private:
	string userName;
	string passWord;
	bool isAministrator;
public:
	User(string,string);
	void readHistory();
	void writeHistory(string);
	void setAministrator();
	string getName();
};
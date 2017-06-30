#include<iostream>
#include<string>
using namespace std;
class User{
private:
	string userName;
	string passWord;
	bool isAministrator;
	int socket_index;
public:
	User(string,string,int);
	void readHistory();
	void writeHistory(string);
	void setAministrator();
	string getName();
	string getPassWord();
	int getIndex();
};
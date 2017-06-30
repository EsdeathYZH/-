#include"User.h"
#include<iostream>
#include<string>
User::User(string userName,string passWord):
userName(userName),passWord(passWord),isAministrator(false)
{}
void User::writeHistory(string history)
{

}
void User::readHistory()
{}
void User::setAministrator()
{
	this->isAministrator=true;
}
string User::getName()
{
	return userName;
}
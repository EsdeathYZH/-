#include"User.h"
#include<iostream>
#include<string>
User::User(string userName,string passWord,int index):
userName(userName),passWord(passWord),isAministrator(false),socket_index(index)
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
	return this->userName;
}
string User::getPassWord()
{
	return this->passWord;
}
int User::getIndex()
{
	return this->socket_index;
}
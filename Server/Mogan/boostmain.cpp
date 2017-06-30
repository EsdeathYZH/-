//#include<iostream>
//#include<boost/regex.hpp>
//#include<boost/asio.hpp>
//using namespace std;
//int main()
//{
//	boost::asio::io_service io_s;
//	io_s.run();
//	char sendbuffer[50];
//	char recvbuffer[50];
//	memset(sendbuffer,'\0',50);
//	memset(recvbuffer,'\0',50);
//	boost::asio::ip::tcp::socket client(io_s);
//	boost::asio::ip::tcp::endpoint server(boost::asio::ip::address.from_string("192.168.1.247"),47);
//	client.connect(server);
//	client.write_some(boost::asio::buffer("516030910293"));
//	client.read_some(boost::asio::buffer(recvbuffer));
//	cin>>sendbuffer;
//	client.write_some(boost::asio::buffer(sendbuffer));
//	system("pause");
//}
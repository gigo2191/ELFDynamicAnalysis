#include <stdio.h>
#include <iostream>
#include <map>
#include <string>
#include <regex>

enum STD_DESCRIPTOR{
	STDIN = 0,
	STDOUT = 1,
	STDERROR = 2
};

class Descriptor;
class FileDescriptor;
class SocketDescriptor;
class ManageDescriptor;
class ManageFileDescriptor;
class ManageSocketDescriptor;

typedef std::map<int, Descriptor*> DescriptorMap;
typedef std::map<int, FileDescriptor*> FileDescriptorMap;
typedef std::map<int, SocketDescriptor*> SocketDescriptorMap;

class Descriptor{
	public:
		Descriptor();
		Descriptor(int fd);
		void setState(std::string stat);
		std::string getState();
		int getDescriptor();
	
	private:
		int fd_;
		std::string state_;
};

class FileDescriptor : public Descriptor{
	public:
		FileDescriptor(int fd);
		FileDescriptor(int fd, std::string name);
		std::string getName();

	private:
		int fd_;
		std::string name_;
		std::string state_;
};

class SocketDescriptor : public Descriptor{
	public:
		SocketDescriptor(int fd);
		SocketDescriptor(int fd, std::string addr);
		std::string getIP();
		void setIP(std::string addr);

	private:
		int fd_;
		std::string ip_;
		std::string state_;
};

class ManageDescriptor{
	public:
		ManageDescriptor();
		DescriptorMap& getDescriptorMap();
		bool pushDescriptor(int fd);
		Descriptor* popDescriptor(int fd);
		bool setState(int fd, std::string stat);
		std::string getState(int fd);
	private:
		DescriptorMap fds_;
};

class ManageFileDescriptor : public ManageDescriptor{
	public:
		void pushStandard();
		bool pushDescriptor(int fd, std::string name);
		std::string getName(int fd);
		FileDescriptorMap& getDescriptorMap();
		FileDescriptor* popDescriptor(int fd);
	private:
		FileDescriptorMap fds_;
};

class ManageSocketDescriptor : public ManageDescriptor{
	public:
		bool pushDescriptor(int fd);
		bool pushDescriptor(int fd, std::string addr);
		bool setIP(int fd, std::string addr);
		std::string getIP(int fd);
		bool chkState(int fd);
		SocketDescriptorMap& getDescriptorMap();
		std::string match(std::string addr);
	private:
		SocketDescriptorMap fds_;
};


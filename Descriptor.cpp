#include "Descriptor.h"
using namespace std;

Descriptor::Descriptor(){}

Descriptor::Descriptor(int fd):
fd_(fd),
state_("")
{
}

void Descriptor::setState(string state){
	state_ = state;
}

string Descriptor::getState(){
	return state_;
}

int Descriptor::getDescriptor(){
	return fd_;
}


FileDescriptor::FileDescriptor(int fd):fd_(fd),name_(""),state_(""){}

FileDescriptor::FileDescriptor(int fd, string name):fd_(fd),name_(name),state_(""){}

string FileDescriptor::getName(){
	return name_;
}

SocketDescriptor::SocketDescriptor(int fd):fd_(fd),ip_(""),state_(""){}

SocketDescriptor::SocketDescriptor(int fd, string addr):fd_(fd),ip_(addr),state_(""){}

string SocketDescriptor::getIP(){
	return ip_;
}

void SocketDescriptor::setIP(string addr){
	ip_ = addr;
}

ManageDescriptor::ManageDescriptor(){}

DescriptorMap& ManageDescriptor::getDescriptorMap(){
	DescriptorMap& fds = fds_;
	return fds;
}

bool ManageDescriptor::pushDescriptor(int fd){
	DescriptorMap& fds = getDescriptorMap();
	if (fds.find(fd) == fds.end()){
		fds.insert(pair<int, Descriptor*>(fd, new Descriptor(fd)));
		return true;
	}return false;
}

Descriptor* ManageDescriptor::popDescriptor(int fd){
	DescriptorMap& fds = getDescriptorMap();
	if (fd >= 0 && fd <=2)
		return NULL;
	if (fds.find(fd) == fds.end())
		return NULL;
	Descriptor* descriptor = *(&fds.find(fd)->second);
	fds.erase(fds.find(fd));
	return descriptor;
}

bool ManageDescriptor::setState(int fd, string state){
	DescriptorMap& fds = getDescriptorMap();
	if (fds.find(fd) == fds.end())
		return false;
	Descriptor* descriptor = *(&fds.find(fd)->second);
	descriptor->setState(state);
	return true;
}

string ManageDescriptor::getState(int fd){
	DescriptorMap& fds = getDescriptorMap();
	if (fds.find(fd) == fds.end())
		return "";
	Descriptor* descriptor = *(&fds.find(fd)->second);
	return descriptor->getState();
}

FileDescriptorMap& ManageFileDescriptor::getDescriptorMap(){
        FileDescriptorMap& fds = fds_;
        return fds;
}

void ManageFileDescriptor::pushStandard(){
	string in = "stdin";
	string out = "stdout";
	string err = "stderr";
	pushDescriptor(0, in);
	pushDescriptor(1, out);
	pushDescriptor(2, err);
}

bool ManageFileDescriptor::pushDescriptor(int fd, string name){
	FileDescriptorMap& fds = getDescriptorMap();
	if (fds.find(fd) == fds.end()){
		fds.insert(pair<int, FileDescriptor*>(fd, new FileDescriptor(fd, name)));
		return true;
	}
	return false;
}


FileDescriptor* ManageFileDescriptor::popDescriptor(int fd){
	FileDescriptorMap& fds = getDescriptorMap();
	if (fds.find(fd) == fds.end())
		return NULL;
	FileDescriptor* descriptor = *(&fds.find(fd)->second);
	fds.erase(fds.find(fd));
	return descriptor;
}

string ManageFileDescriptor::getName(int fd){
	FileDescriptorMap& fds = getDescriptorMap();
	if (fds.find(fd) == fds.end())
		return "";
	FileDescriptor* descriptor = *(&fds.find(fd)->second);
	return descriptor->getName();
}

SocketDescriptorMap& ManageSocketDescriptor::getDescriptorMap(){
        SocketDescriptorMap& fds = fds_;
        return fds;
}

bool ManageSocketDescriptor::pushDescriptor(int fd){
	SocketDescriptorMap& fds = getDescriptorMap();
	if (fds.find(fd) == fds.end()){
		fds.insert(pair<int, SocketDescriptor*>(fd, new SocketDescriptor(fd)));
		return true;
	}return false;
}
bool ManageSocketDescriptor::pushDescriptor(int fd, string addr){
	SocketDescriptorMap& fds = getDescriptorMap();
	if (fds.find(fd) == fds.end()){
		fds.insert(pair<int, SocketDescriptor*>(fd, new SocketDescriptor(fd, addr)));
		return true;
	}return false;
}

string ManageSocketDescriptor::match(string addr){

	string result;
	regex re("\\b(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
	smatch mat;

	try{
		if(regex_search(addr, mat, re) && mat.size() > 1){
			result = mat.str(0);
		}
	}catch(exception &e){
		printf("Syntax error in the regular expression.");
		printf("[] %s\n", e.what());
	}
	return  result;
}

bool ManageSocketDescriptor::setIP(int fd, string addr){
	SocketDescriptorMap& fds = getDescriptorMap();
	if (fds.find(fd) != fds.end()){
		SocketDescriptor* descriptor = *(&fds.find(fd)->second);
		descriptor->setIP(match(addr));
		return true;
	}return false;
}

string ManageSocketDescriptor::getIP(int fd){
	SocketDescriptorMap& fds = getDescriptorMap();
	if (fds.find(fd) != fds.end()){
		SocketDescriptor* descriptor = *(&fds.find(fd)->second);
		return descriptor->getIP();
	}return "";
}

bool ManageSocketDescriptor::chkState(int fd){
	SocketDescriptorMap& fds = getDescriptorMap();
	if (fds.find(fd) != fds.end()){
		SocketDescriptor* descriptor = *(&fds.find(fd)->second);
		if(descriptor->getState() == "SIOCGIFCONF")
			return true;
	}return false;
}



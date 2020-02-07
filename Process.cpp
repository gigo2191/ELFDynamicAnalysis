#include "Process.h"
using namespace std;

Process::Process():pid_(0){}
Process::Process(int pid):pid_(pid),ppid_(0){}
Process::Process(int pid, int ppid):pid_(pid),ppid_(ppid){}

int Process::getPid(){
	return pid_;
}

int Process::getPpid(){
	return ppid_;
}

void Process::setPpid(int ppid){
	ppid_ = ppid;
}

vector<int>& Process::getChildren(){
	vector<int>& children = child_;
	return children;
}

void Process::addChild(int pid){
	vector<int>& children = getChildren();
	children.push_back(pid);
}

ProcessLog::ProcessLog(int pid){
	pid_ = pid;
	ppid_ = 0;
}

int ProcessLog::getPid(){
	return pid_;
}

vector<LOG_ENTRY>& ProcessLog::getAllLog(){
	vector<LOG_ENTRY>& log = log_;
	return log;
}

vector<CATEGORIZED_LOG>& ProcessLog::getCategorizedLog(string category){
	vector<CATEGORIZED_LOG>& categorizedLog = categorizedLog_;
	return categorizedLog_;
}


ManageFileDescriptor& ProcessLog::getFileDescriptorMap(){
	ManageFileDescriptor& fds =  fds_;
	return fds;
}

ManageSocketDescriptor& ProcessLog::getSocketDescriptorMap(){
	ManageSocketDescriptor& sds =  sds;
	return sds;
}

CATEGORIZED_LOG ProcessLog::processRawLog(LOG_ENTRY entry){
	CATEGORIZED_LOG result;
	result.pid = entry.pid;
	result.timestamp = entry.timestamp;
	result.syscall = entry.syscall;
	result.category = entry.category;
	ManageFileDescriptor& fds = getFileDescriptorMap();
	ManageSocketDescriptor& sds = getSocketDescriptorMap();

	if (result.category == "FileSystem"){
		if (result.syscall == "open"){
			result.arguments["filepath"] = entry.args.at(0);
			result.arguments["fd"] = entry.result;
			if(atoi(result.arguments["fd"].c_str()) != -1)
				fds.pushDescriptor(atoi(result.arguments["fd"].c_str()), result.arguments["filepath"]);
		}else if(result.syscall == "openat"){
			result.arguments["filepath"] = entry.args.at(1);
			result.arguments["fd"] = entry.result;
			result.arguments["mode"] = entry.args.at(2);
			if(atoi(result.arguments["fd"].c_str()) != -1)
				fds.pushDescriptor(atoi(result.arguments["fd"].c_str()), result.arguments["filepath"]);
		}else if (result.syscall == "read" || result.syscall == "write" || result.syscall == "close"){
			result.arguments["fd"] = entry.args.at(0);
			result.arguments["filepath"] = fds.getName(atoi(entry.args.at(0).c_str()));
			if (result.syscall == "close"){
				fds.popDescriptor(atoi(entry.args.at(0).c_str()));
			}else{
				result.arguments["buf"] = entry.args.at(1);
				result.arguments["count"] = entry.args.at(2);
			}
		}else if (result.syscall == "fstat" || result.syscall == "fstat64"){
			result.arguments["fd"] = entry.args.at(0);
			result.arguments["filepath"] = fds.getName(atoi(entry.args.at(0).c_str()));
			size_t pos = entry.args.at(1).find("st_mode=", 0) + 8;
			size_t end = entry.args.at(1).find("|");
			string stat = entry.args.at(1).substr(pos, end-pos);
			fds.setState(atoi(result.arguments["fd"].c_str()), stat);
		}else if (result.syscall == "rename"){
			result.arguments["oldpath"] = entry.args.at(0);
			result.arguments["newpath"] = entry.args.at(1);
		}else if (result.syscall == "renameat"){
			result.arguments["oldpath"] = entry.args.at(2);
			result.arguments["newpath"] = entry.args.at(3);
			result.arguments["oldfd"] = entry.args.at(0);
			result.arguments["newfd"] = entry.args.at(1);
		}else if (result.syscall == "chmod" || result.syscall == "access"){
			result.arguments["filepath"] = entry.args.at(0);
			result.arguments["mode"] = entry.args.at(1);
		}else if (result.syscall == "mkdir"){
			result.arguments["dirname"] = entry.args.at(0);
		}else if (result.syscall == "remove" || result.syscall == "unlink"){
			result.arguments["filepath"] = entry.args.at(0);
			result.arguments["result"] = entry.result;			
		}
	}else if (result.category == "Process"){
		if (result.syscall == "fork" || result.syscall == "clone"){
			result.arguments["pid"] = entry.result;
		}else if (result.syscall == "kill"){
			result.arguments["pid"] = entry.args.at(0);
			result.arguments["sig"] = entry.args.at(1);
		}
	}else if (result.category == "Network"){
		if (result.syscall == "socket"){
			result.arguments["domain"] = entry.args.at(0);
			result.arguments["type"] = entry.args.at(1);
			result.arguments["protocol"] = entry.args.at(2);
			result.arguments["fd"] = entry.result;
			sds.pushDescriptor(atoi(result.arguments["fd"].c_str()));
		}else if (result.syscall == "send" || result.syscall == "recv"){
			result.arguments["fd"] = entry.args.at(0);
			result.arguments["buf"] = entry.args.at(1);
			result.arguments["len"] = entry.args.at(2);
			result.arguments["ip"] = sds.getIP(atoi(entry.args.at(0).c_str()));
		}else if (result.syscall == "sendto" || result.syscall == "recvfrom"){
			result.arguments["fd"] = entry.args.at(0);
			result.arguments["buf"] = entry.args.at(1);
			result.arguments["len"] = entry.args.at(2);
			result.arguments["sockaddr"] = entry.args.at(4);
		}else if (result.syscall == "connect"){
			result.arguments["fd"] = entry.args.at(0);
			result.arguments["sockaddr"] = entry.args.at(1);
			sds.setIP(atoi(entry.args.at(0).c_str()), result.arguments["sockaddr"]);
		}else if (result.syscall == "accept"){
			result.arguments["fd"] = entry.args.at(0);
			result.arguments["sockaddr"] = entry.args.at(1);
			result.arguments["new_fd"] = entry.result;
			sds.pushDescriptor(atoi(result.arguments["new_fd"].c_str()), result.arguments["sockaddr"]);
		}else if (result.syscall == "ioctl"){
			int fd = atoi(entry.args.at(0).c_str());
			string stat = entry.args.at(1);
			sds.setState(fd, stat);
		}else if (result.syscall == "bind" || result.syscall == "listen"){
			result.arguments["fd"] = entry.args.at(0);
			result.arguments["sockaddr"] = entry.args.at(1);
		}else if (result.syscall == "listen"){
			result.arguments["fd"] = entry.args.at(0);
		}
	}
	string o = "";
	/*cout << result.pid << "\t" << result.timestamp << "\t" << result.syscall << "\t" << result.category << "\t";*/
	for(map<string,string>::iterator it = result.arguments.begin(); it != result.arguments.end(); it++){
		o += it->first;
		o += ":";
		o += it->second;
		o += "\t";
	}o += "\n";
	//printf("%s", o.c_str());
	return result;
}

ManageProcess::ManageProcess(string input): input_(input), output_(""){}
ManageProcess::ManageProcess(string input, string output): input_(input), output_(output){}

vector<ProcessLog> ManageProcess::getProcessList(){
	return processList_;
}

ProcessLog* ManageProcess::getProcess(int pid){
	for(vector<ProcessLog>::iterator it = processList_.begin(); it != processList_.end(); ++it){
		if(it->getPid() == pid){
			return &(*it);
		}
	}return NULL;
}

ProcessLog* ManageProcess::addProcess(int pid){
	if (getProcess(pid) == nullptr){
		if (processList_.size() > 500){
			return nullptr;
		}

		ProcessLog p = ProcessLog(pid);
		p.getFileDescriptorMap().pushStandard();
		processList_.push_back(p);
		return getProcess(pid);
	}return getProcess(pid);
}

bool ManageProcess::setPpid(int pid, int ppid){
	for(vector<ProcessLog>::iterator it = processList_.begin(); it != processList_.end(); ++it){
		if(it->getPid() == pid){
			it->setPpid(ppid);
			return true;
		}
	}return false;
}

void ManageProcess::process(){

	StraceInputStream stream = StraceInputStream(input_);
	do{
		try{
			StraceEntry entry = stream.nextEntry();
			ProcessLog* process = addProcess(entry.getPid());
			if (process == nullptr){
				throw runtime_error("Process List Size Limit.");
			}
			DATA_ENTRY data;
			data.pid = entry.getPid();
			data.timestamp = entry.getTimestamp();
			data.syscall = entry.getSystemCall();
			data.category = entry.getCategory();
			data.argc = entry.getArguments().size();
			data.result = entry.getReturnValue();
			process->addLog(data, entry.getArguments());
			
			if (entry.getSystemCall() == "fork" || entry.getSystemCall() == "clone"){
				if(atoi(entry.getReturnValue().c_str()) != 0){
					process->addChild(atoi(entry.getReturnValue().c_str()));
					ProcessLog* newProcess = addProcess(atoi(entry.getReturnValue().c_str()));
					if (newProcess == nullptr){
						throw runtime_error("Process List Size Limit.");
					}
					newProcess->setPpid(entry.getPid());
				}
			}

		}catch(exception &e){
			if (string(e.what()).find("vector::_M_range_check:") != string::npos){
				continue;			
			}
			if (string(e.what()).find("Process List Size Limit.") != string::npos){
				continue;			
			}
			//printf("[ERROR] %s\n", e.what());
			if (string(e.what()).find("Cannot read input file.", 0) != string::npos) {
				printf("end of file.\n");
				break;
			}
			if (string(e.what()).find("basic_string::_M_construct null not valid") != string::npos){
				break;			
			}
		}
	}while(true);


	// Reporting
	createProcessTree();

	return;
}

void ManageProcess::writeJson(){
	for(vector<ProcessLog>::iterator it = processList_.begin(); it != processList_.end(); ++it){
		JsonWriter jsonWriter(output_);
		vector<CATEGORIZED_LOG>& log = it->getCategorizedLog("");
		for (vector<CATEGORIZED_LOG>::iterator i = log.begin(); i != log.end(); i++){
			jsonWriter.writeEntry(*i);
		}jsonWriter.writeResultJsons(to_string(it->getPid()));
	}

	JsonWriter pstreeWriter;
	string pstreePath = output_ + "/pstree.json";
	pstreeWriter.writeJson(pstreePath, tree_);
	
}

void ManageProcess::createProcessTree(){
	for(vector<ProcessLog>::iterator it = processList_.begin(); it != processList_.end(); ++it){
		int pid = it->getPid();
		vector<int>& child = it->getChildren();
		string key = to_string(pid);
		Json::Value arr(Json::arrayValue);
		for (vector<int>::iterator i = child.begin(); i != child.end(); i++){
			arr.append(Json::Value(*i));
		}
		tree_[key] = arr;
	}
}

void ProcessLog::addLog(DATA_ENTRY data, vector<string> arguments){
	LOG_ENTRY entry;
	CATEGORIZED_LOG catagorized;
	int argc = 0;
	entry.timestamp = data.timestamp;
	entry.result = data.result;
	entry.args = arguments;
	entry.pid = data.pid;
	entry.argc = arguments.size();
	entry.syscall = data.syscall;
	entry.category = data.category;
	if (entry.category != ""){
		catagorized = processRawLog(entry);
		categorizedLog_.push_back(catagorized);
	}
}



int main(int argc, char *argv[]){

	if (argc != 4){
		printf("Usage: %s [targetRawPath] [targetPcapPath] [resultDirPath]\n", argv[0]);
		return 0;
	}

	struct stat dirStat;
	if((stat(argv[3], &dirStat) != 0) || (((dirStat.st_mode) & S_IFMT) != S_IFDIR)){
		if(mkdir(argv[3], S_IRWXU|S_IXGRP|S_IROTH|S_IXOTH) == -1){
			printf("Cannot create resultDirPath.\n");
			return 0;
		}
	}

	if(!isExists(argv[1])){
		printf("RawFile is not exist.\n");
	}else{
		ManageProcess mp = ManageProcess(argv[1], argv[3]);
		mp.process();
		mp.writeJson();
	}

	if(!isExists(argv[2])){
		printf("PcapFile is not exist.\n");
	}else{
		PcapParser p = PcapParser(argv[2], argv[3]);
		p.process();
		p.writeJson();
	}

	return 0;

}

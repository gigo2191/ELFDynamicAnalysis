
#pragma once
#include "Descriptor.h"
#include "Strace.h"
#include "Utils.h"
#include "PcapParser.h"
#include <json/json.h>
#include <json/writer.h>
#include <json/value.h>
#include <sys/stat.h>
#include <sys/types.h>

class Process;
class ProcessLog;
class ManageProcess;

class Process{
	public:
		Process();
		Process(int pid);
		Process(int pid, int ppid);
		int getPid();
		int getPpid();
		void setPpid(int ppid);
		void addChild(int pid);
		std::vector<int>& getChildren();
	private:
		int pid_;
		int ppid_;
		std::vector<int> child_;
};

class ProcessLog : public Process{
	public:
		ProcessLog(int pid);
		int getPid();
		void addLog(DATA_ENTRY data, std::vector<std::string> arguments);
		std::vector<CATEGORIZED_LOG>& getCategorizedLog(std::string category);
		std::vector<LOG_ENTRY>& getAllLog();
		ManageFileDescriptor& getFileDescriptorMap();
		ManageSocketDescriptor& getSocketDescriptorMap();
		CATEGORIZED_LOG processRawLog(LOG_ENTRY entry);

	private:
		int pid_;
		int ppid_;
		std::vector<LOG_ENTRY> log_;
		std::vector<CATEGORIZED_LOG> categorizedLog_;
		ManageFileDescriptor fds_;
		ManageSocketDescriptor sds_;
};

class ManageProcess{
	public:
		ManageProcess(std::string input, std::string output);
		ManageProcess(std::string input);
		std::vector<ProcessLog> getProcessList();
		ProcessLog* getProcess(int pid);
		ProcessLog* addProcess(int pid);
		bool setPpid(int pid, int ppid);
		void createProcessTree();
		void process();
		void writeJson();

	private:
		std::vector<ProcessLog> processList_;
		std::string input_;
		std::string output_;
		Json::Value tree_;
};

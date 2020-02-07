#pragma once

#include <string>
#include <stdio.h>
#include <ctype.h>
#include <fstream>
#include <regex>
#include <vector>
#include "Utils.h"

enum filesystemCalls{open = 0,READ, WRITE, CLOSE, FSTAT, FSTAT64, RENAME, MKDIR, CHMOD, ACCESS, MKDIR2, OPENAT, UNLINK, REMOVE, NUM_FILESYSTEM_CALL};

//const constexpr std::string kFilesystemCallTable[NUM_FILESYSTEM_CALL] = {"open", "read", "write", "close", "fstat", "fstat64", "rename", "mkdir", "chmod", "access", "mkdir"};

enum processCalls{FORK = 0, KILL, CLONE, NUM_PROCESS_CALL};

//const constexpr char* kProcessCallTable[NUM_PROCESS_CALL] = {"fork", "kill"};

enum networkCalls{SOCKET = 0, SEND, SENDTO, RECV, RECVFROM, CONNECT, IOCTL, ACCEPT, BIND, LISTEN, GETSOCKNAME, NUM_NETWORK_CALL};

//const constexpr char* kNetworkCallTable[NUM_NETWORK_CALL] = {"socket", "send", "sendto", "recv", "recvfrom", "connect", "ioctl", "accept", "bind", "listen", "getsockname"};

class StraceEntry;
class StraceInputStream;

class StraceEntry{
	public:
		static const std::string FILESYSTEM[NUM_FILESYSTEM_CALL];
		static const std::string PROCESS[NUM_PROCESS_CALL];
		static const std::string NETWORK[NUM_NETWORK_CALL];
		StraceEntry(int pid, int timestamp, bool unfinished, std::string elapsedTime, std::string systemCall, std::vector<std::string> arguments, std::string returnValue, int argumentsCount);
		int getPid();
		int getTimestamp();
		bool isUnfinished();
		std::string getElapsedTime();
		std::string getSystemCall();
		std::vector<std::string> getArguments();
		int getArgumentsCount();
		std::string getReturnValue();
		std::string getCategory();
	private:
		int pid_;
		int timestamp_;
		bool unfinished_;
		std::string elapsedTime_;
		std::string systemCall_;
		std::vector<std::string> arguments_;
		std::string returnValue_;
		std::string category_;	
		int argumentsCount_;
};

class StraceInputStream{
	public:
		StraceInputStream(std::string input);
		std::vector<std::string> parseArguments(std::string argumentsString);
		StraceEntry nextEntry();
		std::vector<std::string> match(std::string data, std::string, int count);
		static const std::string rePid;
		static const std::string reExtract;
		static const std::string reExtractUnfinished;
		static const std::string reExtractNoElapsed;
		static const std::string reExtractResume;
		static const std::string reExtractSignal;
		static const std::string reExtractExit;
		static const std::string reExtractKill;
		static const std::string reExtractArgumentsAndReturnValueNone;
		static const std::string reExtractArgumentsAndReturnValueOk;
		static const std::string reExtractArgumentsAndReturnValueOkHex;
		static const std::string reExtractArgumentsAndReturnValueError;
		static const std::string reExtractArgumentsAndReturnValueErrorUnknown;
		static const std::string reExtractArgumentsAndReturnValueExt;
		static const std::string reExtractArgumentsAndReturnValueExtHex;
		void close();
	private:
		std::ifstream inputStream_;
		std::string inputFileName_;
		int lineNumber_;
};


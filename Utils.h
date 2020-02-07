
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <json/json.h>
#include <json/writer.h>
#include <json/value.h>
#pragma once

typedef struct LOG_ENTRY_{
	int pid;
	int timestamp;
	int argc;
	std::string result;
	std::vector<std::string> args;
	std::string syscall;
	std::string category;
}LOG_ENTRY, *PLOG_ENTRY;

typedef struct DATA_ENTRY_{
	int pid;
	int timestamp;
	std::string syscall;
	std::string category;
	bool split;
	int argc;
	std::string result;
	std::string elapsed;
}DATA_ENTRY, *PDATA_ENTRY;

typedef struct CATEGORIZED_LOG_{
	int pid;
	int timestamp;
	std::string syscall;
	std::string category;
	std::map<std::string, std::string> arguments;
}CATEGORIZED_LOG, *PCATEGORIZED_LOG;

std::string findIP(std::string addr);

std::string findState(std::string argument);

bool isExists(std::string filename);

bool HasEnding(std::string const &fullString, std::string const &ending);

bool HasStart(std::string const &fullString, std::string const &start);

bool HasString(std::string const &fullString, std::string const &substring);

bool WriteJson();

class JsonWriter;

class JsonWriter{
	public:
		JsonWriter();
		JsonWriter(std::string resultPath);
		bool writeEntry(CATEGORIZED_LOG log);
		bool writeRawLog(LOG_ENTRY log);
		//bool writeArguments(std::vector<std::string> args);
		bool writeResultJsons(std::string name);
		bool writeJson(std::string name, Json::Value data);
	private:
		std::string categoryFilePath_;
		std::string rawFilePath_;
		std::string resultPath_;
		Json::Value result_;
};

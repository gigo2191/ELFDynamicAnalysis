#include "Utils.h"
using namespace std;
std::string findIP(std::string addr){
	std::regex reg("\\b(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
	std::smatch m;

	if (std::regex_match(addr, m, reg)){
		for( auto& sm : m)
			return sm;
	}
	return "";
}

std::string findState(std::string argument){
	int start = argument.find("st_mode=");
	int end = argument.find("|");
	if (start != std::string::npos && end != std::string::npos){
		return argument.substr(start, end-1);
	}
	return "";
}

bool isExists(string filename){
	ifstream ifile(filename.c_str());
	return (bool)ifile;
}

bool HasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

bool HasStart(string const &fullString, string const &start){
	if(fullString.length() >= start.length()){
		return (0 == fullString.find(start));
	} else {
		return false;
	}
}

bool HasString(string const &fullString, string const &substring){
	if(fullString.length() >= substring.length()){
		return (string::npos != fullString.find(substring));
	} else {
		return false;
	}
}

JsonWriter::JsonWriter(){
	resultPath_ = "";
	categoryFilePath_ = "_category.json";
	rawFilePath_ = "all.json";
	result_ = Json::Value(Json::arrayValue);
}

JsonWriter::JsonWriter(string resultPath){
	resultPath_ = resultPath;
	categoryFilePath_ = "_category.json";
	rawFilePath_ = "all.json";
	result_ = Json::Value(Json::arrayValue);
}

bool JsonWriter::writeEntry(CATEGORIZED_LOG log){
	Json::Value entry;
	Json::Value args;
	entry["pid"] = log.pid;
	entry["timestamp"] = log.timestamp;
	entry["category"] = log.category;
	entry["syscall"] = log.syscall;
	for (map<string,string>::iterator it = log.arguments.begin(); it != log.arguments.end(); it++){
		args[it->first] = it->second;
	}
	entry["arguments"] = args;
	result_.append(entry);
}

bool JsonWriter::writeRawLog(LOG_ENTRY log){}

bool JsonWriter::writeResultJsons(string name){
	try{
		Json::StreamWriterBuilder builder;
		ofstream outputFileStream(resultPath_ + "/" + name + categoryFilePath_);
		
		builder["commentStyle"] = "None";
		builder["indentation"] = "";
		
		unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
		writer->write(result_, &outputFileStream);
	}catch(exception &e){
		printf("[JsonWriter Error] %s\n", e.what());
		return false;
	}
	return true;
}

bool JsonWriter::writeJson(string name, Json::Value data){
	try{
		Json::StreamWriterBuilder builder;
		ofstream outputFileStream(name);
		
		builder["commentStyle"] = "None";
		builder["indentation"] = "";
		
		unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
		writer->write(data, &outputFileStream);
	}catch(exception &e){
		printf("[ProcessTreeWriter Error] %s\n", e.what());
		return false;
	}
	return true;
}


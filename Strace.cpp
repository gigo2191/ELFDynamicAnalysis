#include "Strace.h"

#include <stdexcept>
using namespace std;

const std::string StraceEntry::FILESYSTEM[] = {"open", "read", "write", "close", "fstat", "fstat64", "rename", "mkdir", "chmod", "access", "mkdir", "unlink", "remove", "openat"};
const std::string StraceEntry::PROCESS[] = {"fork", "kill", "clone"};
const std::string StraceEntry::NETWORK[] = {"socket", "send", "sendto", "recv", "recvfrom", "connect", "ioctl", "accept", "bind", "listen", "getsockname"};

const string StraceInputStream::rePid = "(\\d+) .*";
const string StraceInputStream::reExtract = "\\s*(\\d+\\.\\d+) (\\w+)(\\(.*) <(.+)>$";
const string StraceInputStream::reExtractUnfinished = "\\s*(\\d+\\.\\d+ .*) <unfinished \\.\\.\\.>$";
const string StraceInputStream::reExtractNoElapsed = "\\s*(\\d+\\.\\d+) (\\w+)(\\(.*)$";
const string StraceInputStream::reExtractResume = "\\s*(\\d+\\.\\d+) <\\.\\.\\. [\\a-zA-Z\\d]+ resumed>(.*)$";
const string StraceInputStream::reExtractSignal = "\\s*(\\d+\\.\\d+) --- (\\w+) \\{(.)*\\} ---$";
const string StraceInputStream::reExtractExit = "\\s*(\\d+\\.\\d+) \\+\\+\\+ exited with (-?[\\d]+) \\+\\+\\+$";
const string StraceInputStream::reExtractKill = "\\s*(\\d+\\.\\d+) \\+\\+\\+ killed by ([\\w]+) \\+\\+\\+$";
const string StraceInputStream::reExtractArgumentsAndReturnValueNone = "\\((.*)\\)[ \\t]*= (\\?)$";
const string StraceInputStream::reExtractArgumentsAndReturnValueOk = "\\((.*)\\)[ \\t]*= (-?\\d+)$";
const string StraceInputStream::reExtractArgumentsAndReturnValueOkHex = "\\((.*)\\)[ \\t]*= (-?0[xX][a-fA-F\\d]+)$";
const string StraceInputStream::reExtractArgumentsAndReturnValueError = "\\((.*)\\)[ \\t]*= (-?\\d+) (\\w+) \\([\\w ]+\\)$";
const string StraceInputStream::reExtractArgumentsAndReturnValueErrorUnknown = "\\((.*)\\)[ \\t]*= (\\?) (\\w+) \\([\\w ]+\\)$";
const string StraceInputStream::reExtractArgumentsAndReturnValueExt = "\\((.*)\\)[ \\t]*= (-?\\d+) \\(([^()]+)\\)$";
const string StraceInputStream::reExtractArgumentsAndReturnValueExtHex = "\\((.*)\\)[ \\t]*= (-?0[xX][a-fA-F\\d]+) \\(([^()]+)\\)$";

StraceEntry::StraceEntry(int pid, int timestamp, bool unfinished, string elapsedTime, string systemCall, vector<string> arguments, string returnValue, int argumentsCount):pid_(pid),timestamp_(timestamp),unfinished_(unfinished), elapsedTime_(elapsedTime), systemCall_(systemCall), arguments_(arguments), returnValue_(returnValue), argumentsCount_(argumentsCount), category_(""){};

int StraceEntry::getPid(){
	return pid_;
}

int StraceEntry::getTimestamp(){
	return timestamp_;
}

bool StraceEntry::isUnfinished(){
	return unfinished_;
}

string StraceEntry::getElapsedTime(){
	return elapsedTime_;
}

string StraceEntry::getSystemCall(){
	return systemCall_;
}

vector<string> StraceEntry::getArguments(){
	return arguments_;
}

int StraceEntry::getArgumentsCount(){
	return argumentsCount_;
}

string StraceEntry::getReturnValue(){
	return returnValue_;
}

string StraceEntry::getCategory(){
	for (int i = 0; i < NUM_FILESYSTEM_CALL; i++){
		if (FILESYSTEM[i] == systemCall_) {
			return "FileSystem";
		}
	}
	for (int i = 0; i < NUM_PROCESS_CALL; i++){
                if (PROCESS[i] == systemCall_) {
                        return "Process";
                }
        }
	for (int i = 0; i < NUM_NETWORK_CALL; i++){
                if (NETWORK[i] == systemCall_) {
                        return "Network";
                }
        }
	return "";
}
StraceInputStream::StraceInputStream(string input): inputFileName_(input), inputStream_(input), lineNumber_(0) {}

std::vector<std::string> StraceInputStream::parseArguments(string argumentsString){
	bool betweenArguments = false;
	bool expectComma = false;
	char quote = 0x0;
	bool escaped = false;
	string currentArgument = "";
	vector<string> arguments;
	stack<char> nest;

	for (char& c : argumentsString) {
		if (betweenArguments) {
			if (c == ' ' || c == '\t'){
				continue;
			}else{
			betweenArguments = false;
			}
		}
		if (expectComma) {
			if (quote != 0x0) {
				throw invalid_argument("Exception in StraceInpuStream::parseArguments");
			}
			if (c == '.') {
				currentArgument += c;
			}else if (c == ',') {
				expectComma = false;
				betweenArguments = true;
				arguments.push_back(currentArgument);
				currentArgument = "";
			}else if (c == ' ' || c == '\t') {
				continue;
			}else{
				printf("C: %s\n", currentArgument.c_str());
				throw invalid_argument("Exception in StraceInpuStream::parseArguments");
			}
			continue;
		}
		if (escaped) {
			currentArgument += c;
			escaped = false;
		}else if (c == '\\') {
			currentArgument += c;
			escaped = true;
		}else if (c == '\"' || c == '\'' || c == '[' || c == ']' || c == '{' || c == '}') {
			if ((quote == '\"' || quote == '\'') && c != quote) {
				currentArgument += c;
			}else if (c == quote) {
				if (nest.size() > 1){
					nest.pop();
					quote = nest.top();
				}else{
					nest.pop();
					quote = 0x0;
					if (currentArgument == "[?]") {
						expectComma = true;
					}
				}
			}else if (c == ']' || c == '}') {
				currentArgument += c;
			}else{
				if (nest.size() > 0) {
					currentArgument += c;
				}
				if (c == '[') {
					c = ']';
				}
				if (c == '{') {
					c = '}';
				}
				quote = c;
				nest.push(c);
			}
			
		}else if (c == ',' && quote == 0x0) {
			arguments.push_back(currentArgument);
			currentArgument = "";
			betweenArguments = true;
		}else{
			currentArgument += c;
		}
	}
	if (quote != 0x0) {
		throw invalid_argument("Exception in StraceInpuStream::parseArguments");
	}
	if (currentArgument.size() > 0) {
		arguments.push_back(currentArgument);
	}
	return arguments;
}

StraceEntry StraceInputStream::nextEntry(){
	int start = 0;
	if (inputStream_.is_open() && !inputStream_.eof()){
		string line;
		if (!getline(inputStream_, line)) {
			throw invalid_argument("Exception in StraceInputStream::nextEntry. StopIteration.");
		}
		lineNumber_ += 1;
		start = 0;

		if (line == "") {
			if (lineNumber_ == 1){
				throw invalid_argument("Exception in StraceInputStream::nextEntry. The first line needs to be valid.");
			}else{
				return nextEntry();
			}
		}

		if (!isdigit(line[0])){
			if (lineNumber_ == 1){
				throw invalid_argument("Exception in StraceInputStream::nextEntry. The first line needs to be valid.");
                        }else{
                                return nextEntry();
                        }
                }

		// get the pid
		int pid = 0;
		string pidStr;

		vector<string> r = match(line.substr(start, line.length() - 1), rePid, 1);
		if (r.size() != 0){
			pidStr = r.at(1);
			pid = stoi(pidStr);
			start = pidStr.length() + 1;
		}
		// signals
		if (HasEnding(line, "----")) {
			vector<string> r = match(line.substr(start, line.length() - 1), reExtractSignal, 3);
			if (r.size() != 0){
				int timestamp = stoi(r.at(0));
				string signalName = r.at(1);
				vector<string> arguments = parseArguments(r.at(2));
				return StraceEntry(pid, timestamp, false, "0", signalName, arguments, "0", arguments.size());
			}
		}

		// Extrace basic information
		string argsAndResult = "";
		int timestamp = 0;
		string syscallName = "";
		string elapsedTime = "";
		string returnValue = "";
		string argumentsString = "";
		if (!HasEnding(line, "+++") && !HasEnding(line, "<unfinished ...>")) {
			string resumedStart = "<... ";
			string resumedString = " resumed> ";
			if (HasString(line, resumedStart) && HasString(line, resumedString)){
				line.replace(line.find(resumedStart), resumedStart.length(), "");
				line.replace(line.find(resumedString), resumedString.length(), "(");
			}

			vector<string> r = match(line.substr(start, line.length() - 1), reExtract, 4);
			if (r.size() != 0) {
				timestamp = stoi(r.at(1));
				syscallName = r.at(2);
				argsAndResult = r.at(3);
				elapsedTime = r.at(4);
			}else{
				r = match(line.substr(start, line.length() - 1), reExtractNoElapsed, 3);
				if (r.size() != 0) {
					timestamp = stoi(r.at(1));
					syscallName = r.at(2);
					argsAndResult = r.at(3);
					elapsedTime = string();
				}else{
					throw invalid_argument("Exception in StraceInputStream::nextEntry. Invalid line");
				}
			}
			
			// Extract the return value
			vector<string> mArgsRet = match(argsAndResult, reExtractArgumentsAndReturnValueOk, 2);
			if (mArgsRet.size() != 0) {
				argumentsString = mArgsRet.at(1);
				returnValue = mArgsRet.at(2);
			}else{
				mArgsRet = match(argsAndResult, reExtractArgumentsAndReturnValueOkHex, 2);
				if (mArgsRet.size() != 0){
					returnValue = mArgsRet.at(2);
					argumentsString = mArgsRet.at(1);
				}
				mArgsRet = match(argsAndResult, reExtractArgumentsAndReturnValueError, 2);
				if (mArgsRet.size() != 0){
					argumentsString = mArgsRet.at(1);
					returnValue = mArgsRet.at(2);
				}
				mArgsRet = match(argsAndResult, reExtractArgumentsAndReturnValueErrorUnknown, 2);
				if (mArgsRet.size() != 0){
					argumentsString = mArgsRet.at(1);
					returnValue = mArgsRet.at(2);
				}
				mArgsRet = match(argsAndResult, reExtractArgumentsAndReturnValueExt, 2);
				if (mArgsRet.size() != 0){
					argumentsString = mArgsRet.at(1);
					returnValue = mArgsRet.at(2);
				}
				mArgsRet = match(argsAndResult, reExtractArgumentsAndReturnValueExtHex, 2);
				if (mArgsRet.size() != 0){
					argumentsString = mArgsRet.at(1);
					returnValue = mArgsRet.at(2);
				}
				mArgsRet = match(argsAndResult, reExtractArgumentsAndReturnValueNone, 1);
				if (mArgsRet.size() != 0){
					argumentsString = mArgsRet.at(1);
					returnValue = "";
				}
				if (mArgsRet.size() != 0){
					throw invalid_argument("Exception in StraceInputStream::nextEntry. Invalid line");
				} 
			}

			vector<string> arguments = parseArguments(argumentsString);
			
			return StraceEntry(pid, timestamp, false, elapsedTime, syscallName, arguments, returnValue, arguments.size());
			
		}else{
			return nextEntry();
		}


	}else{
		throw invalid_argument("Exception in StraceInputStream::nextEntry. Cannot read input file.");
	}
}

std::vector<std::string> StraceInputStream::match(string data, string regexString, int count){

	std::vector<std::string> result;
	regex re(regexString);
	smatch mat;
	try{
		if (regex_search(data, mat, re) && mat.size() > 1) {
			for (int i = 0; i < count+1; i++){
				result.push_back(mat.str(i));
			}
		}
	}catch(exception &e){
		printf("Syntax error in the regular expression.");
		printf("[] %s\n", e.what());
	}
	return  result;
}

void StraceInputStream::close(){
	inputStream_.close();
}

/*int main(){
	StraceInputStream stream = StraceInputStream("s.txt");
	do{
		try{
			StraceEntry entry = stream.nextEntry();
			cout << entry.getPid() << "\t" << entry.getSystemCall() << "\t" << entry.getReturnValue() << "\t";
			for(int i = 0; i < entry.getArguments().size(); i++)
				cout << entry.getArguments().at(i) << "\t"; 
			printf("\n");
		}catch(exception &e){
			printf("[ERROR] %s\n", e.what());
			if (string(e.what()).find("Cannot read input file.", 0) != string::npos) {
				printf("end of file.\n");
				break;
			}
		}
	}while(true);
	return 0;
}*/

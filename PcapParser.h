#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <pcap.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <pcap/pcap.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "Utils.h"

class PcapParser;

class PcapParser{
	public:
		PcapParser(std::string path, std::string result);
		void handler(const struct pcap_pkthdr* pkthdr, const char* packet);
		bool process();
		void writeJson();

	private:
		std::string path_;
		Json::Value result_;
		std::string resultPath_;
};

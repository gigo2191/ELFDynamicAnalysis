#include "PcapParser.h"

using namespace std;

PcapParser::PcapParser(string path, string result):path_(path),resultPath_(result){}

void PcapParser::handler(const struct pcap_pkthdr* pkthdr, const char* packet){
	const struct ether_header* ethHeader;
	const struct ip* ipHeader;
	const struct tcphdr* tcpHeader;
	const struct udphdr* udpHeader;
	char src[INET_ADDRSTRLEN];
	char dst[INET_ADDRSTRLEN];
	u_int srcPort, dstPort;
	u_char *data;
	int dataLength = 0;
	string dataStr = "";
	Json::Value entry;
	ethHeader = (struct ether_header*)packet;
	if (ntohs(ethHeader->ether_type) == ETHERTYPE_IP){
		ipHeader = (struct ip*)(packet + sizeof(struct ether_header));
		inet_ntop(AF_INET, &(ipHeader->ip_src), src, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &(ipHeader->ip_dst), dst, INET_ADDRSTRLEN);
		
		if(ipHeader->ip_p == IPPROTO_TCP || ipHeader->ip_p == IPPROTO_IP){
			tcpHeader = (tcphdr*)(packet + sizeof(struct ether_header) + sizeof(struct ip));
			srcPort = ntohs(tcpHeader->source);
			dstPort = ntohs(tcpHeader->dest);
			data = (u_char*)(packet + sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct tcphdr));
			dataLength = pkthdr->len - (sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct tcphdr));

			for (int i = 0; i < dataLength; i++){
				if((data[i] >= 32 && data[i] <= 126) || data[i] == 10 || data[i] == 11 || data[i] == 13){
					dataStr += (char)data[i];
				}else{
					dataStr += ".";
				}
			}

			entry["timestamp"] = (double)pkthdr->ts.tv_sec;
			entry["protocol"] = "TCP";
			entry["src"] = src;
			entry["dst"] = dst;
			entry["src_port"] = srcPort;
			entry["dst_port"] = dstPort;
			entry["data"] = dataStr;
			if (srcPort == 80 || dstPort == 80){
				entry["protocol"] = "HTTP";
			}else if(srcPort == 20 || dstPort == 20){				
				entry["protocol"] = "FTP";
			}else if(srcPort == 21 || dstPort == 21){				
				entry["protocol"] = "FTP";
			}else if(srcPort == 25 || dstPort == 25){				
				entry["protocol"] = "SMTP";
			}
			result_.append(entry);
			
		}else if(ipHeader->ip_p == IPPROTO_UDP){
			udpHeader = (udphdr*)(packet + sizeof(struct ether_header) + sizeof(struct ip));
			srcPort = ntohs(udpHeader->source);
			dstPort = ntohs(udpHeader->dest);
			data = (u_char*)(packet + sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct tcphdr));
			dataLength = pkthdr->len - (sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct tcphdr));

			for (int i = 0; i < dataLength; i++){
				if((data[i] >= 32 && data[i] <= 126) || data[i] == 10 || data[i] == 11 || data[i] == 13){
					dataStr += (char)data[i];
				}else{
					dataStr += ".";
				}
			}


			entry["timestamp"] = (double)pkthdr->ts.tv_sec;
			entry["protocol"] = "UDP";
			entry["src"] = src;
			entry["dst"] = dst;
			entry["src_port"] = srcPort;
			entry["dst_port"] = dstPort;
			entry["data"] = dataStr;
			if(srcPort == 53 || dstPort == 53){				
				entry["protocol"] = "DNS";
			}
			result_.append(entry);
		}else if(ipHeader->ip_p == IPPROTO_ICMP){
			udpHeader = (udphdr*)(packet + sizeof(struct ether_header) + sizeof(struct ip));
			srcPort = ntohs(udpHeader->source);
			dstPort = ntohs(udpHeader->dest);
			data = (u_char*)(packet + sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct tcphdr));
			dataLength = pkthdr->len - (sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct tcphdr));

			for (int i = 0; i < dataLength; i++){
				if((data[i] >= 32 && data[i] <= 126) || data[i] == 10 || data[i] == 11 || data[i] == 13){
					if (data[i] == 34 || data[i] == 92){
						dataStr += "\\";
					}
					dataStr += (char)data[i];
				}else{
					dataStr += ".";
				}
			}


			entry["timestamp"] = (double)pkthdr->ts.tv_sec;
			entry["protocol"] = "ICMP";
			entry["src"] = src;
			entry["dst"] = dst;
			entry["src_port"] = srcPort;
			entry["dst_port"] = dstPort;
			entry["data"] = dataStr;
			result_.append(entry);
		}
	}
}

bool PcapParser::process(){
	pcap_t *descr;
	char errbuf[PCAP_ERRBUF_SIZE];

	descr = pcap_open_offline(path_.c_str(), errbuf);
	if(descr == NULL){
		printf("pcap_open() failed: %s\n" , errbuf);
		return false;
	}

	char* ptr;
	struct pcap_pkthdr hdr;
	while((ptr = (char*)pcap_next(descr, &hdr)) != NULL){
		handler(&hdr, ptr);
	}

	printf("finished!\n");
	return true;
}

void PcapParser::writeJson(){
	JsonWriter writer;
	string path = resultPath_ + "/network.json";
	writer.writeJson(path, result_);
}


/*
 * TCP Traceroute
 *
 * This program perform a TCP traceroute to some host.
 */
#include <iostream>
#include <string>
#include <crafter.h>

/* Collapse namespaces */
using namespace std;
using namespace Crafter;

int main() {

	/* Init the library */
	InitCrafter();

	/* Set the interface */
	string iface = "wlan0";

	/* ----------- Host and traceroute data --------- */

	/* This is the IP we want to scan */
	string scan_ip = "www.google.com";

	/*  Set the destination port for the TCP packets */
	int port_number = 80;

	/* Max hops, i.e. the max number of TTL on the packets */
	int max_hops = 30;

	/* --------- Common data to all packets ---------- */

	/* Create an IP header */
	IP ip_header;

	/* Set the Source and Destination IP address */
	ip_header.SetSourceIP(GetMyIP(iface));
	ip_header.SetDestinationIP(scan_ip);

	/* Create a TCP header */
	TCP tcp_header;

	tcp_header.SetDstPort(port_number);
	tcp_header.SetFlags(TCP::SYN);
	tcp_header.SetWindowsSize(5480);

    /* ---------------------------------------------- */

	/* Create a PacketContainer to hold all the TCP packets */
	PacketContainer tcp_packets;

	/* Create a packet for each TTL */
	for(int ttl = 1 ; ttl <= max_hops ; ttl++) {

		/* Set a random ID for the IP header */
		ip_header.SetIdentification(RNG16());

		/* Set the TTL */
		ip_header.SetTTL(ttl);

		/* Set a Random sequence number for TCP syn packet*/
		tcp_header.SetSeqNumber(RNG32());
		tcp_header.SetSrcPort(RNG16());

		/* Create a packet */
		Packet* packet = new Packet;

		/* Push each layer... */
		packet->PushLayer(ip_header);
		packet->PushLayer(tcp_header);

		/* Push the packet into the container */
		tcp_packets.push_back(packet);
	}

	cout << "[@] Sending the TCP packets. Wait..." << endl;
	PacketContainer* ret_packets = SendRecv(&tcp_packets,iface,32,2,2);
	cout << "[@] SendRecv function returns :-) " << endl;

	/* Now, check each answer */
	PacketContainer::iterator it_pck;

	int counter = 1;
	for(it_pck = ret_packets->begin() ; it_pck < ret_packets->end() ; it_pck++) {
		/* Check if the pointer is not NULL */
		Packet* reply_packet = (*it_pck);
		if(reply_packet) {

			/* Get the IP layer of the replied packet */
			IP* ip_layer = GetIP(*reply_packet);

			/* Get source IP of the packet */
			string src_ip = ip_layer->GetSourceIP();
			/* Print the Source IP */
			cout << "[" << counter << "] " << src_ip << endl;

			/* Check if the received packet was an ICMP message */
			ICMP* icmp_layer = GetICMP(*reply_packet);

			/* If isn't an ICMP packet, break the loop. You can do additional checks. (TCP flags, ICMP type, etc...)*/
			if(!icmp_layer) break;

		} else
			cout << "[" << counter << "] *.*.*.* " << endl;

		/* Increment counter (= TTL) */
		counter++;
	}

	/* Delete the container with the PINGS packets */
	for(it_pck = tcp_packets.begin() ; it_pck < tcp_packets.end() ; it_pck++)
		delete (*it_pck);

	/* Delete the container with the responses, if there is one (check the NULL pointer) */
	for(it_pck = ret_packets->begin() ; it_pck < ret_packets->end() ; it_pck++)
		if((*it_pck)) delete (*it_pck);

	/* Delete the container itself */
	delete ret_packets;

	/* Clean up library stuff */
	CleanCrafter();

	return 0;
}

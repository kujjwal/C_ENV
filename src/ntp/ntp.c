//
// Created by Ujjwal Krishnamurthi on 8/20/20.
//

#include "ntp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>

int setupServer() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // Create a UDP socket.
    if (sockfd < 0) error("ERROR opening socket");

    server = gethostbyname(hostname); // Convert URL to IP.
    if (server == NULL) error("ERROR, no such host");

    // Zero out the server address structure.
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    // Copy the server's IP address to the server address structure.
    bcopy((char*)server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);

    // Convert the port number integer to network big-endian style and save it to the server address structure.
    serv_addr.sin_port = htons(port);
    return sockfd;
}

void messageServer(int sockfd, ntp_packet packet) {
    int n;
    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("Error connecting");
    n = write(sockfd, (char*) &packet, sizeof(ntp_packet));
    if(n < 0) error("Error writing to socket");

    n = read(sockfd, (char*) &packet, sizeof(ntp_packet));
    if(n < 0) error("Error reading from socket");
}

void parseMessage(ntp_packet packet) {
    packet.txTm_s = ntohl(packet.txTm_s);
    packet.txTm_f = ntohl(packet.txTm_f);

    // Subtracting time from NTP epoch (1900) to Unix Epoch (1970)
    time_t txTm = (time_t) (packet.txTm_s - NTP_TIMESTAMP_DELTA);
    printf("Time: %s", ctime((const time_t*) &txTm));
}

int main(int argc, char** argv) {
    // Create and zero out the packet. All 48 bytes worth.
    ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    memset( &packet, 0, sizeof(ntp_packet) );
    // Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.
    *((char *) &packet + 0) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.

    // Startup
    int sockfd = setupServer();
    messageServer(sockfd, packet);
    parseMessage(packet);
    return 0;
}
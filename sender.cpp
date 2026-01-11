#include "protocol.h"
#include "utils.h"

#include <iostream>
#include <vector>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 4) {
        std::cerr << "usage: sender <receiver_ip> <port> <file_path>\n";
        return 1;
    }

    std::string receiver_ip = argv[1];
    int port = std::stoi(argv[2]);
    std::string filepath = argv[3];

    auto chunks = readFileChunks(filepath, CHUNK_SIZE);
    if (chunks.empty()) {
        std::cerr << "failed to read file\n";
        return 1;
    }

    uint32_t total_chunks = chunks.size();
    std::string filename = filepath.substr(filepath.find_last_of("/\\") + 1);

    std::vector<uint8_t> full;
    for (const auto &c : chunks)
        full.insert(full.end(), c.begin(), c.end());

    uint32_t file_crc = crc32(full);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    inet_pton(AF_INET, receiver_ip.c_str(), &dest.sin_addr);

    Packet hello = makeHelloPacket(filename, total_chunks, file_crc);
    auto hello_buf = serializePacket(hello);
    sendto(sockfd, hello_buf.data(), hello_buf.size(), 0,
           (sockaddr *)&dest, sizeof(dest));

    std::cout << "HELLO sent: chunks=" << total_chunks << std::endl;

    sleepMs(200);

    for (uint32_t i = 0; i < total_chunks; i++) {
        uint32_t crc = crc32(chunks[i]);
        Packet pkt = makeDataPacket(i, total_chunks, chunks[i], crc);
        auto buf = serializePacket(pkt);

        sendto(sockfd, buf.data(), buf.size(), 0,
               (sockaddr *)&dest, sizeof(dest));

        std::cout << "sent chunk " << i << std::endl;
        sleepMs(10);
    }

    Packet fin = makeFinPacket(file_crc);
    auto fin_buf = serializePacket(fin);
    sendto(sockfd, fin_buf.data(), fin_buf.size(), 0,
           (sockaddr *)&dest, sizeof(dest));

    std::cout << "FIN sent\n";

    close(sockfd);
    return 0;
}

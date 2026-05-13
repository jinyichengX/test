int sockfd0, sockfd1, sockfd2, sockfd3, sockfd4, sockfd5, sockfd6, sockfd7, sockfd8, sockfd9, sockfd10, sockfd11;

int main(int argc, char *argv[])
{
    sockfd0 = __socket(AF_INET, SOCK_DGRAM, 0);
    sockfd1 = __socket(AF_INET, SOCK_DGRAM, 0);
    sockfd2 = __socket(AF_INET, SOCK_DGRAM, 0);
    sockfd3 = __socket(AF_INET, SOCK_DGRAM, 0);
    sockfd4 = __socket(AF_INET, SOCK_DGRAM, 0);
    sockfd5 = __socket(AF_INET, SOCK_DGRAM, 0);
    // sockfd6 = __socket(AF_INET, SOCK_DGRAM, 0);
    // sockfd7 = __socket(AF_INET, SOCK_DGRAM, 0);
    // sockfd8 = __socket(AF_INET, SOCK_DGRAM, 0);
    sockfd9 = __socket(AF_INET, SOCK_DGRAM, 0);

    __close(sockfd4);
    __close(sockfd1);
    __close(sockfd9);

    sockfd1 = __socket(AF_INET, SOCK_DGRAM, 0);
    sockfd7 = __socket(AF_INET, SOCK_DGRAM, 0);
    sockfd8 = __socket(AF_INET, SOCK_DGRAM, 0);
    sockfd6 = __socket(AF_INET, SOCK_DGRAM, 0);
    sockfd10 = __socket(AF_INET, SOCK_DGRAM, 0);
    sockfd11 = __socket(AF_INET, SOCK_DGRAM, 0);
    sockfd4 = __socket(AF_INET, SOCK_DGRAM, 0);
    return 0;
}
#pragma once

#include <string>
#include <chrono>
#include "byte_tools.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <cstring>
#include <cerrno>
#include <stdexcept>



/*
 * Обертка над низкоуровневой структурой сокета.
 */
class TcpConnect {
public:
    TcpConnect(std::string ip, int port, std::chrono::milliseconds connectTimeout, std::chrono::milliseconds readTimeout)
        : ip_(ip), port_(port), connectTimeout_(connectTimeout), readTimeout_(readTimeout){
    }
    ~TcpConnect() {
        CloseConnection();
    }

    /*
     * Установить tcp соединение.
     * Если соединение занимает более `connectTimeout` времени, то прервать подключение и выбросить исключение.
     * Полезная информация:
     * - https://man7.org/linux/man-pages/man7/socket.7.html
     * - https://man7.org/linux/man-pages/man2/connect.2.html
     * - https://man7.org/linux/man-pages/man2/fcntl.2.html (чтобы включить неблокирующий режим работы операций)
     * - https://man7.org/linux/man-pages/man2/select.2.html
     * - https://man7.org/linux/man-pages/man2/setsockopt.2.html
     * - https://man7.org/linux/man-pages/man2/close.2.html
     * - https://man7.org/linux/man-pages/man3/errno.3.html
     * - https://man7.org/linux/man-pages/man3/strerror.3.html
     */
    
    void EstablishConnection() {
        sock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_ < 0) {
            throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
        }

        int flags = fcntl(sock_, F_GETFL, 0);
        if (fcntl(sock_, F_SETFL, flags | O_NONBLOCK) < 0) {
            close(sock_);
            throw std::runtime_error("fcntl(F_SETFL) failed");
        }
        if (flags < 0) {
            throw std::runtime_error("fcntl(F_GETFL) failed");
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port_);
        if (inet_pton(AF_INET, ip_.c_str(), &serverAddr.sin_addr) != 1) {
            close(sock_);
            throw std::runtime_error("Invalid IP address: " + ip_);
        }
        int result = connect(sock_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
        if (result < 0) {
            if (errno != EINPROGRESS) {
                close(sock_);
                throw std::runtime_error("Failed to connect: " + std::string(strerror(errno)));
            }
        }

        pollfd pfd{sock_, POLLOUT, 0};
        int pollResult = poll(&pfd, 1, static_cast<int>(connectTimeout_.count()));
        if (pollResult < 0) {
            close(sock_);
            throw std::runtime_error("poll() failed: " + std::string(strerror(errno)));
        } else if (pollResult == 0) {
            close(sock_);
            throw std::runtime_error("Connection timed out");
        }

        int soError;
        socklen_t len = sizeof(soError);
        if (getsockopt(sock_, SOL_SOCKET, SO_ERROR, &soError, &len) < 0) {
            close(sock_);
            throw std::runtime_error(std::string("getsockopt() failed: ") + strerror(errno));
        }
        if (soError != 0) {
            close(sock_);
            throw std::runtime_error(std::string("connect() error after poll: ") + strerror(soError));
        }

        if (fcntl(sock_, F_SETFL, flags) < 0) {
            close(sock_);
            throw std::runtime_error(std::string("fcntl(F_SETFL restore) failed: ") + strerror(errno));
        }
    }

    /*
     * Послать данные в сокет
     * Полезная информация:
     * - https://man7.org/linux/man-pages/man2/send.2.html
     */
    void SendData(const std::string& data) const {
        if (sock_ == -1) {
            throw std::runtime_error("Socket is not connected");
        }
        ssize_t bytes_sent = send(sock_, data.c_str(), data.size(), 0);
        if (bytes_sent < 0) {
            throw std::runtime_error("Failed to send data: " + std::string(strerror(errno)));
        }
    }

    /*
     * Прочитать данные из сокета.
     * Если передан `bufferSize`, то прочитать `bufferSize` байт.
     * Если параметр `bufferSize` не передан, то сначала прочитать 4 байта, а затем прочитать количество байт, равное
     * прочитанному значению.
     * Первые 4 байта (в которых хранится длина сообщения) интерпретируются как целое число в формате big endian,
     * см https://wiki.theory.org/BitTorrentSpecification#Data_Types
     * Полезная информация:
     * - https://man7.org/linux/man-pages/man2/poll.2.html
     * - https://man7.org/linux/man-pages/man2/recv.2.html
     */

    void ReadAll(char* buf, size_t count) const {
        size_t recieved = 0;
            while (recieved < count) {
                ssize_t n = recv(sock_, buf + recieved, count - recieved, 0);
                if (n > 0) {
                    recieved += static_cast<size_t>(n);
                } else if (n == 0) {
                    throw std::runtime_error("Connection closed by peer");
                } else {
                    if (errno == EINTR) {
                        continue;
                    }
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        pollfd pfd{sock_, POLLIN, 0};
                        int result = poll(&pfd, 1, static_cast<int>(readTimeout_.count()));
                        if (result < 0) {
                            throw std::runtime_error("poll() failed: " + std::string(strerror(errno)));
                        } else if (result == 0) {
                            throw std::runtime_error("Read timed out");
                        }
                        continue;

                    }
                    throw std::runtime_error("recv() failed: " + std::string(strerror(errno)));
                }
            }
    }

    std::string ReceiveData(size_t bufferSize = 0) const {
        if (sock_ == -1) {
            throw std::runtime_error("Socket is not connected");
        }
        if (bufferSize == 0) {
            char lengthBuffer[4];
            ReadAll(lengthBuffer, sizeof(lengthBuffer));
            uint32_t len = BytesToInt({lengthBuffer, 4});
            std::string data(len, '\0');
            ReadAll(data.data(), len);
            return data;
        }
        std::string data(bufferSize, '\0');
        ReadAll(data.data(), bufferSize);
        return data;

    }
    /*
     * Закрыть сокет
     */
    void CloseConnection() {
        if (sock_ != -1) {
            close(sock_);
            sock_ = -1;
        }
    }

    const std::string& GetIp() const;
    int GetPort() const;

private:
    const std::string ip_;
    const int port_;
    std::chrono::milliseconds connectTimeout_, readTimeout_;
    int sock_;
};

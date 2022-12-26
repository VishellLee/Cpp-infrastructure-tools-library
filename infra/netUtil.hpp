#pragma once
#include <string>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/socket.h>

namespace Infra
{
    namespace Net
    {
        enum Protocol : int
        {
            TCP,
            UDP,
        };

        enum IPVersion : int
        {
            IPV4,
            IPV6,
            Unix,
        };

        inline int getSocketFd(Protocol protocol, IPVersion ipv)
        {
            int domain = -1, type = -1;

            switch (protocol)
            {
            case Protocol::TCP:
                type = SOCK_STREAM;
                break;

            case Protocol::UDP:
                type = SOCK_DGRAM;
                break;
            }

            switch (ipv)
            {
            case IPVersion::IPV4:
                domain = AF_INET;
                break;

            case IPVersion::IPV6:
                domain = AF_INET6;
                break;

            case IPVersion::Unix:
                domain = AF_UNIX;
                break;
            }

            return socket(domain, type, 0);
        }

        inline int reuseAddr(int socketFd)
        {
            int on = 1;
            return setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
        }

        inline in_addr_t toNetIP(std::string const &ip)
        {
            return inet_addr(ip.c_str());
        }

        inline in_port_t toNetPort(uint16_t port)
        {
            return htons(port);
        }

        template <typename Sockaddr>
        inline std::string toLocIPV4(Sockaddr &&addr)
        {
            char str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(reinterpret_cast<sockaddr_in *>(&addr)->sin_addr), str, sizeof(str));
            std::string ret(str);
            return ret;
        }

        template <typename Sockaddr>
        inline void toLocIPV4(Sockaddr &&addr, std::string &ip)
        {
            char str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(reinterpret_cast<sockaddr_in *>(&addr)->sin_addr), str, sizeof(str));
            ip = str;
        }

        inline in_port_t toLocPort(uint16_t netPort)
        {
            return ntohs(netPort);
        }

        template <typename Sockaddr>
        inline in_port_t toLocPort(Sockaddr &&addr)
        {
            return ntohs(reinterpret_cast<sockaddr_in *>(&addr)->sin_port);
        }

        inline sockaddr_in getSockaddrIn(std::string const &ip, uint16_t port)
        {
            sockaddr_in addrIn;
            std::memset(&addrIn, 0, sizeof(addrIn));

            addrIn.sin_family = AF_INET;
            addrIn.sin_addr.s_addr = toNetIP(ip);
            addrIn.sin_port = toNetPort(port);

            return addrIn;
        }

        inline sockaddr_un getSockaddrUn(std::string const &unixPath)
        {
            sockaddr_un addrUn;
            std::memset(&addrUn, 0, sizeof(addrUn));

            addrUn.sun_family = AF_UNIX;
            std::strncpy(addrUn.sun_path, unixPath.c_str(), sizeof(addrUn.sun_path) - 1);

            return addrUn;
        }

        template <typename Sockaddr>
        inline int bind(int socketFd, Sockaddr const &addr)
        {
            return ::bind(socketFd, reinterpret_cast<sockaddr const *>(&addr), sizeof(addr));
        }

        template <typename Sockaddr>
        int connect(int socketFd, Sockaddr &&addr)
        {
            return ::connect(socketFd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
        }

        template <typename Sockaddr>
        int accept(int fd, Sockaddr &&addr)
        {
            socklen_t addrLen = sizeof(Sockaddr);
            return ::accept(fd, reinterpret_cast<sockaddr *>(&addr), &addrLen);
        }

        inline int setNonBlock(int fd)
        {
            int flag = fcntl(fd, F_GETFL, 0);
            if (flag < 0)
            {
                return flag;
            }

            return fcntl(fd, F_SETFL, flag | O_NONBLOCK);
        }

        inline int setBlock(int fd)
        {
            int flag = fcntl(fd, F_GETFL, 0);
            if (flag < 0)
            {
                return flag;
            }

            return fcntl(fd, F_SETFL, flag & ~O_NONBLOCK);
        }

        class Socket
        {
        public:
            Socket(IPVersion ipv, Protocol protocol = Protocol::TCP)
                : m_fd(getSocketFd(protocol, ipv))
            {
                m_fd >= 0 ? m_isClose = false : m_isClose = true;
            }

            Socket(int fd) noexcept
                : m_fd(fd), m_isClose(false) {}

            Socket(Socket const &) = delete;

            Socket(Socket &&sfd) noexcept
            {
                if (*this)
                {
                    ::close(m_fd);
                }

                m_fd = sfd.get();

                sfd.m_isClose = true;
            }

            Socket() noexcept
                : m_fd(-1), m_isClose(true) {}

            virtual ~Socket()
            {
                if (m_isClose)
                {
                    return;
                }

                ::close(m_fd);
            }

            operator bool() noexcept
            {
                return m_fd >= 0 ? true : false;
            }

            bool isClose() { return m_isClose; }

            int close()
            {
                int ret = ::close(m_fd);

                if (ret >= 0)
                {
                    m_isClose = true;
                    m_fd = -1;
                }

                return ret;
            }

            int get() const
            {
                return m_fd;
            }

            int createFD(IPVersion ipv, Protocol protocol = Protocol::TCP)
            {
                if (!m_isClose)
                {
                    ::close(m_fd);
                    m_isClose = true;
                }

                m_fd = getSocketFd(protocol, ipv);
                m_fd >= 0 ? m_isClose = false : m_isClose = true;

                return m_fd;
            }

            template <typename Sockaddr>
            int bind(Sockaddr &&addr)
            {
                return Net::bind(m_fd, std::forward<Sockaddr>(addr));
            }

            int listen(int n)
            {
                return ::listen(m_fd, 20);
            }

            template <typename Sockaddr>
            int accept(Sockaddr &&addr)
            {
                return Net::accept(m_fd, std::forward<Sockaddr>(addr));
            }

            int setNonBlock()
            {
                return Net::setNonBlock(m_fd);
            }

            int setBlock()
            {
                return Net::setBlock(m_fd);
            }

            template <typename Sockaddr>
            int connect(Sockaddr &&addr)
            {
                return ::connect(m_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
            }

            template <typename Sockaddr>
            int connect(Sockaddr &&addr, timeval &timeOut)
            {
                if (Net::setNonBlock(m_fd) < 0)
                {
                    return -1;
                }

                int ret = ::connect(m_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
                if (ret < 0 && errno == EINPROGRESS)
                {
                    fd_set fdSet;
                    FD_ZERO(&fdSet);
                    FD_SET(m_fd, &fdSet);

                    do
                    {
                        ret = select(m_fd + 1, nullptr, &fdSet, nullptr, &timeOut);
                    } while (ret < 0 && errno == EINTR);

                    int err = errno;
                    Net::setBlock(m_fd);

                    if (ret == 0)
                    {
                        errno = ETIMEDOUT;
                        return -1;
                    }
                    else if (ret < 0)
                    {
                        errno = err;
                        return -1;
                    }
                    else if (ret == 1)
                    {
                        socklen_t socklen = sizeof(Sockaddr);
                        if (getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &err, &socklen) == -1)
                        {
                            return -1;
                        }

                        if (err == 0)
                        {
                            return 0;
                        }
                        else
                        {
                            errno = err;
                            return -1;
                        }
                    }
                }

                Net::setBlock(m_fd);
                return ret;
            }

            int reuseAddr()
            {
                return Net::reuseAddr(m_fd);
            }

            template <typename Sockaddr>
            int getpeername(Sockaddr &&addr, socklen_t *lenPtr) const
            {
                return ::getpeername(m_fd, reinterpret_cast<sockaddr *>(&addr), lenPtr);
            }

            ssize_t read(int fd, void *buf, size_t n)
            {
                return ::read(fd, buf, n);
            }

            ssize_t write(const void *buf, size_t n)
            {
                return ::write(m_fd, buf, n);
            }

            ssize_t write(std::string const &buf)
            {
                return ::write(m_fd, buf.c_str(), buf.size());
            }

        protected:
            int m_fd;
            bool m_isClose;
        };

    }
}

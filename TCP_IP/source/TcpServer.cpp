#include "../include/TcpServer.h"
#include <chrono>
#include <cstring>
#include <mutex>
#include <iostream>

#define   INT_TO_STRING(in)  std::to_string(int(reinterpret_cast<char*>in))

using namespace tcp;


TcpServer::TcpServer(const uint16_t port,
                     KeepAliveConfig ka_conf,
                     handler_function_t handler,
                     con_handler_function_t connect_hndl,
                     con_handler_function_t disconnect_hndl,
                     uint thread_count
                     )
  : port(port),
    handler(handler),
    connect_hndl(connect_hndl),
    disconnect_hndl(disconnect_hndl),
    thread_pool(thread_count),
    ka_conf(ka_conf)
    {}

TcpServer::~TcpServer() {
  if(_status == status::up)
    stop();
}

void TcpServer::setHandler(TcpServer::handler_function_t handler) {
    this->handler = handler;
}

uint16_t TcpServer::getPort() const {
    return port;
}

uint16_t TcpServer::setPort( const uint16_t port) {
	this->port = port;
	start();
	return port;
}

TcpServer::status TcpServer::start() {
  int flag;

  if(_status == status::up) {
    stop();
  }

  SocketAddr_in address;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  address.sin_family = AF_INET;

  serv_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if(serv_socket == ERR) {
    _status = status::err_socket_init;
    return _status;
  }

  // Set nonblocking accept
  // not needed becouse socket created with flag SOCK_NONBLOCK
  if(fcntl(serv_socket, F_SETFL, fcntl(serv_socket, F_GETFL, 0) | O_NONBLOCK) < 0) {
    _status = status::err_socket_init;
    return _status;
  }

  // Bind address to socket
  flag = true;
  if((setsockopt(serv_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag)) == ERR) ||
     (bind(serv_socket, (struct sockaddr*)&address, sizeof(address)) < 0)) {
        return _status = status::err_socket_bind;
    }

  if(listen(serv_socket, SOMAXCONN) < 0) {
    _status = status::err_socket_listening;
    return _status;
  }

  _status = status::up;
  thread_pool.addJob([this]{handlingAcceptLoop();});
  thread_pool.addJob([this]{waitingDataLoop();});
  return _status;
}

void TcpServer::stop() {
  thread_pool.dropUnstartedJobs();
  _status = status::close;
  close(serv_socket);
  client_list.clear();
}

void TcpServer::joinLoop() {
    thread_pool.join();
}

void TcpServer::sendData(const void* buffer, const size_t size) {
  for(std::unique_ptr<Client>& client : client_list) {
    client->sendData(buffer, size);
  }

}

void TcpServer::handlingAcceptLoop() {
  SockLen_t addrlen = sizeof(SocketAddr_in);
  SocketAddr_in client_addr;
  Socket client_socket = accept4(serv_socket, (struct sockaddr*)&client_addr, &addrlen, SOCK_NONBLOCK);

  if ((client_socket >= 0) && (_status == status::up)) {
    // Enable keep alive for client
    if(enableKeepAlive(client_socket)) {
      std::unique_ptr<Client> client(new Client(client_socket, client_addr));
      connect_hndl(*client);
      client_mutex.lock();
      client_list.emplace_back(std::move(client));
      client_mutex.unlock();
    } else {
      shutdown(client_socket, 0);
      close(client_socket);
    }
  }

  if(_status == status::up) {
    thread_pool.addJob([this](){handlingAcceptLoop();});
  }
}

void TcpServer::waitingDataLoop() {
  {
    std::lock_guard lock(client_mutex);
    for(auto it = client_list.begin(), end = client_list.end(); it != end; ++it) {
      auto& client = *it;
      if(client) {
        ReceivedData data = client->loadData();
        if(!data.empty()) {

          thread_pool.addJob([this, _data = std::move(data), &client]{
            client->access_mtx.lock();
            handler(std::move(_data), *client);
            client->access_mtx.unlock();
          });
        } else if(client->_status == SocketStatus::disconnected) {

          thread_pool.addJob([this, &client, it]{
            client->access_mtx.lock();
            Client* pointer = client.release();
            client = nullptr;
            pointer->access_mtx.unlock();
            disconnect_hndl(*pointer);
            client_list.erase(it);
            delete pointer;
          });
        }
      }
    }
  }

  if(_status == status::up) {
    thread_pool.addJob([this](){waitingDataLoop();});
  }
}

bool TcpServer::enableKeepAlive(Socket socket) {
  int flag = 1;

  if(setsockopt(socket, SOL_SOCKET,  SO_KEEPALIVE,  &flag,             sizeof(flag)) == ERR)             return false;
  if(setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE,  &ka_conf.ka_idle,  sizeof(ka_conf.ka_idle)) == ERR)  return false;
  if(setsockopt(socket, IPPROTO_TCP, TCP_KEEPINTVL, &ka_conf.ka_intvl, sizeof(ka_conf.ka_intvl)) == ERR) return false;
  if(setsockopt(socket, IPPROTO_TCP, TCP_KEEPCNT,   &ka_conf.ka_cnt,   sizeof(ka_conf.ka_cnt)) == ERR)   return false;

  return true;
}

bool TcpServer::Client::sendData(const void* buffer, const size_t sizeBuffer) const {

  if(_status != SocketStatus::connected) {
    return false;
  }

  void* send_buffer = malloc(sizeBuffer + sizeof (uint32_t));
  memcpy(reinterpret_cast<char*>(send_buffer) + sizeof(uint32_t), buffer, sizeBuffer);
  *reinterpret_cast<uint32_t*>(send_buffer) = sizeBuffer;

  if(send(socket, reinterpret_cast<char*>(send_buffer), sizeBuffer + sizeof (int), 0) < 0) {
    return false;
  }

  free(send_buffer);
  return true;
}

TcpServer::Client::Client(Socket socket, SocketAddr_in address) : socket(socket), address(address) {}

TcpServer::Client::~Client() {
  if(socket == ERR) {
    return;
  }
  shutdown(socket, SD_BOTH);
  close(socket);
}

uint32_t TcpServer::Client::getHost() const {
    return address.sin_addr.s_addr;
}

uint16_t TcpServer::Client::getPort() const {
    return address.sin_port;
}

ReceivedData TcpServer::Client::loadData() {
  if(_status != SocketStatus::connected) {
    return ReceivedData();
  }

  ReceivedData buffer;
  uint32_t sizeBuffer;
  int err;

  int answ = recv(socket, (char*)&sizeBuffer, sizeof(sizeBuffer), MSG_DONTWAIT);

  // Disconnect
  if(!answ) {
    disconnect();
    return ReceivedData();
  } else if(answ == ERR) {

    SockLen_t len = sizeof (err);
    getsockopt (socket, SOL_SOCKET, SO_ERROR, (char*)&err, &len);

    if(!err) {
        err = errno;
    }

    switch (err)
    {
      case 0: break;
        // Keep alive timeout
      case ETIMEDOUT:
      case ECONNRESET:
      case EPIPE:
        disconnect();
        [[fallthrough]];
        // No data
      case EAGAIN: return ReceivedData();
      default:
        disconnect();
        std::cout << "Unhandled error!\n" << "Code: " << err << " Err: " << std::strerror(err) << '\n';
      return ReceivedData();
    }
  }

  if(!sizeBuffer) {
    return ReceivedData();
    }

  buffer.resize(sizeBuffer);
  recv(socket, buffer.data(), buffer.size(), 0);
  return buffer;
}

TcpBase::status TcpServer::Client::disconnect() {
  _status = status::disconnected;
  if(socket == ERR) {
    return _status;
    }
  shutdown(socket, SD_BOTH);
  close(socket);
  socket = ERR;
  return _status;
}

std::string TcpServer::Client::getHostStr(const tcp::TcpServer::Client& client) {
    uint32_t ip = client.getHost ();
    return std::string() + INT_TO_STRING((&ip)[0]) + '.' +
                           INT_TO_STRING((&ip)[1]) + '.' +
                           INT_TO_STRING((&ip)[2]) + '.' +
                           INT_TO_STRING((&ip)[3]) + ':' + std::to_string( client.getPort ());
}

#include "../include/TcpServer.h"

using namespace tcp;

#define WIN(exp)
#define NIX(exp) exp


#include <iostream>

ReceivedData TcpServer::Client::loadData() {
  if(_status != SocketStatus::connected) return ReceivedData();
  using namespace std::chrono_literals;
  ReceivedData buffer;
  uint32_t size;
  int err;

  // Read data length in non-blocking mode
  // MSG_DONTWAIT - Unix non-blocking read
  WIN(if(u_long t = true; SOCKET_ERROR == ioctlsocket(socket, FIONBIO, &t)) return DataBuffer();) // Windows non-blocking mode on
  int answ = recv(socket, (char*)&size, sizeof(size), NIX(MSG_DONTWAIT)WIN(0));
  WIN(if(u_long t = false; SOCKET_ERROR == ioctlsocket(socket, FIONBIO, &t)) return DataBuffer();) // Windows non-blocking mode off

  // Disconnect
  if(!answ) {
    disconnect();
    return ReceivedData();
  } else if(answ == -1) {
    // Error handle (f*ckin OS-dependence!)
    WIN(
      err = convertError();
      if(!err) {
        SockLen_t len = sizeof (err);
        getsockopt (socket, SOL_SOCKET, SO_ERROR, WIN((char*))&err, &len);
      }
    )NIX(
      SockLen_t len = sizeof (err);
      getsockopt (socket, SOL_SOCKET, SO_ERROR, WIN((char*))&err, &len);
      if(!err) err = errno;
    )


    switch (err) {
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
        std::cerr << "Unhandled error!\n"
                    << "Code: " << err << " Err: " << std::strerror(err) << '\n';
      return ReceivedData();
    }
  }

  if(!size) return ReceivedData();
  buffer.resize(size);
  recv(socket, buffer.data(), buffer.size(), 0);
  return buffer;
}


TcpBase::status TcpServer::Client::disconnect() {
  _status = status::disconnected;
  if(socket == WIN(INVALID_SOCKET)NIX(-1)) return _status;
  shutdown(socket, SD_BOTH);
  WIN(closesocket)NIX(close)(socket);
  socket = WIN(INVALID_SOCKET)NIX(-1);
  return _status;
}

bool TcpServer::Client::sendData(const void* buffer, const size_t size) const {
  if(_status != SocketStatus::connected) return false;

  void* send_buffer = malloc(size + sizeof (uint32_t));
  memcpy(reinterpret_cast<char*>(send_buffer) + sizeof(uint32_t), buffer, size);
  *reinterpret_cast<uint32_t*>(send_buffer) = size;

  if(send(socket, reinterpret_cast<char*>(send_buffer), size + sizeof (int), 0) < 0) return false;

  free(send_buffer);
  return true;
}

TcpServer::Client::Client(Socket socket, SocketAddr_in address)
  : address(address), socket(socket) {}


TcpServer::Client::~Client() {
  if(socket == WIN(INVALID_SOCKET)NIX(-1)) return;
  shutdown(socket, SD_BOTH);
  WIN(closesocket(socket);)
  NIX(close(socket);)
}

uint32_t TcpServer::Client::getHost() const {return NIX(address.sin_addr.s_addr) WIN(address.sin_addr.S_un.S_addr);}
uint16_t TcpServer::Client::getPort() const {return address.sin_port;}


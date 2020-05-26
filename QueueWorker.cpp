//
// Created by sokol on 19.05.20.
//

#include <zmq.hpp>
int main(int argc, char* argv[]) {
  zmq::context_t ctx;
  zmq::socket_t sock(ctx, zmq::socket_type::req);
  sock.bind("tcp://*:5554");
  sock.send(zmq::str_buffer("Hello World"), zmq::send_flags::dontwait);
  return 0;
}
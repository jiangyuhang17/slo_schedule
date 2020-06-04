#ifndef MANAGER_H
#define MANAGER_H

#include <vector>
#include <memory>

#include <event2/listener.h>

#include "Connection.h"

using namespace std;

void accept_cb(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int socklen, void *);
void timer_cb(evutil_socket_t fd, short what, void *arg);

class Manager {
public:
  Manager();
  ~Manager();

  void accept_callback(evutil_socket_t sockfd, struct sockaddr *addr, int socklen);
  void timer_callback();
private:
  struct event_base *base_;
  struct evconnlistener *listener_;
  struct event *timer_;

  char* mem_file_;
  size_t num_block_ = 100; // TODO

  vector<Connection*> conns_;
  double interval_ = 5.0; // TODO

};

#endif
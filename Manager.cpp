#include <cstring>

#include <event2/event.h>
#include <event2/bufferevent.h>

#include "Manager.h"
#include "log.h"
#include "config.h"
#include "util.h"

Manager::Manager() {
  mem_file_ = new char[BLOCK_SIZE * num_block_];
  memset(mem_file_, 'a', BLOCK_SIZE * num_block_);

  struct event_config *config;

  if ((config = event_config_new()) == NULL) 
    DIE("event_config_new() fail");
  if (event_config_avoid_method(config, "epoll"))
    DIE("event_config_avoid_method() fail");

  if ((base_ = event_base_new_with_config(config)) == NULL)
    DIE("event_base_new_with_config() fail");
  if (strcmp(event_base_get_method(base_), "poll"))
    DIE("event_base is not using poll");
  
  event_config_free(config);

  struct evutil_addrinfo hints;
  struct evutil_addrinfo *answer = NULL;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (evutil_getaddrinfo(NULL, PORT, &hints, &answer))
    DIE("evutil_getaddrinfo() fail");

  listener_ = evconnlistener_new_bind(base_, accept_cb, this, 
    LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1, answer->ai_addr, sizeof(*answer->ai_addr));
  if (listener_ == NULL)
    DIE("evconnlistener_new_bind() fail");

  timer_ = evtimer_new(base_, timer_cb, this);
  struct timeval tv;
  double_to_tv(interval_, &tv);
  evtimer_add(timer_, &tv);

  I("start event loop");
  event_base_dispatch(base_);

  evutil_freeaddrinfo(answer);
}

Manager::~Manager() {
  for (auto iter = conns_.begin(); iter != conns_.end(); iter++) {
    delete *iter;
  }
  event_free(timer_);
  evconnlistener_free(listener_);
  event_base_free(base_);
}

void Manager::accept_callback(evutil_socket_t sockfd, struct sockaddr *addr, int socklen) {
  conns_.push_back(new Connection {base_, sockfd, mem_file_, num_block_});
}

void Manager::timer_callback() {
  I("In schedule");
  struct timeval tv;
  double_to_tv(interval_, &tv);
  evtimer_add(timer_, &tv);
}

void accept_cb(struct evconnlistener *listener, evutil_socket_t sockfd, struct sockaddr *addr, int socklen, void *arg) {
  Manager *m = (Manager*)arg;
  m->accept_callback(sockfd, addr, socklen);
}

void timer_cb(evutil_socket_t fd, short what, void *arg) {
  Manager *m = (Manager*)arg;
  m->timer_callback();
}
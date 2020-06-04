#include <cstring>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "Connection.h"
#include "log.h"
#include "config.h"

Connection::Connection(struct event_base* base, evutil_socket_t fd, char* mem_file, size_t num_block)
  : base_(base), fd_(fd), mem_file_(mem_file), num_block_(num_block) {
  if (fd_ < 0)
    DIE("sockfd < 0");
  I("Accept %d.", fd_);  
  read_state = CONN_SETUP;
  bev_ = bufferevent_socket_new(base_, fd_, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev_, bev_read_cb, bev_write_cb, bev_event_cb, this);
  bufferevent_enable(bev_, EV_READ | EV_WRITE);

  // bufferevent_set_max_single_read(bev_, 70000);
  // bufferevent_set_max_single_write(bev_, 70000);
};

Connection::~Connection() {
  bufferevent_free(bev_);
}

void Connection::read_callback() {
  char *buf = NULL;
  long int offset = 0;
  switch (read_state) {
  case CONN_SETUP:
    buf = evbuffer_readln(bufferevent_get_input(bev_), NULL, EVBUFFER_EOL_CRLF);
    I(buf);
    evbuffer_add_printf(bufferevent_get_output(bev_), "OK\r\n");
    read_state = RUN;
    break;
  case RUN:
    buf = evbuffer_readln(bufferevent_get_input(bev_), NULL, EVBUFFER_EOL_CRLF);
    if (buf == NULL) {
      W("evbuffer_readln() fail");
      break;
    }
    I("remain len: %d", remain_length_);
    I("buf %s",buf + remain_length_);
    if (!strncmp(buf + remain_length_, "get ", 4)) {
      remain_length_ = 0;
      offset = atoi(buf + 4) % num_block_;
      I("get offset %ld", offset);
      evbuffer_add_printf(bufferevent_get_output(bev_), "resp get\r\n");
      bufferevent_write(bev_, mem_file_ + offset * BLOCK_SIZE, BLOCK_SIZE);
    } else if (!strncmp(buf + remain_length_, "set ", 4)) {
      remain_length_ = 0;
      offset = atoi(buf + 4) % num_block_;
      I("set offset %ld", offset);
      size_t len = bufferevent_read(bev_, mem_file_ + offset * BLOCK_SIZE, BLOCK_SIZE);
      I("len: %d", len);
      remain_length_ = BLOCK_SIZE - len;
      evbuffer_add_printf(bufferevent_get_output(bev_), "END\r\n");
    } else {
      W("undefined behavior");
    }
    break;
  default:
    DIE("Not implemented.");
  }
}

void Connection::write_callback() { }

void Connection::event_callback(short events) {
  if (events & BEV_EVENT_EOF) {
    W("fd: %d, connection has been closed, do any clean up here", fd_);
  } else if (events & BEV_EVENT_ERROR) {
    W("fd: %d, BEV_EVENT_ERROR: %s", fd_, strerror(errno));
  } else if (events & BEV_EVENT_TIMEOUT) {
    W("fd: %d, must be a timeout event handle, handle it", fd_);
  }
  fd_ = -1;
}

void bev_read_cb(struct bufferevent *bev, void *ptr) {
  I("in read");
  Connection *conn = (Connection*)ptr;
  conn->read_callback();
}

void bev_write_cb(struct bufferevent *bev, void *ptr) {
  I("in write");
  Connection *conn = (Connection*)ptr;
  conn->write_callback();
}

void bev_event_cb(struct bufferevent *bev, short events, void *ptr) {
  Connection *conn = (Connection*)ptr;
  conn->event_callback(events);
}
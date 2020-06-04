#ifndef CONNECTION_H
#define CONNECTION_H

#include <event2/event.h>

void bev_event_cb(struct bufferevent *bev, short events, void *ptr);
void bev_read_cb(struct bufferevent *bev, void *ptr);
void bev_write_cb(struct bufferevent *bev, void *ptr);

class Connection {
public:
  Connection(struct event_base* base, evutil_socket_t fd, char* mem_file, size_t num_block);
  ~Connection();

  void read_callback();
  void write_callback();
  void event_callback(short events);
  
private:
  struct event_base *base_;
  evutil_socket_t fd_;
  struct bufferevent *bev_;

  char *mem_file_;
  size_t num_block_;

  enum read_state_enum {
    CONN_SETUP,
    RUN
  };

  enum read_state_enum read_state;
  int remain_length_ = 0;


  bool is_latency_;
  double start_time_;

  enum tail_latency_type {
    tail_latency_95th
  };

  double slo_iops_;
  double slo_avg_latency_;
  enum tail_latency_type slo_tail_type_;
  double slo_tail_latency_;
  double rw_ratio_;
};

#endif
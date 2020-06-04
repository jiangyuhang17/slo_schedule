#include <cstring>

#include <event2/event.h>

#include "Manager.h"
#include "log.h"
#include "config.h"

int main(int argc, char **argv) {

  setvbuf(stdout, NULL, _IONBF, 0);

  Manager m;

  return 0;
}

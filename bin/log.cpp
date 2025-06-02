#include "log.h"

int main() {
    LOG_DEBUG("Hello, world!", 123);
    LOG_INFO("This is an info message.");
    LOG_WARN("This is a warning message.");
    LOG_ERROR("This is an error message.");
    return 0;
}
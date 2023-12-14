// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_CONSOLE_SINK_HPP_20231214_0805
#define PSSP_CONSOLE_SINK_HPP_20231214_0805
#pragma once
/* fltk https://www.fltk.org/doc-1.4 */
#include <FL/Fl.H>
#include <FL/Fl_Terminal.H>
/* spdlog https://github.com/gabime/spdlog */
#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>
/* Standard library
   https://en.cppreference.com/w/cpp/standard_library */
#include <iostream>
// std::shared_ptr
#include <memory>
// std::mutex
#include <mutex>
// std::ostringstream
#include <sstream>
// std::string
#include <string>

/* Special Note

Using the g++ compiler, this can be split into interface/implementation
without issues

However, using the clang++ compiler, it cannot as the linker will fail with
an undefined reference.

In the interest of generality, I'm coding this in a way that works with either
compiler
*/

namespace pssp {

template <typename Mutex>
class Console_Sink : public spdlog::sinks::base_sink<Mutex> {
public:
  explicit Console_Sink(Fl_Terminal *tty) { tty_ = tty; }

protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    // log_msg is a struct containing the log entry info like level, timestamp,
    // msg.raw contains the pre-formatted log

    // If needed (very likely, but not madatory), the sink formats the message
    spdlog::memory_buf_t formatted{};
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    std::string message{fmt::to_string(formatted)};
    format_log(&message);
    tty_->append(message.c_str());
  }
  void flush_() override { tty_->clear(); }

private:
  void format_log(std::string *log_msg) {
    // Message comes in as
    // [YYYY-MM-DD HH:MM:SS] [name] [level] [thread #####] Message
    // I want to add color
    std::ostringstream oss{};
    // Date Time
    size_t start{log_msg->find('[')};
    size_t end{log_msg->find(']')};
    oss << "[\033[1m\33[32m";
    oss << log_msg->substr(start + 1, end - 1);
    oss << "\033[0m]";
    // Log-level
    *log_msg = log_msg->substr(end + 2);
    start = log_msg->find('[');
    end = log_msg->find(']');
    oss << "[\033[1m\033[33m";
    oss << log_msg->substr(start + 1, end - 1);
    oss << "\033[0m]";
    // Thread
    *log_msg = log_msg->substr(end + 2);
    start = log_msg->find('[');
    end = log_msg->find(']');
    oss << "[\033[1m\033[36m";
    oss << log_msg->substr(start + 1, end - 1);
    oss << "\033[0m] ";
    // Message
    *log_msg = log_msg->substr(end + 2);
    oss << *log_msg;
    *log_msg = oss.str();
  }
  Fl_Terminal *tty_{};
};

using Console_Sink_mt = Console_Sink<std::mutex>;
using Console_Sink_st = Console_Sink<spdlog::details::null_mutex>;

}  // namespace pssp
#endif

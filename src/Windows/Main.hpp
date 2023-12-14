// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_MAIN_WINDOW_HPP_20231213_1003
#define PSSP_MAIN_WINDOW_HPP_20231213_1003
#pragma once
#include "../Logging/Console_Sink.hpp"
/* fltk https://www.fltk.org/doc-1.4 */
#include <FL/Fl.H>
#include <FL/Fl_Chart.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Terminal.H>
#include <FL/Fl_Window.H>
/* spdlog https://github.com/gabime/spdlog */
#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
/* Standard library
   https://en.cppreference.com/w/cpp/standard_library */
// std::unique_ptr, std::shared_ptr
#include <memory>
// std::string
#include <string>

namespace pssp {
class Main_Window : public Fl_Window {
public:
  Main_Window();
  void append_tty(const char *msg);

private:
  Fl_Sys_Menu_Bar menu{0, 0, 0, 25, nullptr};
  std::unique_ptr<Fl_Chart> plot{};
  void make_menu();
  void make_tty();
  std::shared_ptr<Console_Sink_mt> sink{};
  std::shared_ptr<spdlog::logger> logger{};
  std::unique_ptr<Fl_Terminal> debug_tty{};
  static void quit_cb(Fl_Widget *menu, void *junk);
  inline static const std::string
      // cppcheck-suppress unusedStructMember
      name_{"PsSp - Passive-source Seismic-processing"};
};
}  // namespace pssp
#endif

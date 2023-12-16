// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_MAIN_WINDOW_HPP_20231213_1003
#define PSSP_MAIN_WINDOW_HPP_20231213_1003
#pragma once

#include "../Logging/Console_Sink.hpp"
#include "../Widgets/Datasheet.hpp"
#include "../Widgets/Status_Bar.hpp"
#include "About.hpp"
// fltk https://www.fltk.org/doc-1.4
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Terminal.H>
#include <FL/fl_ask.H>
// spdlog https://github.com/gabime/spdlog
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
// Standard library
//   https://en.cppreference.com/w/cpp/standard_library
// std::unique_ptr, std::shared_ptr
#include <memory>
// std::string
#include <string>

namespace pssp {
class Main_Window : public Fl_Double_Window {
public:
  Main_Window();
  void append_tty(const char *msg);
  void show_about();

private:
  Fl_Sys_Menu_Bar menu{0, 0, 0, 25, nullptr};
  void make_menu();
  void make_tty();
  std::unique_ptr<Status_Bar> status_bar_{};
  std::unique_ptr<Fl_Grid> gridspace_{};
  std::unique_ptr<Fl_Box> list_{};
  std::shared_ptr<Console_Sink_mt> sink{};
  std::shared_ptr<spdlog::logger> logger{};
  std::unique_ptr<Fl_Terminal> debug_tty{};
  std::unique_ptr<About_Window> about_window_{};
  std::unique_ptr<Datasheet> datasheet_{};
  static void quit_cb(Fl_Widget *menu, void *junk);
  static void about_cb(Fl_Widget *menu, void *junk);
  inline static const std::string
      // cppcheck-suppress unusedStructMember
      name_{"PsSp - Passive-source Seismic-processing"};
  static void prevent_escape(Fl_Widget *, void *);  // MwuaHaHaha
};
}  // namespace pssp
#endif

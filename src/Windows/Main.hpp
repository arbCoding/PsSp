// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_MAIN_WINDOW_HPP_20231213_1003
#define PSSP_MAIN_WINDOW_HPP_20231213_1003
#pragma once
/* fltk */
// Needed for all fltk
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Terminal.H>
#include <FL/Fl_Window.H>
/* Standard library
   https://en.cppreference.com/w/cpp/standard_library */
// std::string
#include <iostream>
#include <string>

namespace pssp {
constexpr int mw_width{500};
constexpr int mw_height{500};
constexpr int term_height{100};

class Main_Window : public Fl_Window {
public:
  Main_Window();
  void append_tty(const char *msg);

private:
  Fl_Sys_Menu_Bar menu{0, 0, mw_width, 15, nullptr};
  void make_menu();
  void make_tty();
  // cppcheck-suppress unusedStructMember
  Fl_Terminal *debug_tty{};
  static void quit_cb(Fl_Widget *menu, void *junk);
  inline static const std::string
      // cppcheck-suppress unusedStructMember
      name_{"PsSp - Passive-source Seismic-processing"};
};
}  // namespace pssp
#endif

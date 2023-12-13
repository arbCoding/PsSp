// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_MAIN_WINDOW_HPP_20231213_1003
#define PSSP_MAIN_WINDOW_HPP_20231213_1003
#pragma once
/* fltk */
// Needed for all fltk
#include <FL/Fl.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Window.H>
/* Standard library
   https://en.cppreference.com/w/cpp/standard_library */
// std::string
#include <string>
#include <iostream>

namespace pssp {
constexpr int mw_width{500};
constexpr int mw_height{500};

class Main_Window : public Fl_Window {
public:
  Main_Window();

private:
  Fl_Sys_Menu_Bar menu{0, 0, mw_width, 15, nullptr};
  static void quit_cb(Fl_Widget *, void*) { exit(0); }
  inline static const std::string
      // cppcheck-suppress unusedStructMember
      name_{"PsSp - Passive-source Seismic-processing"};
};
}  // namespace pssp
#endif

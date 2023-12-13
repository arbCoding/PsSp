// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_MAIN_WINDOW_HPP_20231213_1003
#define PSSP_MAIN_WINDOW_HPP_20231213_1003
#pragma once
/* fltk */
// Needed for all fltk
#include <FL/Fl.H>
// Fl_Window widget
#include <FL/Fl_Window.H>
/* Standard library
   https://en.cppreference.com/w/cpp/standard_library */
// std::string
#include <string>

namespace pssp {
constexpr int mw_width{500};
constexpr int mw_height{500};

class Main_Window : public Fl_Window {
public:
  Main_Window();

private:
  inline static const std::string
      // cppcheck-suppress unusedStructMember
      name_{"PsSp - Passive-source Seismic-processing"};
};
}  // namespace pssp
#endif

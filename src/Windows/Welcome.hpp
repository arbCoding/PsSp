// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_WELCOME_WINDOW_HPP_20231213_1019
#define PSSP_WELCOME_WINDOW_HPP_20231213_1019
#pragma once
/* fltk */
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Window.H>
/* Standard library
   https://en.cppreference.com/w/cpp/standard_library */
// std::string
#include <string>

namespace pssp {
constexpr int hw_x{1500 / 2};
constexpr int hw_y{900 / 2};
constexpr int hw_width{400};
constexpr int hw_height{80};

class Welcome_Window : public Fl_Window {
public:
  Welcome_Window();
  Fl_Box message{0, 0, hw_width, 50};
  Fl_Return_Button continue_button{(hw_width - 150) / 2, 50, 150, 25,
                                   "Continue"};

private:
  static void continue_cb(Fl_Widget *btn);
  inline static const std::string
      // cppcheck-suppress unusedStructMember
      message_{"Welcome to Post-seismic Processing-software (PsSp)!\n"
               "This program is very early in development..."};
};
}  // namespace pssp
#endif

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
constexpr int hw_x{10};
constexpr int hw_y{10};
constexpr int hw_width{200};
constexpr int hw_height{50};

class Welcome_Window : public Fl_Window {
public:
  Welcome_Window();
  Fl_Box message{hw_x + (hw_width / 2), hw_y + 10, hw_width, hw_height / 2};
  Fl_Return_Button continue_button{hw_x + (hw_width / 2) + 30,
                                   hw_y + hw_height, 150,
                                   hw_height / 2, "Continue"};

private:
  static void continue_cb(Fl_Widget *btn);
  inline static const std::string
      // cppcheck-suppress unusedStructMember
      message_{"Welcome to Post-seismic Processing-software (PsSp)!\n"
      "This program is very early in development..."};
};
}  // namespace pssp
#endif

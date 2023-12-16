// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_WELCOME_WINDOW_HPP_20231213_1019
#define PSSP_WELCOME_WINDOW_HPP_20231213_1019
#pragma once

/* fltk https://www.fltk.org/doc-1.4 */
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Window.H>
/* Standard library
   https://en.cppreference.com/w/cpp/standard_library */
// std::unique_ptr
#include <memory>
// std::string
#include <string>

namespace pssp {
namespace welcome {
constexpr int button_width{125};
constexpr int button_height{25};
constexpr int text_height{50};
constexpr int height{text_height + button_height + 10};
constexpr int text_width{380};
constexpr int width{text_width + 20};
}  // namespace welcome

class Welcome_Window : public Fl_Window {
public:
  Welcome_Window();
  std::unique_ptr<Fl_Box> message{};
  std::unique_ptr<Fl_Return_Button> continue_button{};

private:
  static void continue_cb(Fl_Widget *btn);
  inline static const std::string
      // cppcheck-suppress unusedStructMember
      message_{"Welcome to Passive-source Seismic-processing (PsSp)!\n"
               "This program is very early in development..."};
};
}  // namespace pssp
#endif

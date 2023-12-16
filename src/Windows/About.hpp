// Copyright Alexander R. Blanchette

#ifndef PSSP_ABOUT_WINDOW_HPP_20231214_1411
#define PSSP_ABOUT_WINDOW_HPP_20231214_1411
#pragma once

// fltk https://www.fltk.org/doc-1.4
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Window.H>
// Standard library
//   https://en.cppreference.com/w/cpp/standard_library
// std::unique_ptr
#include <memory>
// std::string
#include <string>

namespace pssp {
namespace about {
constexpr int button_width{75};
constexpr int button_height{25};
constexpr int text_height{90};
constexpr int height{text_height + button_height + 10};
constexpr int text_width{330};
constexpr int width{text_width + 50};
}  // namespace about

class About_Window : public Fl_Window {
public:
  About_Window();
  std::unique_ptr<Fl_Box> message{};
  std::unique_ptr<Fl_Return_Button> okay_button{};

private:
  static void okay_cb(Fl_Widget *btn);
  inline static const std::string
      // cppcheck-suppress unusedStructMember
      message_{"Website: https://arbCoding.github.io/PsSp/\n"
               "GitHub: https://arbCoding.github.com/PsSp\n"
               "Developer: Alexander R. Blanchette <arbCoding@gmail.com>"
               "License: MIT"};
};
}  // namespace pssp
#endif

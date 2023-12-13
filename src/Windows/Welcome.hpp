// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_WELCOME_WINDOW_HPP_20231213_1019
#define PSSP_WELCOME_WINDOW_HPP_20231213_1019
#pragma once
/* fltk */
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
/* Standard library
   https://en.cppreference.com/w/cpp/standard_library */
// std::string_view
#include <string_view>

namespace pssp {
constexpr int hw_width{50};
constexpr int hw_height{50};

class Welcome_Window : public Fl_Window {
public:
  Welcome_Window();

private:
  inline static constexpr std::string_view
      // cppcheck-suppress unusedStructMember
      message_{"Test text!"};
};
}  // namespace pssp
#endif

// Copyright 2023-2024 Alexander R. Blanchette

/*!
  \file PsSp/Windows/About.hpp
  \brief AboutWindow Interface.
  \author Alexander R. Blanchette
  This file contains the AboutWindow class interface, which provides information
  about the PsSp application.
  */

#ifndef PSSP_ABOUTWINDOW_HPP_20231214_1411
#define PSSP_ABOUTWINDOW_HPP_20231214_1411
#pragma once
// PsSp
#include "PsSp/Utility/Structs.hpp"
// fltk https://www.fltk.org/doc-1.4
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
/*!
  \namespace pssp::about

  Constants specific to the AboutWindow.

  \todo Move this to PsSp/Utility/Constants.hpp
  */
namespace about {
//! Width (pixels) of the AboutWindow.okay_button object.
constexpr int button_width{75};
//! Height (pixels) of the AboutWindow.okay_button object.
constexpr int button_height{25};
//! Height (pixels) of the AboutWindow.message object.
constexpr int text_height{90};
//! Height (pixels) of the AboutWindow.
constexpr int height{text_height + button_height + 10};
//! Width (pixels) of the AboutWindow.message object.
constexpr int text_width{330};
//! Width (pixels) of the AboutWindow.
constexpr int width{text_width + 50};
}  // namespace about

/*!
  \class AboutWindow
  \brief Class to provide the About Window.

  This provides the about window for the PsSp program.
 */
class AboutWindow : public Fl_Window {
public:
  AboutWindow();
  std::unique_ptr<Fl_Box> message{};
  std::unique_ptr<Fl_Return_Button> okay_button{};

private:
  static void okay_cb(Fl_Widget *btn);
  //! Message to display in the about window.
  inline static const std::string
      // cppcheck-suppress unusedStructMember
      message_{"Website: https://arbCoding.github.io/PsSp/\n"
               "GitHub: https://arbCoding.github.com/PsSp\n"
               "Developer: Alexander R. Blanchette <arbCoding@gmail.com>"
               "License: MIT"};
};
}  // namespace pssp
#endif

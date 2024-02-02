// Copyright 2023-2024 Alexander R. Blanchette

/*!
  \file PsSp/Windows/Welcome.hpp
  \brief WelcomeWindow interface.
  \author Alexander R. Blanchette
  This file contains the WelcomeWindow class interface, which provides a window
  that is open when the user opens PsSp for the first time with some
  information.
  */

#ifndef PSSP_WELCOMEWINDOW_HPP_20231213_1019
#define PSSP_WELCOMEWINDOW_HPP_20231213_1019
#pragma once
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
  \namespace pssp::welcome

  Constants specific to the WelcomeWindow.

  \todo Move this to PsSp/Utility/Constants.hpp
  */
namespace welcome {
//! Width of WelcomeWindow.continue_button (pixels).
constexpr int button_width{125};
//! Height of WelcomeWindow.continue_button (pixels).
constexpr int button_height{25};
//! Height of WelcomeWindow.message box (pixels).
constexpr int text_height{50};
//! Height of WelcomeWindow (pixels).
constexpr int height{text_height + button_height + 10};
//! Width of WelcomeWindow.message box (pixels).
constexpr int text_width{380};
//! Width of WelcomeWindow (pixels).
constexpr int width{text_width + 20};
}  // namespace welcome

/*!
  \class WelcomeWindow
  \brief Class to provide a Welcome Window

  This provides a welcome window that is open on program startup.

  \todo Auto-size window to size of message.
  \todo "Do not show again" checkbox.
  */
class WelcomeWindow : public Fl_Window {
public:
  WelcomeWindow();
  //! Box to contain message.
  std::unique_ptr<Fl_Box> message{};
  //! Button to close window.
  std::unique_ptr<Fl_Return_Button> continue_button{};

private:
  //! Continue button callback function.
  static void continue_cb(Fl_Widget *btn);
  //! Message to display in the welcome window.
  inline static const std::string
      // cppcheck-suppress unusedStructMember
      message_{"Welcome to Passive-source Seismic-processing (PsSp)!\n"
               "This program is very early in development..."};
};
}  // namespace pssp
#endif

// Copyright 2023-2024 Alexander R. Blanchette

/*!
  \file PsSp/Windows/Main.hpp
  \brief MainWindow interface.
  \author Alexander R. Blanchette
  This file contains the MainWindow class interface, which provides the main
  program window for the PsSp applicaion.

  This includes the debug console (log display), the Datasheet (spreadsheet
  display), the window menu, and the (currently) prototype data-organization
  placeholder.
  */

#ifndef PSSP_MAINWINDOW_HPP_20231213_1003
#define PSSP_MAINWINDOW_HPP_20231213_1003
#pragma once
// PsSp
#include "PsSp/Logging/ConsoleSink.hpp"
#include "PsSp/Utility/Structs.hpp"
#include "PsSp/Widgets/Datasheet.hpp"
#include "PsSp/Widgets/StatusBar.hpp"
#include "PsSp/Windows/About.hpp"
// fltk https://www.fltk.org/doc-1.4
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Grid.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Terminal.H>
#include <FL/fl_ask.H>
// spdlog https://github.com/gabime/spdlog
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
// Standard library
//   https://en.cppreference.com/w/cpp/standard_library
// std::unique_ptr, std::shared_ptr
#include <memory>
// std::string
#include <string>

namespace pssp {
/*!
  \namespace pssp::mw

  Constants specific to the MainWindow.

  \todo Move this to PsSp/Utility/Constants.hpp
  */
namespace mw {
//! Minimum width of the MainWindow.
constexpr int minimum_x{300};
//! Minimum height of the MainWindow.
constexpr int minimum_y{300};
//! Height of the menubar (Linux/Windows only).
constexpr int menu_height{25};
}  // namespace mw

/*!
  \class MainWindow
  \brief Class to provide the Main Window.

  This provides the main window for the PsSp program.

  \todo Work on record-organization sidebar object.
  */
class MainWindow : public Fl_Double_Window {
public:
  MainWindow();
  void append_tty(const char *msg);
  void show_about();

private:
  //! The menubar (Window/Linux) or systembar (macOS).
  Fl_Sys_Menu_Bar menu{0, 0, 0, mw::menu_height, nullptr};
  void make_menu();
  void make_tty();
  //! PsSp StatusBar.
  std::unique_ptr<StatusBar> status_bar_{};
  //! Grid to layout window components.
  std::unique_ptr<Fl_Grid> gridspace_{};
  //! Record-organization sidebar object (prototype).
  std::unique_ptr<Fl_Box> list_{};
  //! ConsoleSink debug log sink.
  std::shared_ptr<ConsoleSink_mt> sink{};
  //! spdlog log source
  std::shared_ptr<spdlog::logger> logger{};
  //! Terminal to display ConsoleSink formatted logs.
  std::unique_ptr<Fl_Terminal> debug_tty{};
  //! The AboutWindow.
  std::unique_ptr<AboutWindow> about_window_{};
  //! The Datasheet to display (spreadsheet of records).
  std::unique_ptr<Datasheet> datasheet_{};
  static void about_cb(Fl_Widget *menu, void *junk);
  static void quit_cb(Fl_Widget *menu, void *junk);
  //! Program name
  inline static const std::string
      // cppcheck-suppress unusedStructMember
      name_{"PsSp - Passive-source Seismic-processing"};
  static void prevent_escape(Fl_Widget *, void *);  // MwuaHaHaha
};
}  // namespace pssp

#endif

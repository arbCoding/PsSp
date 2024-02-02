// Copyright 2023-2024 Alexander R. Blanchette

/*!
  \file Windows/About.cpp
  \brief AboutWindow implementation.
  \author Alexander R. Blanchette
  This file contains the AboutWindow class implementation.
  */

#include "PsSp/Windows/About.hpp"

namespace pssp {
/*!
  \brief AboutWindow constructor.

  This creates the AboutWindow object with all the specified sizes from the
  pssp::about namespaces and centers the window.
  */
AboutWindow::AboutWindow() : Fl_Window(0, 0, 0, 0, "About") {
  this->begin();
  structs::Geometry geo{};
  Fl::screen_work_area(geo.x_pos, geo.y_pos, geo.width, geo.height);
  geo.x_pos = ((geo.width - about::width) / 2);
  geo.y_pos = ((geo.height - about::height) / 2);
  this->resize(geo.x_pos, geo.y_pos, about::width, about::height);
  this->box(FL_BORDER_BOX);
  set_modal();
  message = std::make_unique<Fl_Box>(about::width - about::text_width, 0,
                                     about::text_width, about::text_height);
  okay_button = std::make_unique<Fl_Return_Button>(
      (about::width - about::button_width) / 2, about::text_height,
      about::button_width, about::button_height, "Okay");
  message->label(message_.c_str());
  message->align(FL_ALIGN_CENTER);
  okay_button->callback(okay_cb);
  this->end();
}

/*!
  \brief AboutWindow.okay_button callback

  When the user choses to close the About window, the button tells the parent
  to hide (how FLTK handles closing a window).
 */
void AboutWindow::okay_cb(Fl_Widget *btn) { btn->parent()->hide(); }
}  // namespace pssp

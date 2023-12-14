// Copyright 2023 Alexander R. Blanchette

#include "Welcome.hpp"

namespace pssp {
Welcome_Window::Welcome_Window()
    : Fl_Window(hw_x, hw_y, hw_width, hw_height, "Welcome!") {
  this->begin();
  int x, y, w, h;
  Fl::screen_work_area(x, y, w, h);
  hw_x = ((w - hw_width) / 2);
  hw_y = ((h - hw_height) / 2);
  this->resize(hw_x, hw_y, hw_width, hw_height);
  this->box(FL_BORDER_BOX);
  set_modal();
  message.label(message_.c_str());
  continue_button.callback(continue_cb);
  this->end();
}

void Welcome_Window::continue_cb(Fl_Widget *btn) { btn->parent()->hide(); }
}  // namespace pssp

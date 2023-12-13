// Copyright 2023 Alexander R. Blanchette

#include "Welcome.hpp"

namespace pssp {
Welcome_Window::Welcome_Window()
    : Fl_Window(hw_x, hw_y, hw_width, hw_height, "Welcome!") {
  this->begin();
  this->box(FL_BORDER_BOX);
  set_modal();
  message.label(message_.c_str());
  continue_button.callback(continue_cb);
  this->end();
}

void Welcome_Window::continue_cb(Fl_Widget *btn) { btn->parent()->hide(); }
}  // namespace pssp

// Copyright 2023 Alexander R. Blanchette

#include "Welcome.hpp"

namespace pssp {
Welcome_Window::Welcome_Window() : Fl_Window(0, 0, 0, 0, "Welcome!") {
  this->begin();
  int x_start{};
  int y_start{};
  int width{};
  int height{};
  Fl::screen_work_area(x_start, y_start, width, height);
  x_start = ((width - welcome_width) / 2);
  y_start = ((height - welcome_height) / 2);
  this->resize(x_start, y_start, welcome_width, welcome_height);
  this->box(FL_BORDER_BOX);
  set_modal();
  message.label(message_.c_str());
  continue_button.callback(continue_cb);
  this->end();
}

void Welcome_Window::continue_cb(Fl_Widget *btn) { btn->parent()->hide(); }
}  // namespace pssp

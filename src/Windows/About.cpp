// Copyright 2023 Alexander R. Blanchette

#include "About.hpp"

namespace pssp {
About_Window::About_Window() : Fl_Window(0, 0, 0, 0, "About") {
  this->begin();
  int x_start{};
  int y_start{};
  int width{};
  int height{};
  Fl::screen_work_area(x_start, y_start, width, height);
  x_start = ((width - about::width) / 2);
  y_start = ((height - about::height) / 2);
  this->resize(x_start, y_start, about::width, about::height);
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

void About_Window::okay_cb(Fl_Widget *btn) { btn->parent()->hide(); }
}  // namespace pssp

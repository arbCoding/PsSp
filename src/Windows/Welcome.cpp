// Copyright 2023 Alexander R. Blanchette

#include "PsSp/Windows/Welcome.hpp"

namespace pssp {
Welcome_Window::Welcome_Window() : Fl_Window(0, 0, 0, 0, "Welcome!") {
  this->begin();
  int x_start{};
  int y_start{};
  int width{};
  int height{};
  Fl::screen_work_area(x_start, y_start, width, height);
  x_start = ((width - welcome::width) / 2);
  y_start = ((height - welcome::height) / 2);
  this->resize(x_start, y_start, welcome::width, welcome::height);
  this->box(FL_BORDER_BOX);
  set_modal();
  message =
      std::make_unique<Fl_Box>((welcome::width - welcome::text_width) / 2, 0,
                               welcome::text_width, welcome::text_height);
  continue_button = std::make_unique<Fl_Return_Button>(
      (welcome::width - welcome::button_width) / 2, welcome::text_height,
      welcome::button_width, welcome::button_height, "Continue");
  message->label(message_.c_str());
  message->align(FL_ALIGN_CENTER);
  continue_button->callback(continue_cb);
  this->end();
}

void Welcome_Window::continue_cb(Fl_Widget *btn) { btn->parent()->hide(); }
}  // namespace pssp

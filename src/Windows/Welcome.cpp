// Copyright 2023 Alexander R. Blanchette

#include "Welcome.hpp"

namespace pssp {
Welcome_Window::Welcome_Window() : Fl_Window(hw_width, hw_height, "Welcome!") {
  set_modal();
}
}  // namespace pssp

// Copyright 2023 Alexander R. Blanchette

#include "Main.hpp"

namespace pssp {
Main_Window::Main_Window() : Fl_Window(mw_width, mw_height, name_.c_str()) {
  resizable(this);
}
}  // namespace pssp

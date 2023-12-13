// Copyright 2023 Alexander R. Blanchette

// Program main window
#include "Application/Application.hpp"
/* fltk */
#include <FL/Fl.H>

int main() {
  pssp::Application app{};
  return Fl::run();
}

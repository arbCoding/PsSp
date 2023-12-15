// Copyright 2023 Alexander R. Blanchette

#include "Datasheet.hpp"

namespace pssp {
Datasheet::Datasheet() : Fl_Table(0, 0, 0, 0) {
  spdlog::trace("Making \033[1mDatasheet\033[0m.");
  this->begin();
  this->color(FL_CYAN);
  this->end();
  spdlog::trace("Done making \033[1mDatasheet\033[0m.");
}
}  // namespace pssp

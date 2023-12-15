// Copyright 2023 Alexander R. Blanchette

#include "Status_Bar.hpp"

namespace pssp {
Status_Bar::Status_Bar(int container_height, int width,
                       int height) :
  Fl_Grid(0, container_height - height, width, height) {
  spdlog::trace("Making \033[1mStatus_Bar\033[0m.");
  this->begin();
  this->layout(1, 10, 1, 1);
  left_box_ = std::make_unique<Fl_Box>(0, 0, 0, 0, "Left Box");
  left_box_->box(FL_BORDER_BOX);
  this->widget(left_box_.get(), 0, 0, 1, 2);
  middle_box_ = std::make_unique<Fl_Box>(0, 0, 0, 0, "Middle Box");
  middle_box_->box(FL_BORDER_BOX);
  this->widget(middle_box_.get(), 0, 2, 1, 6);
  right_box_ = std::make_unique<Fl_Box>(0, 0, 0, 0, "Right Box");
  right_box_->box(FL_BORDER_BOX);
  this->widget(right_box_.get(), 0, 8, 1, 2);
  this->end();
  spdlog::trace("Done making \033[1mStatus_Bar\033[0m.");
}
} // namespace pssp

// Copyright 2023 Alexander R. Blanchette

#include "PsSp/Widgets/Status_Bar.hpp"

namespace pssp {
Status_Bar::Status_Bar(int container_height, int width, int height)
    : Fl_Grid(0, container_height - height, width, height) {
  spdlog::trace("Making \033[1mStatus_Bar\033[0m.");
  this->begin();
  constexpr structs::Grid layout{1, 10, 1, 1};
  this->layout(layout.row, layout.col, layout.row_span, layout.col_span);
  left_box_ = std::make_unique<Fl_Box>(0, 0, 0, 0, "Left Box");
  left_box_->box(FL_BORDER_BOX);
  constexpr structs::Grid left{0, 0, 1, 2};
  this->widget(left_box_.get(), left.row, left.col, left.row_span,
               left.col_span);
  middle_box_ = std::make_unique<Fl_Box>(0, 0, 0, 0, "Middle Box");
  middle_box_->box(FL_BORDER_BOX);
  constexpr structs::Grid middle{0, 2, 1, 6};
  this->widget(middle_box_.get(), middle.row, middle.col, middle.row_span,
               middle.col_span);
  right_box_ = std::make_unique<Fl_Box>(0, 0, 0, 0, "Right Box");
  right_box_->box(FL_BORDER_BOX);
  constexpr structs::Grid right{0, 8, 1, 2};
  this->widget(right_box_.get(), right.row, right.col, right.row_span,
               right.col_span);
  this->end();
  spdlog::trace("Done making \033[1mStatus_Bar\033[0m.");
}
}  // namespace pssp

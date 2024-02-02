// Copyright 2023-2024 Alexander R. Blanchette

#ifndef PSSP_STATUSBAR_HPP_20231215_1002
#define PSSP_STATUSBAR_HPP_20231215_1002
#pragma once
// PsSp
#include "PsSp/Utility/Structs.hpp"
// fltk https://www.fltk.org/doc-1.4
#include <FL/Fl_Box.H>
#include <FL/Fl_Grid.H>
// spdlog https://github.com/gabime/spdlog
#include <spdlog/spdlog.h>
// Standard library
//   https://en.cppreference.com/w/cpp/standard_library
// std::unique_ptr
#include <memory>

namespace pssp {
class StatusBar : public Fl_Grid {
public:
  StatusBar(int container_height, int width, int height);

private:
  std::unique_ptr<Fl_Box> left_box_{};
  std::unique_ptr<Fl_Box> middle_box_{};
  std::unique_ptr<Fl_Box> right_box_{};
};
}  // namespace pssp

#endif

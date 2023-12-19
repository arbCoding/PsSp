// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_INPUTMANAGER_HPP_20231219_1323
#define PSSP_INPUTMANAGER_HPP_20231219_1323
#pragma once

// PsSp
#include "PsSp/Utility/Enums.hpp"
#include "PsSp/Utility/Structs.hpp"
// fltk https://www.fltk.org/doc-1.4
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
// spdlog https://github.com/gabime/spdlog
#include <spdlog/spdlog.h>
// Standard library
//   https://en.cppreference.com/w/cpp/standard_library
// std::unique_ptr
#include <memory>
// std::string
#include <string>

namespace pssp {
class InputManager {
public:
  InputManager();
  std::string value();
  void start_editing(const trace_info &info, const structs::Geometry &geo,
                     const std::string &input);
  void done_editing();
  [[nodiscard]] bool visible() const;
  static void input_cb(Fl_Widget *widget, void *input_manager) {
    (void)widget;
    reinterpret_cast<InputManager *>(input_manager)->modified = true;
  }
  void hide();
  void cleanup();
  // This allows us to communicate to the Datasheet that despite being hidden,
  // we've been modified
  // cppcheck-suppress unusedStructMember
  bool modified{false};

private:
  void clear();
  std::unique_ptr<Fl_Input> input_string{};
  std::unique_ptr<Fl_Int_Input> input_int{};
  std::unique_ptr<Fl_Float_Input> input_float{};
};
}  // namespace pssp

#endif

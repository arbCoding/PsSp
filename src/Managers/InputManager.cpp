// Copyright 2023 Alexander R. Blanchette

#include "PsSp/Managers/InputManager.hpp"

namespace pssp {
InputManager::InputManager() {
  input_string = std::make_unique<Fl_Input>(0, 0, 0, 0);
  input_int = std::make_unique<Fl_Int_Input>(0, 0, 0, 0);
  input_float = std::make_unique<Fl_Float_Input>(0, 0, 0, 0);
  hide();
  input_string->callback(input_cb, reinterpret_cast<void *>(this));
  input_int->callback(input_cb, reinterpret_cast<void *>(this));
  input_float->callback(input_cb, reinterpret_cast<void *>(this));
  input_string->when(FL_WHEN_ENTER_KEY_ALWAYS);
  input_int->when(FL_WHEN_ENTER_KEY_ALWAYS);
  input_float->when(FL_WHEN_ENTER_KEY_ALWAYS);
  input_string->maximum_size(40);
  input_int->maximum_size(40);
  input_float->maximum_size(40);
  input_string->color(FL_YELLOW);
  input_int->color(FL_RED);
  input_float->color(FL_GREEN);
}

bool InputManager::visible() const {
  return ((input_string->visible() != 0) || (input_int->visible() != 0) ||
          (input_float->visible() != 0));
}

void InputManager::cleanup() {
  clear();
  hide();
}

void InputManager::hide() {
  input_string->hide();
  input_int->hide();
  input_float->hide();
}

void InputManager::clear() {
  input_string->value("");
  input_int->value("");
  input_float->value("");
}

std::string InputManager::value() {
  std::string result{};
  // Which one is being used? They're empty after cleanup
  // so only the used one is full
  if (!std::string(input_string->value()).empty()) {
    result = input_string->value();
  } else if (!std::string(input_int->value()).empty()) {
    result = input_int->value();
  } else {
    result = input_float->value();
  }
  return result;
}

void InputManager::start_editing(const trace_info &info,
                                 const structs::Geometry &geo,
                                 const std::string &input) {
  if (info.type == Type::string_) {
    input_string->resize(geo.x_pos, geo.y_pos, geo.width, geo.height);
    input_string->value(input.c_str());
    input_string->insert_position(0, static_cast<int>(input.size()));
    input_string->show();
    input_string->take_focus();
  } else if (info.type == Type::int_) {
    input_int->resize(geo.x_pos, geo.y_pos, geo.width, geo.height);
    input_int->value(input.c_str());
    input_int->insert_position(0, static_cast<int>(input.size()));
    input_int->show();
    input_int->take_focus();
  } else if (info.type == Type::float_) {
    input_float->resize(geo.x_pos, geo.y_pos, geo.width, geo.height);
    input_float->value(input.c_str());
    input_float->insert_position(0, static_cast<int>(input.size()));
    input_float->show();
    input_float->take_focus();
  } else if (info.type == Type::double_) {
    input_float->resize(geo.x_pos, geo.y_pos, geo.width, geo.height);
    input_float->value(input.c_str());
    input_float->insert_position(0, static_cast<int>(input.size()));
    input_float->show();
    input_float->take_focus();
  } else if (info.type == Type::bool_) {
    input_string->resize(geo.x_pos, geo.y_pos, geo.width, geo.height);
    input_string->value(input.c_str());
    input_string->insert_position(0, static_cast<int>(input.size()));
    input_string->show();
    input_string->take_focus();
  }
}
}  // namespace pssp

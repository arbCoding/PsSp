// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_SHEETMANAGER_HPP_20231219_1042
#define PSSP_SHEETMANAGER_HPP_20231219_1042
#pragma once

// PsSp
#include "PsSp/Utility/Constants.hpp"
#include "PsSp/Utility/Enums.hpp"
// fltk hptts://www.fltk.org/doc-1.4
// spdlog https://github.com/gabime/spdlog
#include <spdlog/spdlog.h>
// Standard library
//   https://en.cppreference.com/w/cpp/standard_library
// std::array
#include <array>
// std::ostringstream
#include <sstream>
// std::string
#include <string>
// std::vector
#include <vector>

namespace pssp {
class SheetManager {
public:
  SheetManager();
  void resize_data(int size);
  [[nodiscard]] int rows() const;
  [[nodiscard]] int cols() const;
  // Setters
  void set(int row, const Field &field, const std::string &input);
  void set(int row, const Field &field, int input);
  void set(int row, const Field &field, float input);
  void set(int row, const Field &field, double input);
  void set(int row, const Field &field, bool input);
  // Getters
  // This does do conversion if necessary
  std::string get(int row, const Field &field);
  // These don't do conversions
  std::string get_string(int row, const Field &field);
  int get_int(int row, const Field &field);
  float get_float(int row, const Field &field);
  double get_double(int row, const Field &field);
  bool get_bool(int row, const Field &field);

private:
  // cppcheck-suppress unusedStructMember
  std::vector<std::array<std::string, constants::sac_string>> strings{};
  // cppcheck-suppress unusedStructMember
  std::vector<std::array<int, constants::sac_int>> ints{};
  // cppcheck-suppress unusedStructMember
  std::vector<std::array<float, constants::sac_float>> floats{};
  // cppcheck-suppress unusedStructMember
  std::vector<std::array<double, constants::sac_double>> doubles{};
  // cppcheck-suppress unusedStructMember
  std::vector<std::array<bool, constants::sac_bool>> bools{};
};
}  // namespace pssp

#endif

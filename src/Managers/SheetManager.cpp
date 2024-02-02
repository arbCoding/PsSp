// Copyright 2023-2024 Alexander R. Blanchette

#include "PsSp/Managers/SheetManager.hpp"

namespace pssp {
SheetManager::SheetManager() { resize_data(100); }

void SheetManager::resize_data(const size_t size) {
  strings.resize(size);
  ints.resize(size);
  floats.resize(size);
  doubles.resize(size);
  bools.resize(size);
}

int SheetManager::rows() const { return static_cast<int>(bools.size()); }

int SheetManager::cols() const {
  size_t num_cols{strings[0].size() + ints[0].size() + floats[0].size() +
                  doubles[0].size() + bools[0].size()};
  return static_cast<int>(num_cols);
}

void SheetManager::set(const size_t row, const Field &field,
                       const std::string &input) {
  const trace_info &info{field_info.at(field)};
  if (info.type == Type::string_) {
    strings[row][info.array_col] = input;
  } else {
    spdlog::error("Wrong type {0} inserted into field {1}.",
                  type_names.at(info.type), info.name);
  }
}

void SheetManager::set(const size_t row, const Field &field, const int input) {
  const trace_info &info{field_info.at(field)};
  if (info.type == Type::int_) {
    ints[row][info.array_col] = input;
  } else {
    spdlog::error("Wrong type {0} inserted into field {1}.",
                  type_names.at(info.type), info.name);
  }
}

void SheetManager::set(const size_t row, const Field &field,
                       const float input) {
  const trace_info &info{field_info.at(field)};
  if (info.type == Type::float_) {
    floats[row][info.array_col] = input;
  } else {
    spdlog::error("Wrong type {0} inserted into field {1}.",
                  type_names.at(info.type), info.name);
  }
}

void SheetManager::set(const size_t row, const Field &field,
                       const double input) {
  const trace_info &info{field_info.at(field)};
  if (info.type == Type::double_) {
    doubles[row][info.array_col] = input;
  } else {
    spdlog::error("Wrong type {0} inserted into field {1}.",
                  type_names.at(info.type), info.name);
  }
}

void SheetManager::set(const size_t row, const Field &field, const bool input) {
  const trace_info &info{field_info.at(field)};
  if (info.type == Type::bool_) {
    bools[row][info.array_col] = input;
  } else {
    spdlog::error("Wrong type {0} inserted into field {1}.",
                  type_names.at(info.type), info.name);
  }
}

std::string SheetManager::get(const size_t row, const Field &field) {
  std::string result{};
  const trace_info &info{field_info.at(field)};
  switch (info.type) {
  case Type::string_:
    break;
  case Type::int_:
    result = std::to_string(ints[row][info.array_col]);
    break;
  case Type::float_:
    result = std::to_string(floats[row][info.array_col]);
    break;
  case Type::double_:
    result = std::to_string(doubles[row][info.array_col]);
    break;
  case Type::bool_:
    result = std::to_string(static_cast<int>(bools[row][info.array_col]));
    break;
  default:
    break;
  }
  return result;
}

std::string SheetManager::get_string(const size_t row, const Field &field) {
  std::string result{};
  const trace_info &info{field_info.at(field)};
  if (info.type == Type::string_) {
    result = strings[row][info.array_col];
  } else {
    spdlog::error("Field {0} wrong type {1} for get_string.", info.name,
                  type_names.at(info.type));
  }
  return result;
}

int SheetManager::get_int(const size_t row, const Field &field) {
  int result{};
  const trace_info &info{field_info.at(field)};
  if (info.type == Type::int_) {
    result = ints[row][info.array_col];
  } else {
    spdlog::error("Field {0} wrong type {1} for get_string.", info.name,
                  type_names.at(info.type));
  }
  return result;
}

float SheetManager::get_float(const size_t row, const Field &field) {
  float result{};
  const trace_info &info{field_info.at(field)};
  if (info.type == Type::float_) {
    result = floats[row][info.array_col];
  } else {
    spdlog::error("Field {0} wrong type {1} for get_string.", info.name,
                  type_names.at(info.type));
  }
  return result;
}

double SheetManager::get_double(const size_t row, const Field &field) {
  double result{};
  const trace_info &info{field_info.at(field)};
  if (info.type == Type::double_) {
    result = doubles[row][info.array_col];
  } else {
    spdlog::error("Field {0} wrong type {1} for get_string.", info.name,
                  type_names.at(info.type));
  }
  return result;
}

bool SheetManager::get_bool(const size_t row, const Field &field) {
  bool result{};
  const trace_info &info{field_info.at(field)};
  if (info.type == Type::bool_) {
    result = bools[row][info.array_col];
  } else {
    spdlog::error("Field {0} wrong type {1} for get_string.", info.name,
                  type_names.at(info.type));
  }
  return result;
}
}  // namespace pssp

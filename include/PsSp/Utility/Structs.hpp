// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_STRUCTS_HPP_20231217_0600
#define PSSP_STRUCTS_HPP_20231217_0600
#pragma once

namespace pssp::structs {
struct Geometry {
  // cppcheck-suppress unusedStructMember
  int x_pos{0};
  // cppcheck-suppress unusedStructMember
  int y_pos{0};
  // cppcheck-suppress unusedStructMember
  int width{0};
  // cppcheck-suppress unusedStructMember
  int height{0};
};

struct Grid {
  // cppcheck-suppress unusedStructMember
  int row{0};
  // cppcheck-suppress unusedStructMember
  int col{0};
  // cppcheck-suppress unusedStructMember
  int row_span{0};
  // cppcheck-suppress unusedStructMember
  int col_span{0};
};
}  // namespace pssp::structs

#endif

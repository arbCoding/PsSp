// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_DATASHEET_HPP_20231215_1255
#define PSSP_DATASHEET_HPP_20231215_1255
#pragma once

/* fltk https://www.fltk.org/doc-1.4 */
#include <FL/Fl.H>
#include <FL/Fl_Table.H>
#include <FL/fl_draw.H>
/* spdlog https://github.com/gabime/spdlog */
#include <spdlog/spdlog.h>
/* Standard library
   https://en.cppreference.com/w/cpp/standard_library */
// std::ostringstream
#include <sstream>
// std::string
#include <string>

namespace pssp {
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

struct Cell {
  // cppcheck-suppress unusedStructMember
  Geometry full_box{};
  // cppcheck-suppress unusedStructMember
  Geometry text_box{};
  Fl_Font font{FL_HELVETICA};
  Fl_Color text_color{FL_BLACK};
  Fl_Color box_color{FL_GRAY};
  Fl_Boxtype box_type{FL_THIN_UP_BOX};
  Fl_Align alignment{FL_ALIGN_CENTER};
  // cppcheck-suppress unusedStructMember
  std::string text{""};
};

class Datasheet : public Fl_Table {
public:
  Datasheet();

protected:
  void draw_cell(TableContext context, int row = 0, int col = 0, int x_pos = 0,
                 int y_pos = 0, int width = 0, int height = 0) FL_OVERRIDE;

private:
  void draw_generic_cell(const Cell &cell);
  void draw_header_cell(int x_pos, int y_pos, int width, int height,
                        const std::string &text);
};
}  // namespace pssp

#endif

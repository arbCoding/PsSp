// Copyright 2023 Alexander R. Blanchette

#ifndef PSSP_DATASHEET_HPP_20231215_1255
#define PSSP_DATASHEET_HPP_20231215_1255
#pragma once

/* fltk https://www.fltk.org/doc-1.4 */
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Table.H>
/* spdlog https://github.com/gabime/spdlog */
#include <spdlog/spdlog.h>
/* Standard library
   https://en.cppreference.com/w/cpp/standard_library */
// std::unique_ptr
//#include <memory>
// std::ostringstream
#include <sstream>

namespace pssp {
class Datasheet : public Fl_Table {
public:
  Datasheet();
protected:
  void draw_cell(TableContext context, int row = 0, int col = 0, int x_pos = 0,
                 int y_pos = 0, int width = 0, int height = 0) FL_OVERRIDE;
private:
  void draw_col_header_cell(int col, int x_pos, int y_pos,
                            int width, int height);
  void draw_row_header_cell(int row, int x_pos, int y_pos,
                            int width, int height);
  void draw_interior_cell(int row, int col, int x_pos, int y_pos,
                          int width, int height);
};
} // namespace pssp

#endif

// Copyright 2023 Alexander R. Blanchette

#include "Datasheet.hpp"

namespace pssp {
Datasheet::Datasheet() : Fl_Table(0, 0, 0, 0) {
  spdlog::trace("Making \033[1mDatasheet\033[0m.");
  this->begin();
  //this->color(FL_CYAN);
  tab_cell_nav(1); // enable tab navigation
  tooltip("Use keyboard to navigate cells:\n"
          "Arrow keys or Tab/Shift-Tab");
  row_header(1);
  row_header_width(70);
  row_resize(1);
  rows(27);
  row_height_all(25);
  col_header(1);
  col_header_height(25);
  col_resize(1);
  cols(21);
  col_width_all(70);
  this->end();
  spdlog::trace("Done making \033[1mDatasheet\033[0m.");
}

void Datasheet::draw_col_header_cell(int col, int x_pos, int y_pos,
                                     int width, int height) {
  fl_font(FL_HELVETICA | FL_BOLD, 14);
  fl_push_clip(x_pos, y_pos, width, height);
  fl_draw_box(FL_THIN_UP_BOX, x_pos, y_pos, width, height, col_header_color());
  fl_color(FL_BLACK);
  static std::ostringstream oss{};
  oss << 'A';
  oss << col;
  fl_draw(oss.str().c_str(), x_pos, y_pos, width, height, FL_ALIGN_CENTER);
  oss.str("");
  oss.clear();
  fl_pop_clip();
}

void Datasheet::draw_row_header_cell(int row, int x_pos, int y_pos,
                                     int width, int height) {
  fl_font(FL_HELVETICA | FL_BOLD, 14);
  fl_push_clip(x_pos, y_pos, width, height);
  fl_draw_box(FL_THIN_UP_BOX, x_pos, y_pos, width, height, row_header_color());
  fl_color(FL_BLACK);
  static std::ostringstream oss{};
  oss << row + 1;
  fl_draw(oss.str().c_str(), x_pos, y_pos, width, height, FL_ALIGN_CENTER);
  oss.str("");
  oss.clear();
  fl_pop_clip();
}

void Datasheet::draw_interior_cell(int row, int col, int x_pos, int y_pos,
                                   int width, int height) {
  // Background
  fl_draw_box(FL_THIN_UP_BOX, x_pos, y_pos, width, height,
              is_selected(row, col) ? FL_YELLOW : FL_WHITE);
  // Text
  fl_push_clip(x_pos + 3, y_pos + 3, width - 6, height - 6);
  fl_color(FL_BLACK);
  fl_font(FL_HELVETICA, 14);
  static std::ostringstream oss{};
  oss << 100;
  fl_draw(oss.str().c_str(), x_pos + 3, y_pos + 3, width - 6, height - 6,
          FL_ALIGN_RIGHT);
  oss.str("");
  oss.clear();
  fl_pop_clip();
}

void Datasheet::draw_cell(TableContext context, int row, int col, int x_pos,
                          int y_pos, int width, int height) {
  switch (context) {
    case CONTEXT_STARTPAGE:  // table about to redraw
      break;
    case CONTEXT_COL_HEADER:  // table wants us to draw a column heading
      draw_col_header_cell(col, x_pos, y_pos, width, height);
      return;
    case CONTEXT_ROW_HEADER: // table wants us to draw a row heading
      draw_row_header_cell(row, x_pos, y_pos, width, height);
      return;
    case CONTEXT_CELL:
      draw_interior_cell(row, col, x_pos, y_pos, width, height);
      return;
    case CONTEXT_RC_RESIZE:
      return;
    default:
      return;
  }
}
} // namespace pssp

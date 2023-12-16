// Copyright 2023 Alexander R. Blanchette

#include "Datasheet.hpp"

namespace pssp {
Datasheet::Datasheet() : Fl_Table(0, 0, 0, 0) {
  spdlog::trace("Making \033[1mDatasheet\033[0m.");
  this->begin();
  tab_cell_nav(1);  // enable tab navigation
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

void Datasheet::draw_generic_cell(const Cell &cell) {
  fl_font(cell.font, 14);
  fl_draw_box(cell.box_type, cell.full_box.x_pos, cell.full_box.y_pos,
              cell.full_box.width, cell.full_box.height, cell.box_color);
  fl_push_clip(cell.text_box.x_pos, cell.text_box.y_pos, cell.text_box.width,
               cell.text_box.height);
  fl_color(cell.text_color);
  fl_draw(cell.text.c_str(), cell.text_box.x_pos, cell.text_box.y_pos,
          cell.text_box.width, cell.text_box.height, cell.alignment);
  fl_pop_clip();
}

void Datasheet::draw_header_cell(const int x_pos, const int y_pos,
                                 const int width, const int height,
                                 const std::string &text) {
  Cell cell{};
  cell.full_box = {x_pos, y_pos, width, height};
  cell.text_box = cell.full_box;
  cell.font = FL_HELVETICA | FL_BOLD;
  cell.text = text;
  draw_generic_cell(cell);
}

void Datasheet::draw_cell(const TableContext context, const int row,
                          const int col, const int x_pos, const int y_pos,
                          const int width, const int height) {
  switch (context) {
  case CONTEXT_COL_HEADER: {
    std::ostringstream oss{};
    oss << 'A';
    oss << col;
    draw_header_cell(x_pos, y_pos, width, height, oss.str());
  } break;
  case CONTEXT_ROW_HEADER:
    draw_header_cell(x_pos, y_pos, width, height, std::to_string(row + 1));
    break;
  case CONTEXT_CELL: {
    Cell cell{};
    cell.full_box = {x_pos, y_pos, width, height};
    cell.text_box = {x_pos + 3, y_pos + 3, width - 6, height - 6};
    // Eventually there will be logic to generate this value
    cell.text = "100";
    cell.box_color = (is_selected(row, col) ? FL_YELLOW : FL_WHITE);
    cell.alignment = FL_ALIGN_RIGHT;
    draw_generic_cell(cell);
  } break;
  default:
    return;
  }
}
}  // namespace pssp

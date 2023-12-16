// Copyright 2023 Alexander R. Blanchette

#include "Datasheet.hpp"

namespace pssp {
Datasheet::Datasheet() : Fl_Table(0, 0, 0, 0) {
  spdlog::trace("Making \033[1mDatasheet\033[0m.");
  // trick to use event_callback2
  callback(&event_callback, reinterpret_cast<void *>(this));
  this->begin();
  this->when(FL_WHEN_NOT_CHANGED | when());
  input = std::make_unique<Fl_Int_Input>(0, 0, 0, 0);
  // Hide until needed
  input->hide();
  input->callback(input_cb, reinterpret_cast<void *>(this));
  // callback trigger condition
  input->when(FL_WHEN_ENTER_KEY_ALWAYS);
  input->maximum_size(5);
  input->color(FL_YELLOW);
  tab_cell_nav(1);  // enable tab navigation
  tooltip("Use keyboard to navigate cells:\n"
          "Arrow keys or Tab/Shift-Tab");
  for (int col{0}; col < datasheet::max_col; ++col) {
    for (int row{0}; row < datasheet::max_row; ++row) {
      values[row][col] = col + (row * datasheet::max_row);
    }
  }
  row_header(1);
  row_header_width(70);
  row_resize(1);
  rows(datasheet::max_row);
  row_height_all(25);
  col_header(1);
  col_header_height(25);
  col_resize(1);
  cols(datasheet::max_col);
  col_width_all(70);
  set_selection(0, 0, 0, 0);
  this->end();
  spdlog::trace("Done making \033[1mDatasheet\033[0m.");
}

void Datasheet::set_value_hide() {
  values[edit_row][edit_col] = std::stoi(input->value());
  input->hide();
  window()->cursor(FL_CURSOR_DEFAULT);  // deals with disappearing cursor
}

void Datasheet::start_editing(int row, int col) {
  edit_row = row;
  edit_col = col;
  set_selection(row, col, row, col);
  int x_pos{};
  int y_pos{};
  int width{};
  int height{};
  find_cell(CONTEXT_CELL, row, col, x_pos, y_pos, width, height);
  input->resize(x_pos, y_pos, width, height);
  std::ostringstream oss{};
  oss << values[row][col];
  input->value(oss.str().c_str());
  input->insert_position(0, static_cast<int>(oss.str().size()));
  input->show();
  input->take_focus();
}

void Datasheet::done_editing() {
  if (input->visible()) {
    set_value_hide();
    edit_row = 0;
    edit_col = 0;
  }
}

void Datasheet::draw_generic_cell(const datasheet::Cell &cell) {
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
  datasheet::Cell cell{};
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
    datasheet::Cell cell{};
    cell.full_box = {x_pos, y_pos, width, height};
    cell.text_box = {x_pos + 3, y_pos + 3, width - 6, height - 6};
    // Eventually there will be logic to generate this value
    cell.text = std::to_string(values[row][col]);
    cell.box_color = (is_selected(row, col) ? FL_YELLOW : FL_WHITE);
    cell.alignment = FL_ALIGN_RIGHT;
    draw_generic_cell(cell);
  } break;
  default:
    return;
  }
}

// Gets called when someone clicks somewhere on the table
// This blocks shortcuts from reaching the main app if it is the focus
// This needs to be broken up into more organized functions
void Datasheet::event_callback2() {
  int row{callback_row()};
  int col{callback_col()};
  TableContext context{callback_context()};
  switch (context) {
  case CONTEXT_CELL: {
    switch (Fl::event()) {
    case FL_PUSH:
      start_editing(row, col);
      break;
    case FL_KEYBOARD:
      done_editing();
      if (!((Fl::event() == FL_SHORTCUT) && (Fl::event_key() == FL_Escape))) {
        if (datasheet::edit_chars.find(Fl::e_text[0]) != std::string::npos) {
          start_editing(row, col);
        }
      }
      break;
    }
  } break;
  case CONTEXT_TABLE:
  case CONTEXT_ROW_HEADER:
  case CONTEXT_COL_HEADER:
    done_editing();
    break;
  default:
    return;
  }
}
}  // namespace pssp

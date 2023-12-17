// Copyright 2023 Alexander R. Blanchette

#include "PsSp/Widgets/Datasheet.hpp"

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
  input->maximum_size(datasheet::max_chars);
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
  row_header_width(datasheet::spec.header_width);
  row_height_all(datasheet::spec.height);
  rows(datasheet::max_row);
  col_header(1);
  col_header_height(datasheet::spec.header_height);
  col_width_all(datasheet::spec.width);
  cols(datasheet::max_col);
  row_resize(1);
  col_resize(1);
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
  structs::Geometry geo{};
  find_cell(CONTEXT_CELL, row, col, geo.x_pos, geo.y_pos, geo.width,
            geo.height);
  input->resize(geo.x_pos, geo.y_pos, geo.width, geo.height);
  std::ostringstream oss{};
  oss << values[row][col];
  input->value(oss.str().c_str());
  input->insert_position(0, static_cast<int>(oss.str().size()));
  input->show();
  input->take_focus();
}

void Datasheet::done_editing() {
  if (input->visible() != 0) {
    set_value_hide();
    edit_row = 0;
    edit_col = 0;
  }
}

void Datasheet::draw_generic_cell(const datasheet::Cell &cell) {
  fl_font(cell.font, datasheet::font_size);
  fl_draw_box(cell.box_type, cell.full_box.x_pos, cell.full_box.y_pos,
              cell.full_box.width, cell.full_box.height, cell.box_color);
  fl_push_clip(cell.text_box.x_pos, cell.text_box.y_pos, cell.text_box.width,
               cell.text_box.height);
  fl_color(cell.text_color);
  fl_draw(cell.text.c_str(), cell.text_box.x_pos, cell.text_box.y_pos,
          cell.text_box.width, cell.text_box.height, cell.alignment);
  fl_pop_clip();
}

void Datasheet::draw_header_cell(structs::Geometry *geo,
                                 const std::string &text) {
  datasheet::Cell cell{};
  cell.full_box = *geo;
  cell.text_box = cell.full_box;
  cell.font = FL_HELVETICA | FL_BOLD;
  cell.text = text;
  draw_generic_cell(cell);
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
void Datasheet::draw_cell(const TableContext context, const int row,
                          const int col, const int x_pos, const int y_pos,
                          const int width, const int height) {
  // NOLINTEND(bugprone-easily-swappable-parameters)
  switch (context) {
  case CONTEXT_COL_HEADER: {
    structs::Geometry geo{x_pos, y_pos, width, height};
    std::ostringstream oss{};
    oss << 'A';
    oss << col;
    draw_header_cell(&geo, oss.str());
  } break;
  case CONTEXT_ROW_HEADER: {
    structs::Geometry geo{x_pos, y_pos, width, height};
    draw_header_cell(&geo, std::to_string(row + 1));
  } break;
  case CONTEXT_CELL: {
    datasheet::Cell cell{};
    cell.full_box = {x_pos, y_pos, width, height};
    cell.text_box = {x_pos + datasheet::cell_buffer,
                     y_pos + datasheet::cell_buffer,
                     width - (2 * datasheet::cell_buffer),
                     height - (2 * datasheet::cell_buffer)};
    // Eventually there will be logic to generate this value
    cell.text = std::to_string(values[row][col]);
    cell.box_color = ((is_selected(row, col) != 0) ? FL_YELLOW : FL_WHITE);
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
      if (Fl::event() != FL_SHORTCUT || Fl::event_key() != FL_Escape) {
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

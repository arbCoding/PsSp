// Copyright 2023 Alexander R. Blanchette

#include "PsSp/Widgets/Datasheet.hpp"

namespace pssp {
Datasheet::Datasheet() : Fl_Table(0, 0, 0, 0) {
  spdlog::trace("Making \033[1mDatasheet\033[0m.");
  // trick to use event_callback2
  callback(&event_callback, reinterpret_cast<void *>(this));
  this->begin();
  this->when(FL_WHEN_NOT_CHANGED | this->when());
  input_manager = std::make_unique<InputManager>();
  this->tab_cell_nav(1);  // enable tab navigation
  tooltip("Use keyboard to navigate cells:\n"
          "Arrow keys or Tab/Shift-Tab");
  sheet_manager = std::make_unique<SheetManager>();
  check_button = std::make_unique<Fl_Check_Button>(0, 0, 0, 0);
  check_button->hide();
  max_col = sheet_manager->cols();
  max_row = sheet_manager->rows();
  constexpr datasheet::Spec spec{25, 25, 25, 70};
  row_header(1);
  row_header_width(spec.header_width);
  row_height_all(spec.height);
  rows(max_row);
  col_header(1);
  col_header_height(spec.header_height);
  col_width_all(spec.width);
  cols(max_col);
  row_resize(1);
  col_resize(1);
  set_selection(0, 0, 0, 0);
  this->end();
  spdlog::trace("Done making \033[1mDatasheet\033[0m.");
}

// This function likely does not need the logic for booleans once
// check_box is functional
//
// The type checking logic should be hidden in SheetManager
// For now just get functional
// Since we know the value is always a std::string...
// Also, on empty value, the values should be their unset versions defined
// by the SAC standard (later)
void Datasheet::set_value_hide() {
  const Field &field{field_num.at(edit_col)};
  const trace_info &info{field_info.at(field)};
  switch (info.type) {
  case Type::string_:
    sheet_manager->set(edit_row, field, input_manager->value());
    break;
  case Type::int_:
    if (!input_manager->value().empty()) {
      sheet_manager->set(edit_row, field, std::stoi(input_manager->value()));
    } else {
      sheet_manager->set(edit_row, field, 0);
    }
    break;
  case Type::float_:
    if (!input_manager->value().empty()) {
      sheet_manager->set(edit_row, field, std::stof(input_manager->value()));
    } else {
      sheet_manager->set(edit_row, field, 0.0F);
    }
    break;
  case Type::double_:
    if (!input_manager->value().empty()) {
      sheet_manager->set(edit_row, field, std::stod(input_manager->value()));
    } else {
      sheet_manager->set(edit_row, field, 0.0);
    }
    break;
  case Type::bool_:
    // This is just junk for prototyping
    sheet_manager->set(edit_row, field, !input_manager->value().empty());
    break;
  default:
    break;
  }
  input_manager->cleanup();
  input_manager->modified = false;
  window()->cursor(FL_CURSOR_DEFAULT);  // deals with disappearing cursor
}

// this function needs logic to handle check_button (or to not handle
// bools at all here)
void Datasheet::start_editing(int row, int col) {
  edit_row = row;
  edit_col = col;
  set_selection(row, col, row, col);
  structs::Geometry geo{};
  find_cell(CONTEXT_CELL, row, col, geo.x_pos, geo.y_pos, geo.width,
            geo.height);
  // Need to refactor
  const Field &field{field_num.at(col)};
  const trace_info &info{field_info.at(field)};
  if (info.type == Type::string_) {
    input_manager->start_editing(info, geo,
                                 sheet_manager->get_string(row, field));
  } else if (info.type == Type::int_) {
    std::ostringstream oss{};
    oss << sheet_manager->get_int(row, field);
    input_manager->start_editing(info, geo, oss.str());
  } else if (info.type == Type::float_) {
    std::ostringstream oss{};
    oss << sheet_manager->get_float(row, field);
    input_manager->start_editing(info, geo, oss.str());
  } else if (info.type == Type::double_) {
    std::ostringstream oss{};
    oss << sheet_manager->get_double(row, field);
    input_manager->start_editing(info, geo, oss.str());
  } else if (info.type == Type::bool_) {
    std::ostringstream oss{};
    oss << sheet_manager->get_bool(row, field);
    input_manager->start_editing(info, geo, oss.str());
  }
}

void Datasheet::done_editing() {
  if (input_manager->visible() || input_manager->modified) {
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

// Need to refactor
// This function needs logic to handle check_button

// Prior to check_button logic, make a new function
// make_cell that does all the stuff in the CONTEXT_CELL portion
// then it'll be easier to break out logic for check_button
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
void Datasheet::draw_cell(const TableContext context, const int row,
                          const int col, const int x_pos, const int y_pos,
                          const int width, const int height) {
  // NOLINTEND(bugprone-easily-swappable-parameters)
  switch (context) {
  case CONTEXT_COL_HEADER: {
    structs::Geometry geo{x_pos, y_pos, width, height};
    draw_header_cell(&geo, field_info.at(field_num.at(col)).name);
  } break;
  case CONTEXT_ROW_HEADER: {
    structs::Geometry geo{x_pos, y_pos, width, height};
    draw_header_cell(&geo, std::to_string(row + 1));
  } break;
  case CONTEXT_CELL: {
    // This needs to be refactored
    datasheet::Cell cell{};
    cell.full_box = {x_pos, y_pos, width, height};
    cell.text_box = {x_pos + datasheet::cell_buffer,
                     y_pos + datasheet::cell_buffer,
                     width - (2 * datasheet::cell_buffer),
                     height - (2 * datasheet::cell_buffer)};
    const Field &field{field_num.at(col)};
    const trace_info &info{field_info.at(field)};
    if (info.type == Type::string_) {
      cell.text = sheet_manager->get_string(row, field);
    } else if (info.type == Type::int_) {
      std::ostringstream oss{};
      oss << sheet_manager->get_int(row, field);
      cell.text = oss.str();
    } else if (info.type == Type::float_) {
      std::ostringstream oss{};
      oss << sheet_manager->get_float(row, field);
      cell.text = oss.str();
    } else if (info.type == Type::double_) {
      std::ostringstream oss{};
      oss << sheet_manager->get_double(row, field);
      cell.text = oss.str();
    } else if (info.type == Type::bool_) {
      std::ostringstream oss{};
      oss << sheet_manager->get_bool(row, field);
      cell.text = oss.str();
    }
    cell.box_color = ((is_selected(row, col) != 0) ? FL_YELLOW : FL_WHITE);
    cell.alignment = FL_ALIGN_RIGHT;
    draw_generic_cell(cell);
  } break;
  default:
    return;
  }
}

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
      if (Fl::event_state() == FL_COMMAND) {
        parent()->take_focus();
      } else if (datasheet::edit_chars.find(Fl::e_text[0]) !=
                 std::string::npos) {
        start_editing(row, col);
      }
      break;
    default:
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

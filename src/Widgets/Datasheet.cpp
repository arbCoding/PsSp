// Copyright 2023 Alexander R. Blanchette

#include "PsSp/Widgets/Datasheet.hpp"

namespace pssp {
Datasheet::Datasheet() : Fl_Table(0, 0, 0, 0) {
  spdlog::trace("Making \033[1mDatasheet\033[0m.");
  // trick to use event_callback2
  callback(&event_callback, reinterpret_cast<void *>(this));
  this->begin();
  this->when(FL_WHEN_NOT_CHANGED | this->when());
  make_inputs();
  this->tab_cell_nav(1);  // enable tab navigation
  tooltip("Use keyboard to navigate cells:\n"
          "Arrow keys or Tab/Shift-Tab");
  make_sheet();
  constexpr datasheet::Spec spec{25, 25, 25, 70};
  row_header(1);
  row_header_width(spec.header_width);
  row_height_all(spec.height);
  rows(datasheet::max_row);
  col_header(1);
  col_header_height(spec.header_height);
  col_width_all(spec.width);
  cols(datasheet::max_col);
  row_resize(1);
  col_resize(1);
  set_selection(0, 0, 0, 0);
  this->end();
  spdlog::trace("Done making \033[1mDatasheet\033[0m.");
}

// I ought to be able to deal with this using templates.
void Datasheet::make_inputs() {
  // Make inputs
  input = std::make_unique<Fl_Input>(0, 0, 0, 0);
  input_int = std::make_unique<Fl_Int_Input>(0, 0, 0, 0);
  input_float = std::make_unique<Fl_Float_Input>(0, 0, 0, 0);
  // Hide all inputs until needed
  input->hide();
  input_int->hide();
  input_float->hide();
  // Assign callbacks
  input->callback(input_cb, reinterpret_cast<void *>(this));
  input_int->callback(input_cb, reinterpret_cast<void *>(this));
  input_float->callback(input_cb, reinterpret_cast<void *>(this));
  // callback trigger condition
  input->when(FL_WHEN_ENTER_KEY_ALWAYS);
  input_int->when(FL_WHEN_ENTER_KEY_ALWAYS);
  input_float->when(FL_WHEN_ENTER_KEY_ALWAYS);
  // Size limits
  input->maximum_size(datasheet::max_chars);
  input_int->maximum_size(datasheet::max_chars);
  input_float->maximum_size(datasheet::max_chars);
  // Input color
  input->color(FL_YELLOW);
  input_int->color(FL_YELLOW);
  input_float->color(FL_YELLOW);
}

void Datasheet::make_string_column(const trace_info &info, const int col) {
  for (int row{0}; row < constants::sac_string; ++row) {
    std::ostringstream oss{};
    oss << col + (row * datasheet::max_col);
    values_string[row][info.array_col] = oss.str();
  }
}

void Datasheet::make_int_column(const trace_info &info, const int col) {
  for (int row{0}; row < constants::sac_int; ++row) {
    values_int[row][info.array_col] = col + (row * datasheet::max_col);
  }
}

void Datasheet::make_float_column(const trace_info &info, const int col) {
  for (int row{0}; row < constants::sac_float; ++row) {
    values_float[row][info.array_col] =
        static_cast<float>(col + (row * datasheet::max_col));
  }
}

void Datasheet::make_double_column(const trace_info &info, const int col) {
  for (int row{0}; row < constants::sac_double; ++row) {
    values_double[row][info.array_col] =
        static_cast<double>(col + (row * datasheet::max_col));
  }
}

void Datasheet::make_bool_column(const trace_info &info, const int col) {
  (void)col;
  for (int row{0}; row < constants::sac_bool; ++row) {
    values_bool[row][info.array_col] = false;
  }
}

void Datasheet::make_generic_column(const int col) {
  const trace_info &info{field_info.at(field_num.at(col))};
  if (info.type == Type::string_) {
    make_string_column(info, col);
  } else if (info.type == Type::int_) {
    make_int_column(info, col);
  } else if (info.type == Type::float_) {
    make_float_column(info, col);
  } else if (info.type == Type::double_) {
    make_double_column(info, col);
  } else if (info.type == Type::bool_) {
    make_bool_column(info, col);
  }
}

void Datasheet::make_sheet() {
  for (int col{0}; col < datasheet::max_col; ++col) {
    make_generic_column(col);
  }
}

void Datasheet::set_value_hide() {
  const trace_info &info{field_info.at(field_num.at(edit_col))};
  switch (info.type) {
  case Type::string_:
    values_string[edit_row][info.array_col] = input->value();
    input->hide();
    break;
  case Type::int_:
    values_int[edit_row][info.array_col] = std::stoi(input_int->value());
    input_int->hide();
    break;
  case Type::float_:
    values_float[edit_row][info.array_col] = std::stof(input_float->value());
    input_float->hide();
    break;
  case Type::double_:
    values_double[edit_row][info.array_col] = std::stod(input_float->value());
    input_float->hide();
    break;
  case Type::bool_:
    // This is just junk for prototyping
    values_bool[edit_row][info.array_col] = (input->value() != nullptr);
    input->hide();
    break;
  default:
    break;
  }
  window()->cursor(FL_CURSOR_DEFAULT);  // deals with disappearing cursor
}

void Datasheet::start_editing(int row, int col) {
  edit_row = row;
  edit_col = col;
  set_selection(row, col, row, col);
  structs::Geometry geo{};
  find_cell(CONTEXT_CELL, row, col, geo.x_pos, geo.y_pos, geo.width,
            geo.height);
  // Need to refactor
  const trace_info &info{field_info.at(field_num.at(col))};
  if (info.type == Type::string_) {
    input->resize(geo.x_pos, geo.y_pos, geo.width, geo.height);
    input->value(values_string[row][info.array_col].c_str());
    input->insert_position(
        0, static_cast<int>(values_string[row][info.array_col].size()));
    input->show();
    input->take_focus();
  } else if (info.type == Type::int_) {
    input_int->resize(geo.x_pos, geo.y_pos, geo.width, geo.height);
    std::ostringstream oss{};
    oss << values_int[row][info.array_col];
    input_int->value(oss.str().c_str());
    input_int->insert_position(0, static_cast<int>(oss.str().size()));
    input_int->show();
    input_int->take_focus();
  } else if (info.type == Type::float_) {
    input_float->resize(geo.x_pos, geo.y_pos, geo.width, geo.height);
    std::ostringstream oss{};
    oss << values_float[row][info.array_col];
    input_float->value(oss.str().c_str());
    input_float->insert_position(0, static_cast<int>(oss.str().size()));
    input_float->show();
    input_float->take_focus();
  } else if (info.type == Type::double_) {
    input_float->resize(geo.x_pos, geo.y_pos, geo.width, geo.height);
    std::ostringstream oss{};
    oss << values_double[row][info.array_col];
    input_float->value(oss.str().c_str());
    input_float->insert_position(0, static_cast<int>(oss.str().size()));
    input_float->show();
    input_float->take_focus();
  } else if (info.type == Type::bool_) {
    input->resize(geo.x_pos, geo.y_pos, geo.width, geo.height);
    std::ostringstream oss{};
    oss << values_bool[row][info.array_col];
    input->value(oss.str().c_str());
    input->insert_position(0, static_cast<int>(oss.str().size()));
    input->show();
    input->take_focus();
  }
}

void Datasheet::done_editing() {
  if (input->visible() != 0 || input_int->visible() != 0 ||
      input_float->visible() != 0) {
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
    datasheet::Cell cell{};
    cell.full_box = {x_pos, y_pos, width, height};
    cell.text_box = {x_pos + datasheet::cell_buffer,
                     y_pos + datasheet::cell_buffer,
                     width - (2 * datasheet::cell_buffer),
                     height - (2 * datasheet::cell_buffer)};
    const trace_info &info{field_info.at(field_num.at(col))};
    if (info.type == Type::string_) {
      cell.text = values_string[row][info.array_col];
    } else if (info.type == Type::int_) {
      std::ostringstream oss{};
      oss << values_int[row][info.array_col];
      cell.text = oss.str();
    } else if (info.type == Type::float_) {
      std::ostringstream oss{};
      oss << values_float[row][info.array_col];
      cell.text = oss.str();
    } else if (info.type == Type::double_) {
      std::ostringstream oss{};
      oss << values_double[row][info.array_col];
      cell.text = oss.str();
    } else if (info.type == Type::bool_) {
      std::ostringstream oss{};
      oss << values_bool[row][info.array_col];
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

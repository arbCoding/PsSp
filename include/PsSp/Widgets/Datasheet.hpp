// Copyright 2023-2024 Alexander R. Blanchette

/*!
  \file PsSp/Widgets/Datasheet.hpp
  \brief Datasheet interface.
  \author Alexander R. Blanchette
  This file contains the Datasheet class interface, which provides the
  spreadsheet functional for scanning/editing seismic header/footer information
  on a per-Trace basic.

  This is handled in an efficient manner by spawning the InputManager wherever
  it is needed in the datasheet (instead of having a separate input manager for
  every entry).

  In testing this can handle millions of rows without any performance effect
  during operation, though closing can be slow (smart-pointers are more
  expensive to close on program exit than just allowing the OS to clear the
  leftover pointers after termination). Topic of allowing the OS to cleanup
  after programing termination versus being a good steward is a topic of much
  debate (OS is faster and usually expected, it is also considered bad overall
  design and not guaranteed behavior).

  I prefer to use smart-pointers so that the program itself handles cleanup.

  \todo Add sorting functional.
  \todo Add ability to drag and drop columns/rows.
  \todo Redo/Undo functionality.
  \todo Boolean cells use checkboxes (or switches).
  */

#ifndef PSSP_DATASHEET_HPP_20231215_1255
#define PSSP_DATASHEET_HPP_20231215_1255
#pragma once
// PsSp
#include "PsSp/Managers/InputManager.hpp"
#include "PsSp/Managers/SheetManager.hpp"
#include "PsSp/Utility/Enums.hpp"
#include "PsSp/Utility/Structs.hpp"
// fltk https://www.fltk.org/doc-1.4
#include <FL/Fl.H>
#include <FL/Fl_Check_Button.H>
#include <FL/fl_draw.H>
// This is needed for the window() function to prevent
// disappearing cursors
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Table.H>
// spdlog https://github.com/gabime/spdlog
#include <spdlog/spdlog.h>
// Standard library
//   https://en.cppreference.com/w/cpp/standard_library
#include <array>
// std::unique_ptr
#include <memory>
// std::ostringstream
#include <sstream>
// std::string
#include <string>

namespace pssp {
/*!
  \namespace pssp::datasheet

  Constants and structs specific to the Datasheet.

  \todo Move constants to PsSp/Utility/Constants.hpp
  \todo Move structs to PsSp/Utility/Structs.hpp
  */
namespace datasheet {
/*!
  \struct Spec
  \brief Used to specify the size of Datasheet cells.
  */
struct Spec {
  // cppcheck-suppress unusedStructMember
  int height{0};  //!< Cell height (pixels).
  // cppcheck-suppress unusedStructMember
  int header_height{0};  //!< Header-cell height (pixels).
  // cppcheck-suppress unusedStructMember
  int width{0};  //!< Cell width (pixels).
  // cppcheck-suppress unusedStructMember
  int header_width{0};  //!< Header-cell width (pixels).
};
//! Font-size in cells
constexpr int font_size{14};
//! Buffer between cell contents region and cell edge (pixels).
constexpr int cell_buffer{3};
//! Maximum number of characters allow in a cell.
constexpr int max_chars{10};
//! Keys that trigger cell editing.
const std::string edit_chars{"0123456789+-\r\n"};
/*!
  \struct Cell
  \brief Specify a datasheet cell.
  This includes placement, size, font, color, box-type, alignment, border-type,
  and content of a datasheet cell.
  */
struct Cell {
  // cppcheck-suppress unusedStructMember
  structs::Geometry full_box{};  //!< Geometry of Cell edges.
  // cppcheck-suppress unusedStructMember
  structs::Geometry
      text_box{};  //!< Geometry of Cell content (internal to full_box).
  Fl_Font font{FL_HELVETICA};           //!< Font used for Cell content.
  Fl_Color text_color{FL_BLACK};        //!< Color of Cell text.
  Fl_Color box_color{FL_GRAY};          //!< Color of cell background.
  Fl_Boxtype box_type{FL_THIN_UP_BOX};  //!< Type of cell drawing.
  Fl_Align alignment{FL_ALIGN_CENTER};  //!< Alignment of cell contents.
  // cppcheck-suppress unusedStructMember
  std::string text{};  //!< String of cell contents.
};
}  // namespace datasheet

// I want boolean cells to have a checkbox in them
// table-as-container.cxx does this in an inefficient way
// So combine with the moving widget setup I've been using to make it efficient
/*
  \brief The Datasheet class.

  This provides the Datasheet class, which is the spreadsheet view of the
  seismic records. In the future, this will need to communicate with the SQLite
  database to obtain and update values on save/load.

  It uses a SheetManager object to deal with the contents, an InputManager
  object to handle the input, and drawing/editing itself.

  \todo Make resizable upon adding/removing data.

  \class Datasheet
 */
class Datasheet : public Fl_Table {
public:
  Datasheet();
  void set_value_hide();
  void start_editing(size_t row, size_t col);
  void done_editing();

protected:
  void draw_cell(TableContext context, int row = 0, int col = 0, int x_pos = 0,
                 int y_pos = 0, int width = 0, int height = 0) FL_OVERRIDE;

  // table event callback (instance)
  void event_callback2();

  // table event callback (static) [trick to use event_callback2]
  static void event_callback(Fl_Widget *widget, void *datasheet) {
    (void)widget;
    reinterpret_cast<Datasheet *>(datasheet)->event_callback2();
  }

private:
  // cppcheck-suppress unusedStructMember
  size_t edit_row{0};  //!< Row of most recently edited cell.
  // cppcheck-suppress unusedStructMember
  size_t edit_col{0};  //!< Column of most recently edited cell.
  // cppcheck-suppress unusedStructMember
  size_t max_col{0};  //!< Maximum number of columns in the Datasheet.
  // cppcheck-suppress unusedStructMember
  size_t max_row{0};  //!< Maximum number of rows in the Datasheet.
  std::unique_ptr<SheetManager> sheet_manager{};  //!< SheetManager
  std::unique_ptr<InputManager> input_manager{};  //!< InputManager
  std::unique_ptr<Fl_Check_Button>
      check_button{};  //!< Boolean toggle (not implemented).
  static void draw_generic_cell(const datasheet::Cell &cell);
  static void draw_header_cell(structs::Geometry *geo, const std::string &text);
};
}  // namespace pssp

#endif

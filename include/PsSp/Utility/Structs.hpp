// Copyright 2023-2024 Alexander R. Blanchette
/*!
  \file PsSp/Utility/Structs.hpp
  \brief Structs used in PsSp
  \author Alexander R. Blanchette
  This file contains structs that are used throughout PsSp.
  */

#ifndef PSSP_STRUCTS_HPP_20231217_0600
#define PSSP_STRUCTS_HPP_20231217_0600
#pragma once

/*!
  \namespace pssp:structs

  Namespace for holding universal PsSp structs.

  \todo Move structs from other files to this file.
 */
namespace pssp::structs {
/*!
  \brief FLTK Geometry handling struct.

  This struct simplifies passing parameters to FLTK drawing functions (instead
  of passing four loose integers).

  \struct Geometry
  */
struct Geometry {
  // cppcheck-suppress unusedStructMember
  int x_pos{0};  //!< Left-most position of FLTK object.
  // cppcheck-suppress unusedStructMember
  int y_pos{0};  //!< Upper-most position of FLTK object.
  // cppcheck-suppress unusedStructMember
  int width{0};  //!< Width of FLTK object.
  // cppcheck-suppress unusedStructMember
  int height{0};  //!< Height of FLTK object.
};

/*!
  \brief FLTK Grid definition struct.

  This struct makes it easy to define objects in an FLTK grid (Fl_Grid).
  Used in Windows/Main.cpp to define the layout of the MainWindow.

  \struct Grid
  */
struct Grid {
  // cppcheck-suppress unusedStructMember
  int row{0};  //!< First row (top-most row) of grid position.
  // cppcheck-suppress unusedStructMember
  int col{0};  //!< First column (left-most column) of grid position.
  // cppcheck-suppress unusedStructMember
  int row_span{0};  //!< Width (in rows) of object.
  // cppcheck-suppress unusedStructMember
  int col_span{0};  //!< Height (in columns) of object.
};
}  // namespace pssp::structs

#endif

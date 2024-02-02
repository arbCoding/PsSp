// Copyright 2023-2024 Alexander R. Blanchette

/*!
  \file PsSp/Utility/Enums.hpp
  \brief Enumerations for PsSp.
  \author Alexander R. Blanchette
  This file contains enumeration definitions for PsSp.

  \todo Non-enums (constants) belong in PsSp/Utility/Constants.hpp
  */

#ifndef PSSP_ENUMS_HPP_20231218_1053
#define PSSP_ENUMS_HPP_20231218_1053
#pragma once

// Standard Library
//   https://en.cppreference.com/w/cpp/standard_library
// std::string
#include <string>
// std::unordered_map
#include <unordered_map>

namespace pssp {
/*!
  \brief Data-type enumeration.

  Allows maintaining the type of data (string, integer, float, double, bool) for
  an object since this isn't supported by default in C++.

  \enum Type
 */
enum class Type {
  string_,  //!< String data-type
  int_,     //!< Integer data-type
  float_,   //!< Float data-type
  double_,  //!< Double data-type
  bool_,    //!< Boolean data-type
};

/*!
  \brief Map Type to string-name.

  Used to provide labels for the trace_info struct.

  \todo Move to PsSp/Utility/Constants.hpp
  */
const std::unordered_map<Type, const std::string> type_names{
    {Type::string_, "string"},
    {Type::int_, "int"},
    {Type::float_, "float"},
    {Type::double_, "double"},
    {Type::bool_, "bool"}};

/*!
  \brief Information for use in the Datasheet.

  This is information for a specific column (row each row) in teh Datasheet.

  \todo Move to PsSp/Utility/Structs.hpp

  \struct trace_info
  */
struct trace_info {
  // cppcheck-suppress unusedStructMember
  const size_t col{0};  //!< Location of value in Datasheet.
  // cppcheck-suppress unusedStructMember
  const size_t array_col{0};  //!< Location of value in internal storage array.
  // cppcheck-suppress unusedStructMember
  const std::string name{};  //!< Data Type name derived from ::type_names
  // cppcheck-suppress unusedStructMember
  const Type type{};  //!< Data Type.
};

/*!
  \brief SAC-header/footer field enumeration

  \todo This is for prototyping SAC-records, in the future this will be supplied
  by the sac-format library (once we're ready to read in SAC-files).

  \enum Field
 */
enum class Field {
  depmin,
  depmax,
  odelta,
  resp0,
  resp1,
  resp2,
  resp3,
  resp4,
  resp5,
  resp6,
  resp7,
  resp8,
  resp9,
  stel,
  stdp,
  evel,
  evdp,
  mag,
  user0,
  user1,
  user2,
  user3,
  user4,
  user5,
  user6,
  user7,
  user8,
  user9,
  dist,
  az,
  baz,
  gcarc,
  depmen,
  cmpaz,
  cmpinc,
  xminimum,
  xmaximum,
  yminimum,
  ymaximum,
  delta,
  b,
  e,
  o,
  a,
  t0,
  t1,
  t2,
  t3,
  t4,
  t5,
  t6,
  t7,
  t8,
  t9,
  f,
  stla,
  stlo,
  evla,
  evlo,
  sb,
  sdelta,
  nzyear,
  nzjday,
  nzhour,
  nzmin,
  nzsec,
  nzmsec,
  nvhdr,
  norid,
  nevid,
  npts,
  nsnpts,
  nwfid,
  nxsize,
  nysize,
  iftype,
  idep,
  iztype,
  iinst,
  istreg,
  ievreg,
  ievtyp,
  iqual,
  isynth,
  imagtyp,
  imagsrc,
  ibody,
  leven,
  lpspol,
  lovrok,
  lcalda,
  kstnm,
  kevnm,
  khole,
  ko,
  ka,
  kt0,
  kt1,
  kt2,
  kt3,
  kt4,
  kt5,
  kt6,
  kt7,
  kt8,
  kt9,
  kf,
  kuser0,
  kuser1,
  kuser2,
  kcmpnm,
  knetwk,
  kdatrd,
  kinst,
  data1,
  data2
};

/*!
  \brief Map of column number (Datasheet) to Field.

  Given a column in the Datasheet, get the Field (used as a key in another map).

  \todo Merge into field_Info
  */
const std::unordered_map<size_t, Field> field_num{// Floats
                                                  {0, Field::depmin},
                                                  {1, Field::depmax},
                                                  {2, Field::odelta},
                                                  {3, Field::resp0},
                                                  {4, Field::resp1},
                                                  {5, Field::resp2},
                                                  {6, Field::resp3},
                                                  {7, Field::resp4},
                                                  {8, Field::resp5},
                                                  {9, Field::resp6},
                                                  {10, Field::resp7},
                                                  {11, Field::resp8},
                                                  {12, Field::resp9},
                                                  {13, Field::stel},
                                                  {14, Field::stdp},
                                                  {15, Field::evel},
                                                  {16, Field::evdp},
                                                  {17, Field::mag},
                                                  {18, Field::user0},
                                                  {19, Field::user1},
                                                  {20, Field::user2},
                                                  {21, Field::user3},
                                                  {22, Field::user4},
                                                  {23, Field::user5},
                                                  {24, Field::user6},
                                                  {25, Field::user7},
                                                  {26, Field::user8},
                                                  {27, Field::user9},
                                                  {28, Field::dist},
                                                  {29, Field::az},
                                                  {30, Field::baz},
                                                  {31, Field::gcarc},
                                                  {32, Field::depmen},
                                                  {33, Field::cmpaz},
                                                  {34, Field::cmpinc},
                                                  {35, Field::xminimum},
                                                  {36, Field::xmaximum},
                                                  {37, Field::yminimum},
                                                  {38, Field::ymaximum},
                                                  // Doubles
                                                  {39, Field::delta},
                                                  {40, Field::b},
                                                  {41, Field::e},
                                                  {42, Field::o},
                                                  {43, Field::a},
                                                  {44, Field::t0},
                                                  {45, Field::t1},
                                                  {46, Field::t2},
                                                  {47, Field::t3},
                                                  {48, Field::t4},
                                                  {49, Field::t5},
                                                  {50, Field::t6},
                                                  {51, Field::t7},
                                                  {52, Field::t8},
                                                  {53, Field::t9},
                                                  {54, Field::f},
                                                  {55, Field::stla},
                                                  {56, Field::stlo},
                                                  {57, Field::evla},
                                                  {58, Field::evlo},
                                                  {59, Field::sb},
                                                  {60, Field::sdelta},
                                                  // Ints
                                                  {61, Field::nzyear},
                                                  {62, Field::nzjday},
                                                  {63, Field::nzhour},
                                                  {64, Field::nzmin},
                                                  {65, Field::nzsec},
                                                  {66, Field::nzmsec},
                                                  {67, Field::nvhdr},
                                                  {68, Field::norid},
                                                  {69, Field::nevid},
                                                  {70, Field::npts},
                                                  {71, Field::nsnpts},
                                                  {72, Field::nwfid},
                                                  {73, Field::nxsize},
                                                  {74, Field::nysize},
                                                  {75, Field::iftype},
                                                  {76, Field::idep},
                                                  {77, Field::iztype},
                                                  {78, Field::iinst},
                                                  {79, Field::istreg},
                                                  {80, Field::ievreg},
                                                  {81, Field::ievtyp},
                                                  {82, Field::iqual},
                                                  {83, Field::isynth},
                                                  {84, Field::imagtyp},
                                                  {85, Field::imagsrc},
                                                  {86, Field::ibody},
                                                  // Bools
                                                  {87, Field::leven},
                                                  {88, Field::lpspol},
                                                  {89, Field::lovrok},
                                                  {90, Field::lcalda},
                                                  // Strings
                                                  {91, Field::kstnm},
                                                  {92, Field::kevnm},
                                                  {93, Field::khole},
                                                  {94, Field::ko},
                                                  {95, Field::ka},
                                                  {96, Field::kt0},
                                                  {97, Field::kt1},
                                                  {98, Field::kt2},
                                                  {99, Field::kt3},
                                                  {100, Field::kt4},
                                                  {101, Field::kt5},
                                                  {102, Field::kt6},
                                                  {103, Field::kt7},
                                                  {104, Field::kt8},
                                                  {105, Field::kt9},
                                                  {106, Field::kf},
                                                  {107, Field::kuser0},
                                                  {108, Field::kuser1},
                                                  {109, Field::kuser2},
                                                  {110, Field::kcmpnm},
                                                  {111, Field::knetwk},
                                                  {112, Field::kdatrd},
                                                  {113, Field::kinst},
                                                  // Data
                                                  {114, Field::data1},
                                                  {115, Field::data2}};

/*!
  \brief Map Field to trace_info.

  Given a field, get its trace_info (column, array-colun, type-name, and Type).

  This is needed for interacting with the Datasheet.

  \todo Merge field_num into this.
  */
const std::unordered_map<Field, trace_info> field_info{
    // Floats
    {Field::depmin, {0, 0, "DepMin", Type::float_}},
    {Field::depmax, {1, 1, "DepMax", Type::float_}},
    {Field::odelta, {2, 2, "ODelta", Type::float_}},
    {Field::resp0, {3, 3, "Resp0", Type::float_}},
    {Field::resp1, {4, 4, "Resp1", Type::float_}},
    {Field::resp2, {5, 5, "Resp2", Type::float_}},
    {Field::resp3, {6, 6, "Resp3", Type::float_}},
    {Field::resp4, {7, 7, "Resp4", Type::float_}},
    {Field::resp5, {8, 8, "Resp5", Type::float_}},
    {Field::resp6, {9, 9, "Resp6", Type::float_}},
    {Field::resp7, {10, 10, "Resp7", Type::float_}},
    {Field::resp8, {11, 11, "Resp8", Type::float_}},
    {Field::resp9, {12, 12, "Resp9", Type::float_}},
    {Field::stel, {13, 13, "StEl", Type::float_}},
    {Field::stdp, {14, 14, "StDp", Type::float_}},
    {Field::evel, {15, 15, "EvEl", Type::float_}},
    {Field::evdp, {16, 16, "EvDp", Type::float_}},
    {Field::mag, {17, 17, "Mag", Type::float_}},
    {Field::user0, {18, 18, "User0", Type::float_}},
    {Field::user1, {19, 19, "User1", Type::float_}},
    {Field::user2, {20, 20, "User2", Type::float_}},
    {Field::user3, {21, 21, "User3", Type::float_}},
    {Field::user4, {21, 22, "User4", Type::float_}},
    {Field::user5, {23, 23, "User5", Type::float_}},
    {Field::user6, {24, 24, "User6", Type::float_}},
    {Field::user7, {25, 25, "User7", Type::float_}},
    {Field::user8, {26, 26, "User8", Type::float_}},
    {Field::user9, {27, 27, "User9", Type::float_}},
    {Field::dist, {28, 28, "Dist", Type::float_}},
    {Field::az, {29, 29, "Az", Type::float_}},
    {Field::baz, {30, 30, "BAz", Type::float_}},
    {Field::gcarc, {31, 31, "GCArc", Type::float_}},
    {Field::depmen, {32, 32, "DepMen", Type::float_}},
    {Field::cmpaz, {33, 33, "CmpAz", Type::float_}},
    {Field::cmpinc, {34, 34, "CmpInc", Type::float_}},
    {Field::xminimum, {35, 35, "XMinimum", Type::float_}},
    {Field::xmaximum, {36, 36, "XMaximum", Type::float_}},
    {Field::yminimum, {37, 37, "YMinimum", Type::float_}},
    {Field::ymaximum, {38, 38, "YMaximum", Type::float_}},
    // Doubles
    {Field::delta, {39, 0, "Delta", Type::double_}},
    {Field::b, {40, 1, "B", Type::double_}},
    {Field::e, {41, 2, "E", Type::double_}},
    {Field::o, {42, 3, "O", Type::double_}},
    {Field::a, {43, 4, "A", Type::double_}},
    {Field::t0, {44, 5, "T0", Type::double_}},
    {Field::t1, {45, 6, "T1", Type::double_}},
    {Field::t2, {46, 7, "T2", Type::double_}},
    {Field::t3, {47, 8, "T3", Type::double_}},
    {Field::t4, {48, 9, "T4", Type::double_}},
    {Field::t5, {49, 10, "T5", Type::double_}},
    {Field::t6, {50, 11, "T6", Type::double_}},
    {Field::t7, {51, 12, "T7", Type::double_}},
    {Field::t8, {52, 13, "T8", Type::double_}},
    {Field::t9, {53, 14, "T9", Type::double_}},
    {Field::f, {54, 15, "F", Type::double_}},
    {Field::stla, {55, 16, "StLa", Type::double_}},
    {Field::stlo, {56, 17, "StLo", Type::double_}},
    {Field::evla, {57, 18, "EvLa", Type::double_}},
    {Field::evlo, {58, 19, "EvLo", Type::double_}},
    {Field::sb, {59, 20, "sB", Type::double_}},
    {Field::sdelta, {60, 21, "sDelta", Type::double_}},
    // Ints
    {Field::nzyear, {61, 0, "nzYear", Type::int_}},
    {Field::nzjday, {62, 1, "nzJDay", Type::int_}},
    {Field::nzhour, {63, 2, "nzHour", Type::int_}},
    {Field::nzmin, {64, 3, "nzMin", Type::int_}},
    {Field::nzsec, {65, 4, "nzSec", Type::int_}},
    {Field::nzmsec, {66, 5, "nzMSec", Type::int_}},
    {Field::nvhdr, {67, 6, "nVHdr", Type::int_}},
    {Field::norid, {68, 7, "nOrID", Type::int_}},
    {Field::nevid, {69, 8, "nEvID", Type::int_}},
    {Field::npts, {70, 9, "nPts", Type::int_}},
    {Field::nsnpts, {71, 10, "nsnPts", Type::int_}},
    {Field::nwfid, {72, 11, "nWfID", Type::int_}},
    {Field::nxsize, {73, 12, "nXSize", Type::int_}},
    {Field::nysize, {74, 13, "nYSize", Type::int_}},
    {Field::iftype, {75, 14, "iFType", Type::int_}},
    {Field::idep, {76, 15, "iDep", Type::int_}},
    {Field::iztype, {77, 16, "iZType", Type::int_}},
    {Field::iinst, {78, 17, "iInst", Type::int_}},
    {Field::istreg, {79, 18, "iStReg", Type::int_}},
    {Field::ievreg, {80, 19, "iEvReg", Type::int_}},
    {Field::ievtyp, {81, 20, "iEvTyp", Type::int_}},
    {Field::iqual, {82, 21, "iQual", Type::int_}},
    {Field::isynth, {83, 22, "iSynth", Type::int_}},
    {Field::imagtyp, {84, 23, "iMagTyp", Type::int_}},
    {Field::imagsrc, {85, 24, "iMagSrc", Type::int_}},
    {Field::ibody, {86, 25, "iBody", Type::int_}},
    // Bools
    {Field::leven, {87, 0, "lEven", Type::bool_}},
    {Field::lpspol, {88, 1, "lPsPol", Type::bool_}},
    {Field::lovrok, {89, 2, "lOvrOK", Type::bool_}},
    {Field::lcalda, {90, 3, "lCalDA", Type::bool_}},
    // Strings
    {Field::kstnm, {91, 0, "kStNm", Type::string_}},
    {Field::kevnm, {92, 1, "kEvNm", Type::string_}},
    {Field::khole, {93, 2, "kHole", Type::string_}},
    {Field::ko, {94, 3, "kO", Type::string_}},
    {Field::ka, {95, 4, "kA", Type::string_}},
    {Field::kt0, {96, 5, "kT0", Type::string_}},
    {Field::kt1, {97, 6, "kT1", Type::string_}},
    {Field::kt2, {98, 7, "kT2", Type::string_}},
    {Field::kt3, {99, 8, "kT3", Type::string_}},
    {Field::kt4, {100, 9, "kT4", Type::string_}},
    {Field::kt5, {101, 10, "kT5", Type::string_}},
    {Field::kt6, {102, 11, "kT6", Type::string_}},
    {Field::kt7, {103, 12, "kT7", Type::string_}},
    {Field::kt8, {104, 13, "kT8", Type::string_}},
    {Field::kt9, {105, 14, "kT9", Type::string_}},
    {Field::kf, {106, 15, "kF", Type::string_}},
    {Field::kuser0, {107, 16, "kUser0", Type::string_}},
    {Field::kuser1, {108, 17, "kUser1", Type::string_}},
    {Field::kuser2, {109, 18, "kUser2", Type::string_}},
    {Field::kcmpnm, {110, 19, "kCmpNm", Type::string_}},
    {Field::knetwk, {111, 20, "kNetwk", Type::string_}},
    {Field::kdatrd, {112, 21, "kDatRd", Type::string_}},
    {Field::kinst, {113, 22, "kInst", Type::string_}},
    // Data
    {Field::data1, {114, 0, "Data1", Type::int_}},
    {Field::data2, {115, 1, "Data2", Type::int_}}};
}  // namespace pssp
#endif

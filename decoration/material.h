// Material Dark - window decoration (KDecoration3)
// SPDX-License-Identifier: MIT
#pragma once

#include <QColor>

namespace Material
{
constexpr int Border = 1;      // tebal bingkai jendela (outline variant)
constexpr int Divider = 1;     // garis tipis di bawah titlebar
constexpr int TitlePad = 6;    // jarak vertikal titlebar
constexpr int ButtonSize = 24; // diameter tombol (lingkaran)
constexpr int ButtonGap = 16;  // jarak antar tombol
constexpr int SideMargin = 12; // jarak tombol dari tepi
constexpr int ShadowSize = 24; // jari-jari bayangan jendela (elevation)
constexpr int Radius = 6;      // jari-jari sudut jendela (setengah dari 12)

const QColor Error(242, 184, 181);  // M3 dark error
const QColor OnError(96, 20, 16);   // M3 dark on-error
}

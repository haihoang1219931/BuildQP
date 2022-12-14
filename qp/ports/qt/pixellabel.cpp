//============================================================================
// Copyright (C) 2005 Quantum Leaps, LLC <state-machine.com>.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
//! @date Last updated on: 2022-08-25
//! @version Last updated for: @ref qpcpp_7_1_0
//!
//! @file
//! @brief QP/C++ port to Qt

#include <QtGui>
#include "pixellabel.hpp"

static int const BMP_HDR = 54U; // size of the header in the BMP file format

//............................................................................
void PixelLabel::init(int width,  int xScale,
                      int height, int yScale,
                      quint8 const bgColor[3])
{
    m_width  = width;
    m_xScale = xScale;
    m_height = height;
    m_yScale = yScale;

    m_bgColor[0] = bgColor[0];
    m_bgColor[1] = bgColor[1];
    m_bgColor[2] = bgColor[2];

    m_size = BMP_HDR + 3*(width*xScale * height*yScale);
    m_bits = new quint8[m_size];
    memset(m_bits, 0, m_size);

    m_bits[ 0] = (quint8)'B';
    m_bits[ 1] = (quint8)'M';
    m_bits[ 2] = m_size & 0xFFU;
    m_bits[ 3] = (m_size >>  8) & 0xFFU;
    m_bits[ 4] = (m_size >> 16) & 0xFFU;
    m_bits[ 5] = (m_size >> 24) & 0xFFU;
    m_bits[10] = BMP_HDR;

    m_bits[14] = 40;  // size of the secondary header

    quint32 tmp = width * xScale;
    m_bits[18] = tmp & 0xFFU;
    m_bits[19] = (tmp >>  8) & 0xFFU;
    m_bits[20] = (tmp >> 16) & 0xFFU;
    m_bits[21] = (tmp >> 24) & 0xFFU;

    tmp = height * yScale;
    m_bits[22] = tmp & 0xFFU;
    m_bits[23] = (tmp >>  8) & 0xFFU;
    m_bits[24] = (tmp >> 16) & 0xFFU;
    m_bits[25] = (tmp >> 24) & 0xFFU;
    m_bits[26] = 1U;           // number of planes
    m_bits[28] = 3U*8U;  // color bits per pixel
    m_bits[38] = 0x12U; // hor: # of pixels per meter LSB
    m_bits[39] = 0x0BU; // hor: # of pixels per meter MSB
    m_bits[42] = 0x12U; // ver: # of pixels per meter LSB
    m_bits[43] = 0x0BU; // ver: # of pixels per meter MSB
}
//............................................................................
PixelLabel::~PixelLabel() {
    delete [] m_bits;
}
//............................................................................
void PixelLabel::clear(void) {
    unsigned n;
    quint8 r = m_bgColor[0];
    quint8 g = m_bgColor[1];
    quint8 b = m_bgColor[2];
    quint8 *bits = &m_bits[BMP_HDR];

    for (n = m_width*m_xScale * m_height * m_yScale;
         n != 0;
         --n, bits += 3)
    {
        bits[0] = b;
        bits[1] = g;
        bits[2] = r;
    }
}
//............................................................................
void PixelLabel::setPixel(int x, int y, quint8 const color[3]) {
    quint16 sx, sy;
    quint8 *pixelRGB =
        &m_bits[BMP_HDR + 3*(m_xScale*x
                  + m_xScale*m_width * m_yScale*(m_height - 1U - y))];
    quint8 r = color[0];
    quint8 g = color[1];
    quint8 b = color[2];
    for (sy = m_yScale; sy != 0U;
         --sy, pixelRGB += m_xScale*m_width*3U)
    {
        for (sx = 3*m_xScale; sx != 0U; sx -= 3U) {
            pixelRGB[sx - 3U] = b;
            pixelRGB[sx - 2U] = g;
            pixelRGB[sx - 1U] = r;
        }
    }
}
//............................................................................
void PixelLabel::redraw(void) {
    (void)m_pixmap.loadFromData(m_bits, m_size, "BMP");
    setPixmap(m_pixmap);
}

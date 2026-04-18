/*
 *  Copyright (C) 2026
 *
 *  This file is part of spectr.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QColor>

// ── Color Constants (reusable in C++ painting code) ──────────────────────────

namespace Theme {
    // Base palette
    static const QColor bgDark(0x0a, 0x0a, 0x14);       // #0a0a14 — deepest background
    static const QColor bgBase(0x1e, 0x1e, 0x2e);       // #1e1e2e — main background
    static const QColor bgSurface(0x2a, 0x2a, 0x3a);    // #2a2a3a — panels / dock
    static const QColor bgElevated(0x33, 0x33, 0x48);    // #333348 — hover / elevated
    static const QColor bgInput(0x18, 0x18, 0x28);       // #181828 — input fields

    // Accent
    static const QColor accent(0x00, 0xd4, 0xaa);        // #00d4aa — teal/cyan accent
    static const QColor accentHover(0x00, 0xf0, 0xc0);   // #00f0c0 — lighter on hover
    static const QColor accentDim(0x00, 0x8a, 0x70);     // #008a70 — dimmed accent

    // Text
    static const QColor textPrimary(0xe0, 0xe0, 0xe8);   // #e0e0e8
    static const QColor textSecondary(0x90, 0x90, 0xa8);  // #9090a8
    static const QColor textMuted(0x60, 0x60, 0x78);      // #606078

    // Trace plot colors
    static const QColor traceI(0x00, 0xd4, 0xaa);        // Cyan/teal — I channel
    static const QColor traceQ(0xff, 0x6b, 0x6b);        // Coral    — Q channel
    static const QColor traceSingle(0xff, 0xd9, 0x3d);   // Amber    — single channel

    // Cursor / tuner colors
    static const QColor cursorLine(0x00, 0xd4, 0xaa);    // accent
    static const QColor cursorGlow(0x00, 0xd4, 0xaa, 0x40); // accent semi-transparent
    static const QColor cursorFill(0x00, 0xd4, 0xaa, 0x18); // very faint fill
    static const QColor tunerCenter(0xff, 0xb0, 0x40);   // Amber center freq

    // Grid
    static const QColor gridLine(0xff, 0xff, 0xff, 0x18); // Very faint white
}

// ── Global Stylesheet ────────────────────────────────────────────────────────

static const char* globalStylesheet = R"QSS(

/* ── Base ─────────────────────────────────────────────────────────────────── */

QMainWindow {
    background-color: #1e1e2e;
}

QWidget {
    color: #e0e0e8;
    font-size: 13px;
}

/* ── Dock Widget ──────────────────────────────────────────────────────────── */

QDockWidget {
    background-color: #2a2a3a;
    titlebar-close-icon: none;
    titlebar-normal-icon: none;
    border: none;
}

QDockWidget::title {
    background-color: #252535;
    padding: 8px 12px;
    font-size: 14px;
    font-weight: bold;
    color: #00d4aa;
    border-bottom: 1px solid #333348;
}

QDockWidget > QWidget {
    background-color: #2a2a3a;
}

/* ── Push Button ──────────────────────────────────────────────────────────── */

QPushButton {
    background-color: #333348;
    color: #e0e0e8;
    border: 1px solid #444460;
    border-radius: 6px;
    padding: 8px 16px;
    font-weight: bold;
    font-size: 13px;
}

QPushButton:hover {
    background-color: #3d3d55;
    border-color: #00d4aa;
    color: #00d4aa;
}

QPushButton:pressed {
    background-color: #00d4aa;
    color: #1e1e2e;
}

/* ── Line Edit ────────────────────────────────────────────────────────────── */

QLineEdit {
    background-color: #181828;
    color: #e0e0e8;
    border: 1px solid #444460;
    border-radius: 4px;
    padding: 5px 8px;
    selection-background-color: #00d4aa;
    selection-color: #1e1e2e;
}

QLineEdit:focus {
    border-color: #00d4aa;
}

/* ── Labels ───────────────────────────────────────────────────────────────── */

QLabel {
    color: #9090a8;
    background: transparent;
    padding: 1px 0px;
}

/* ── Slider ───────────────────────────────────────────────────────────────── */

QSlider::groove:horizontal {
    border: none;
    height: 6px;
    background: #181828;
    border-radius: 3px;
}

QSlider::sub-page:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #008a70, stop:1 #00d4aa);
    border-radius: 3px;
}

QSlider::add-page:horizontal {
    background: #181828;
    border-radius: 3px;
}

QSlider::handle:horizontal {
    background: #00d4aa;
    border: 2px solid #00f0c0;
    width: 14px;
    height: 14px;
    margin: -5px 0;
    border-radius: 8px;
}

QSlider::handle:horizontal:hover {
    background: #00f0c0;
    border-color: #00ffcc;
}

/* ── Check Box ────────────────────────────────────────────────────────────── */

QCheckBox {
    spacing: 8px;
    color: #e0e0e8;
}

QCheckBox::indicator {
    width: 18px;
    height: 18px;
    border-radius: 4px;
    border: 2px solid #444460;
    background-color: #181828;
}

QCheckBox::indicator:checked {
    background-color: #00d4aa;
    border-color: #00d4aa;
}

QCheckBox::indicator:hover {
    border-color: #00d4aa;
}

/* ── Spin Box ─────────────────────────────────────────────────────────────── */

QSpinBox {
    background-color: #181828;
    color: #e0e0e8;
    border: 1px solid #444460;
    border-radius: 4px;
    padding: 4px 8px;
}

QSpinBox:focus {
    border-color: #00d4aa;
}

QSpinBox::up-button, QSpinBox::down-button {
    background-color: #333348;
    border: none;
    width: 16px;
}

QSpinBox::up-button:hover, QSpinBox::down-button:hover {
    background-color: #3d3d55;
}

QSpinBox::up-arrow {
    image: none;
    border-left: 4px solid transparent;
    border-right: 4px solid transparent;
    border-bottom: 5px solid #9090a8;
    width: 0; height: 0;
}

QSpinBox::down-arrow {
    image: none;
    border-left: 4px solid transparent;
    border-right: 4px solid transparent;
    border-top: 5px solid #9090a8;
    width: 0; height: 0;
}

/* ── Scroll Bar ───────────────────────────────────────────────────────────── */

QScrollBar:horizontal {
    background: #1e1e2e;
    height: 10px;
    border: none;
}

QScrollBar::handle:horizontal {
    background: #444460;
    border-radius: 4px;
    min-width: 30px;
    margin: 1px;
}

QScrollBar::handle:horizontal:hover {
    background: #00d4aa;
}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0px;
}

QScrollBar:vertical {
    background: #1e1e2e;
    width: 10px;
    border: none;
}

QScrollBar::handle:vertical {
    background: #444460;
    border-radius: 4px;
    min-height: 30px;
    margin: 1px;
}

QScrollBar::handle:vertical:hover {
    background: #00d4aa;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}

/* ── Group Box ────────────────────────────────────────────────────────────── */

QGroupBox {
    background-color: #252535;
    border: 1px solid #333348;
    border-radius: 8px;
    margin-top: 14px;
    padding: 16px 10px 10px 10px;
    font-weight: bold;
    color: #00d4aa;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    padding: 2px 10px;
    color: #00d4aa;
    font-size: 13px;
}

/* ── Menu ─────────────────────────────────────────────────────────────────── */

QMenu {
    background-color: #2a2a3a;
    border: 1px solid #333348;
    border-radius: 6px;
    padding: 4px;
}

QMenu::item {
    padding: 6px 24px;
    border-radius: 4px;
    color: #e0e0e8;
}

QMenu::item:selected {
    background-color: #00d4aa;
    color: #1e1e2e;
}

QMenu::separator {
    height: 1px;
    background: #333348;
    margin: 4px 8px;
}

/* ── ToolTip ──────────────────────────────────────────────────────────────── */

QToolTip {
    background-color: #333348;
    color: #e0e0e8;
    border: 1px solid #00d4aa;
    border-radius: 4px;
    padding: 4px 8px;
    font-size: 12px;
}

/* ── Status Bar ───────────────────────────────────────────────────────────── */

QStatusBar {
    background-color: #252535;
    color: #9090a8;
    border-top: 1px solid #333348;
    font-size: 12px;
}

QStatusBar::item {
    border: none;
}

/* ── Toolbar ──────────────────────────────────────────────────────────────── */

QToolBar {
    background-color: #252535;
    border-bottom: 1px solid #333348;
    padding: 4px;
    spacing: 4px;
}

QToolButton {
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 4px;
    padding: 6px 10px;
    color: #e0e0e8;
    font-size: 12px;
}

QToolButton:hover {
    background-color: #333348;
    border-color: #444460;
}

QToolButton:pressed {
    background-color: #00d4aa;
    color: #1e1e2e;
}

QToolButton:checked {
    background-color: #008a70;
    color: #e0e0e8;
    border-color: #00d4aa;
}

/* ── Form Layout Labels ──────────────────────────────────────────────────── */

QFormLayout {
    margin: 8px;
}

/* ── Progress Dialog ──────────────────────────────────────────────────────── */

QProgressDialog {
    background-color: #2a2a3a;
}

QProgressBar {
    background-color: #181828;
    border: 1px solid #333348;
    border-radius: 4px;
    text-align: center;
    color: #e0e0e8;
}

QProgressBar::chunk {
    background-color: #00d4aa;
    border-radius: 3px;
}

/* ── Message Box ──────────────────────────────────────────────────────────── */

QMessageBox {
    background-color: #2a2a3a;
}

/* ── File Dialog ──────────────────────────────────────────────────────────── */

QFileDialog {
    background-color: #2a2a3a;
}

QHeaderView::section {
    background-color: #252535;
    color: #9090a8;
    border: 1px solid #333348;
    padding: 4px;
}

QTreeView, QListView, QTableView {
    background-color: #181828;
    alternate-background-color: #1e1e2e;
    color: #e0e0e8;
    border: 1px solid #333348;
    border-radius: 4px;
}

QTreeView::item:selected, QListView::item:selected, QTableView::item:selected {
    background-color: #00d4aa;
    color: #1e1e2e;
}

QComboBox {
    background-color: #181828;
    color: #e0e0e8;
    border: 1px solid #444460;
    border-radius: 4px;
    padding: 4px 8px;
}

QComboBox:hover {
    border-color: #00d4aa;
}

QComboBox QAbstractItemView {
    background-color: #2a2a3a;
    color: #e0e0e8;
    selection-background-color: #00d4aa;
    selection-color: #1e1e2e;
    border: 1px solid #333348;
}

QComboBox::drop-down {
    background-color: #333348;
    border: none;
    width: 20px;
}

/* ── Radio Button ─────────────────────────────────────────────────────────── */

QRadioButton {
    spacing: 8px;
    color: #e0e0e8;
}

QRadioButton::indicator {
    width: 16px;
    height: 16px;
    border-radius: 9px;
    border: 2px solid #444460;
    background-color: #181828;
}

QRadioButton::indicator:checked {
    background-color: #00d4aa;
    border-color: #00d4aa;
}

QRadioButton::indicator:hover {
    border-color: #00d4aa;
}

)QSS";

/*
 *  Copyright (C) 2015, Mike Walters <mike@flomp.net>
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

#include <QMessageBox>
#include <QtWidgets>
#include <QPixmapCache>
#include <QRubberBand>
#include <sstream>

#include "mainwindow.h"
#include "util.h"

MainWindow::MainWindow()
{
    setWindowTitle(tr("spectr"));
    setMinimumSize(800, 600);

    QPixmapCache::setCacheLimit(40960);

    // ── Toolbar ──────────────────────────────────────────────────────────────
    toolBar = addToolBar(tr("Main"));
    toolBar->setMovable(false);
    toolBar->setIconSize(QSize(18, 18));

    openAction = toolBar->addAction(QString::fromUtf8("\xF0\x9F\x93\x82 Open"), this, [this]() {
        dock->fileOpenButtonClicked();
    });
    openAction->setShortcut(QKeySequence::Open);
    openAction->setToolTip(tr("Open file (Ctrl+O)"));

    toolBar->addSeparator();

    zoomInAction = toolBar->addAction(QString::fromUtf8("\xE2\x9E\x95 Zoom In"), this, [this]() {
        dock->zoomIn();
    });
    zoomInAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    zoomInAction->setToolTip(tr("Zoom in (Ctrl++)"));

    zoomOutAction = toolBar->addAction(QString::fromUtf8("\xE2\x9E\x96 Zoom Out"), this, [this]() {
        dock->zoomOut();
    });
    zoomOutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    zoomOutAction->setToolTip(tr("Zoom out (Ctrl+-)"));

    toolBar->addSeparator();

    toggleCursorsAction = toolBar->addAction(QString::fromUtf8("\xE2\x9C\x82 Cursors"));
    toggleCursorsAction->setCheckable(true);
    toggleCursorsAction->setShortcut(QKeySequence(Qt::Key_C));
    toggleCursorsAction->setToolTip(tr("Toggle cursors (C)"));

    toggleScalesAction = toolBar->addAction(QString::fromUtf8("\xF0\x9F\x93\x8F Scales"));
    toggleScalesAction->setCheckable(true);
    toggleScalesAction->setChecked(true);
    toggleScalesAction->setShortcut(QKeySequence(Qt::Key_S));
    toggleScalesAction->setToolTip(tr("Toggle scales (S)"));

    // ── Status Bar ───────────────────────────────────────────────────────────
    statusFileLabel = new QLabel(tr("  No file loaded  "));
    statusFileLabel->setStyleSheet("color: #9090a8; padding: 0 12px;");
    statusRateLabel = new QLabel(tr("  Rate: —  "));
    statusRateLabel->setStyleSheet("color: #9090a8; padding: 0 12px;");
    statusZoomLabel = new QLabel(tr("  Zoom: 1×  "));
    statusZoomLabel->setStyleSheet("color: #9090a8; padding: 0 12px;");

    statusBar()->addWidget(statusFileLabel);
    statusBar()->addWidget(statusRateLabel);
    statusBar()->addPermanentWidget(statusZoomLabel);

    // ── Dock (Controls Panel) ────────────────────────────────────────────────
    dock = new SpectrogramControls(tr("Controls"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    input = new InputSource();
    input->subscribe(this);

    plots = new PlotView(input);
    setCentralWidget(plots);

    // Connect dock inputs
    connect(dock, &SpectrogramControls::openFile, this, &MainWindow::openFile);
    connect(dock->sampleRate, static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textChanged), this, static_cast<void (MainWindow::*)(QString)>(&MainWindow::setSampleRate));
    connect(dock, static_cast<void (SpectrogramControls::*)(int, int)>(&SpectrogramControls::fftOrZoomChanged), plots, &PlotView::setFFTAndZoom);
    connect(dock->powerMaxSlider, &QSlider::valueChanged, plots, &PlotView::setPowerMax);
    connect(dock->powerMinSlider, &QSlider::valueChanged, plots, &PlotView::setPowerMin);
    connect(dock->cursorsCheckBox, &QCheckBox::stateChanged, plots, &PlotView::enableCursors);
    connect(dock->scalesCheckBox, &QCheckBox::stateChanged, plots, &PlotView::enableScales);
    connect(dock->annosCheckBox, &QCheckBox::stateChanged, plots, &PlotView::enableAnnotations);
    connect(dock->annosCheckBox, &QCheckBox::stateChanged, dock, &SpectrogramControls::enableAnnotations);
    connect(dock->commentsCheckBox, &QCheckBox::stateChanged, plots, &PlotView::enableAnnotationCommentsTooltips);
    connect(dock->cursorSymbolsSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), plots, &PlotView::setCursorSegments);

    // Connect toolbar actions to dock checkboxes (bidirectional sync)
    connect(toggleCursorsAction, &QAction::toggled, dock->cursorsCheckBox, &QCheckBox::setChecked);
    connect(dock->cursorsCheckBox, &QCheckBox::toggled, toggleCursorsAction, &QAction::setChecked);
    connect(toggleScalesAction, &QAction::toggled, dock->scalesCheckBox, &QCheckBox::setChecked);
    connect(dock->scalesCheckBox, &QCheckBox::toggled, toggleScalesAction, &QAction::setChecked);

    // Connect dock outputs
    connect(plots, &PlotView::timeSelectionChanged, dock, &SpectrogramControls::timeSelectionChanged);
    connect(plots, &PlotView::zoomIn, dock, &SpectrogramControls::zoomIn);
    connect(plots, &PlotView::zoomOut, dock, &SpectrogramControls::zoomOut);

    // Update status bar on zoom changes
    connect(dock, static_cast<void (SpectrogramControls::*)(int, int)>(&SpectrogramControls::fftOrZoomChanged), this, [this](int fftSize, int zoomLevel) {
        statusZoomLabel->setText(QString("  Zoom: %1×  ").arg(zoomLevel));
    });

    // Set defaults after making connections so everything is in sync
    dock->setDefaults();

}

void MainWindow::openFile(QString fileName)
{
    QString title="%1: %2";
    this->setWindowTitle(title.arg(QApplication::applicationName(),fileName.section('/',-1,-1)));

    // Update status bar
    statusFileLabel->setText(QString("  %1  ").arg(fileName.section('/',-1,-1)));

    // Try to parse osmocom_fft filenames and extract the sample rate and center frequency.
    // Example file name: "name-f2.411200e+09-s5.000000e+06-t20160807180210.cfile"
    QRegularExpression rx(QRegularExpression::anchoredPattern("(.*)-f(.*)-s(.*)-.*\\.cfile"));
    QString basename = fileName.section('/',-1,-1);

    auto match = rx.match(basename);
    if (match.hasMatch()) {
        QString centerfreq = match.captured(2);
        QString samplerate = match.captured(3);

        std::stringstream ss(samplerate.toUtf8().constData());

        // Needs to be a double as the number is in scientific format
        double rate;
        ss >> rate;
        if (!ss.fail()) {
            setSampleRate(rate);
        }
    }

    try
    {
        input->openFile(fileName.toUtf8().constData());
    }
    catch (const std::exception &ex)
    {
        QMessageBox msgBox(QMessageBox::Critical, "spectr openFile error", QString("%1: %2").arg(fileName).arg(ex.what()));
        msgBox.exec();
    }
}

void MainWindow::invalidateEvent()
{
    plots->setSampleRate(input->rate());

    // Update status bar rate
    statusRateLabel->setText(QString("  Rate: %1 Hz  ").arg(QString::number(input->rate(), 'f', 0)));

    // Only update the text box if it is not already representing
    // the current value. Otherwise the cursor might jump or the
    // representation might change (e.g. to scientific).
    double currentValue = dock->sampleRate->text().toDouble();
    if(QString::number(input->rate()) != QString::number(currentValue)) {
        setSampleRate(input->rate());
    }
}

void MainWindow::setSampleRate(QString rate)
{
    auto sampleRate = rate.toDouble();
    input->setSampleRate(sampleRate);
    plots->setSampleRate(sampleRate);

    // Save the sample rate in settings as we're likely to be opening the same file across multiple runs
    QSettings settings;
    settings.setValue("SampleRate", sampleRate);
}

void MainWindow::setSampleRate(double rate)
{
    dock->sampleRate->setText(QString::number(rate));
}

void MainWindow::setFormat(QString fmt)
{
    input->setFormat(fmt.toUtf8().constData());
}

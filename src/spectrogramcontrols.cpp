/*
 *  Copyright (C) 2015, Mike Walters <mike@flomp.net>
 *  Copyright (C) 2015, Jared Boone <jared@sharebrained.com>
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

#include "spectrogramcontrols.h"
#include <QIntValidator>
#include <QFileDialog>
#include <QSettings>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <cmath>
#include "util.h"

SpectrogramControls::SpectrogramControls(const QString & title, QWidget * parent)
    : QDockWidget::QDockWidget(title, parent)
{
    widget = new QWidget(this);

    // Main vertical layout for the dock
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // ── Open File Button ─────────────────────────────────────────────────────
    fileOpenButton = new QPushButton(QString::fromUtf8("\xF0\x9F\x93\x82  Open file..."), widget);
    fileOpenButton->setMinimumHeight(36);
    mainLayout->addWidget(fileOpenButton);

    // ── Sample Rate ──────────────────────────────────────────────────────────
    layout = new QFormLayout();
    layout->setSpacing(6);
    sampleRate = new QLineEdit();
    auto double_validator = new QDoubleValidator(this);
    double_validator->setBottom(0.0);
    sampleRate->setValidator(double_validator);
    layout->addRow(new QLabel(tr("Sample rate:")), sampleRate);
    mainLayout->addLayout(layout);

    // ── Spectrogram Group ────────────────────────────────────────────────────
    QGroupBox *spectrogramGroup = new QGroupBox(tr("Spectrogram"), widget);
    QFormLayout *specLayout = new QFormLayout(spectrogramGroup);
    specLayout->setSpacing(6);
    specLayout->setContentsMargins(10, 16, 10, 10);

    // FFT size slider + value label
    QHBoxLayout *fftRow = new QHBoxLayout();
    fftSizeSlider = new QSlider(Qt::Horizontal, widget);
    fftSizeSlider->setRange(4, 13);
    fftSizeSlider->setPageStep(1);
    fftSizeLabel = new QLabel(tr("512"), widget);
    fftSizeLabel->setMinimumWidth(48);
    fftSizeLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
    fftRow->addWidget(fftSizeSlider);
    fftRow->addWidget(fftSizeLabel);
    specLayout->addRow(new QLabel(tr("FFT size:")), fftRow);

    // Zoom slider + value label
    QHBoxLayout *zoomRow = new QHBoxLayout();
    zoomLevelSlider = new QSlider(Qt::Horizontal, widget);
    zoomLevelSlider->setRange(0, 10);
    zoomLevelSlider->setPageStep(1);
    zoomLevelLabel = new QLabel(tr("1×"), widget);
    zoomLevelLabel->setMinimumWidth(48);
    zoomLevelLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
    zoomRow->addWidget(zoomLevelSlider);
    zoomRow->addWidget(zoomLevelLabel);
    specLayout->addRow(new QLabel(tr("Zoom:")), zoomRow);

    // Power max slider + value label
    QHBoxLayout *pmaxRow = new QHBoxLayout();
    powerMaxSlider = new QSlider(Qt::Horizontal, widget);
    powerMaxSlider->setRange(-140, 10);
    powerMaxLabel = new QLabel(tr("0 dB"), widget);
    powerMaxLabel->setMinimumWidth(56);
    powerMaxLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
    pmaxRow->addWidget(powerMaxSlider);
    pmaxRow->addWidget(powerMaxLabel);
    specLayout->addRow(new QLabel(tr("Power max:")), pmaxRow);

    // Power min slider + value label
    QHBoxLayout *pminRow = new QHBoxLayout();
    powerMinSlider = new QSlider(Qt::Horizontal, widget);
    powerMinSlider->setRange(-140, 10);
    powerMinLabel = new QLabel(tr("-100 dB"), widget);
    powerMinLabel->setMinimumWidth(56);
    powerMinLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
    pminRow->addWidget(powerMinSlider);
    pminRow->addWidget(powerMinLabel);
    specLayout->addRow(new QLabel(tr("Power min:")), pminRow);

    // Scales checkbox
    scalesCheckBox = new QCheckBox(widget);
    scalesCheckBox->setCheckState(Qt::Checked);
    specLayout->addRow(new QLabel(tr("Scales:")), scalesCheckBox);

    mainLayout->addWidget(spectrogramGroup);

    // ── Time Selection Group ─────────────────────────────────────────────────
    QGroupBox *timeGroup = new QGroupBox(tr("Time Selection"), widget);
    QFormLayout *timeLayout = new QFormLayout(timeGroup);
    timeLayout->setSpacing(6);
    timeLayout->setContentsMargins(10, 16, 10, 10);

    cursorsCheckBox = new QCheckBox(widget);
    timeLayout->addRow(new QLabel(tr("Enable cursors:")), cursorsCheckBox);

    cursorSymbolsSpinBox = new QSpinBox();
    cursorSymbolsSpinBox->setMinimum(1);
    cursorSymbolsSpinBox->setMaximum(99999);
    timeLayout->addRow(new QLabel(tr("Symbols:")), cursorSymbolsSpinBox);

    rateLabel = new QLabel();
    timeLayout->addRow(new QLabel(tr("Rate:")), rateLabel);

    periodLabel = new QLabel();
    timeLayout->addRow(new QLabel(tr("Period:")), periodLabel);

    symbolRateLabel = new QLabel();
    timeLayout->addRow(new QLabel(tr("Symbol rate:")), symbolRateLabel);

    symbolPeriodLabel = new QLabel();
    timeLayout->addRow(new QLabel(tr("Symbol period:")), symbolPeriodLabel);

    mainLayout->addWidget(timeGroup);

    // ── SigMF Control Group ──────────────────────────────────────────────────
    QGroupBox *sigmfGroup = new QGroupBox(tr("SigMF Control"), widget);
    QFormLayout *sigmfLayout = new QFormLayout(sigmfGroup);
    sigmfLayout->setSpacing(6);
    sigmfLayout->setContentsMargins(10, 16, 10, 10);

    annosCheckBox = new QCheckBox(widget);
    sigmfLayout->addRow(new QLabel(tr("Annotations:")), annosCheckBox);

    commentsCheckBox = new QCheckBox(widget);
    sigmfLayout->addRow(new QLabel(tr("Comment tooltips:")), commentsCheckBox);

    mainLayout->addWidget(sigmfGroup);

    // Push everything up
    mainLayout->addStretch(1);

    widget->setLayout(mainLayout);
    setWidget(widget);

    // ── Connections ───────────────────────────────────────────────────────────
    connect(fftSizeSlider, &QSlider::valueChanged, this, &SpectrogramControls::fftSizeChanged);
    connect(zoomLevelSlider, &QSlider::valueChanged, this, &SpectrogramControls::zoomLevelChanged);
    connect(fileOpenButton, &QPushButton::clicked, this, &SpectrogramControls::fileOpenButtonClicked);
    connect(cursorsCheckBox, &QCheckBox::stateChanged, this, &SpectrogramControls::cursorsStateChanged);
    connect(powerMinSlider, &QSlider::valueChanged, this, &SpectrogramControls::powerMinChanged);
    connect(powerMaxSlider, &QSlider::valueChanged, this, &SpectrogramControls::powerMaxChanged);

    // Update value labels when sliders change
    connect(fftSizeSlider, &QSlider::valueChanged, this, [this](int value) {
        fftSizeLabel->setText(QString::number((int)pow(2, value)));
    });
    connect(zoomLevelSlider, &QSlider::valueChanged, this, [this](int value) {
        zoomLevelLabel->setText(QString("%1×").arg((int)pow(2, value)));
    });
    connect(powerMaxSlider, &QSlider::valueChanged, this, [this](int value) {
        powerMaxLabel->setText(QString("%1 dB").arg(value));
    });
    connect(powerMinSlider, &QSlider::valueChanged, this, [this](int value) {
        powerMinLabel->setText(QString("%1 dB").arg(value));
    });
}

void SpectrogramControls::clearCursorLabels()
{
    periodLabel->setText("");
    rateLabel->setText("");
    symbolPeriodLabel->setText("");
    symbolRateLabel->setText("");
}

void SpectrogramControls::cursorsStateChanged(int state)
{
    if (state == Qt::Unchecked) {
        clearCursorLabels();
    }
}

void SpectrogramControls::setDefaults()
{
    fftOrZoomChanged();

    cursorsCheckBox->setCheckState(Qt::Unchecked);
    cursorSymbolsSpinBox->setValue(1);

    annosCheckBox->setCheckState(Qt::Checked);
    commentsCheckBox->setCheckState(Qt::Checked);

    // Try to set the sample rate from the last-used value
    QSettings settings;
    int savedSampleRate = settings.value("SampleRate", 8000000).toInt();
    sampleRate->setText(QString::number(savedSampleRate));
    fftSizeSlider->setValue(settings.value("FFTSize", 9).toInt());
    powerMaxSlider->setValue(settings.value("PowerMax", 0).toInt());
    powerMinSlider->setValue(settings.value("PowerMin", -100).toInt());
    zoomLevelSlider->setValue(settings.value("ZoomLevel", 0).toInt());
}

void SpectrogramControls::fftOrZoomChanged(void)
{
    int fftSize = pow(2, fftSizeSlider->value());
    int zoomLevel = std::min(fftSize, (int)pow(2, zoomLevelSlider->value()));
    emit fftOrZoomChanged(fftSize, zoomLevel);
}

void SpectrogramControls::fftSizeChanged(int value)
{
    QSettings settings;
    settings.setValue("FFTSize", value);
    fftOrZoomChanged();
}

void SpectrogramControls::zoomLevelChanged(int value)
{
    QSettings settings;
    settings.setValue("ZoomLevel", value);
    fftOrZoomChanged();
}

void SpectrogramControls::powerMinChanged(int value)
{
    QSettings settings;
    settings.setValue("PowerMin", value);
}

void SpectrogramControls::powerMaxChanged(int value)
{
    QSettings settings;
    settings.setValue("PowerMax", value);
}

void SpectrogramControls::fileOpenButtonClicked()
{
    QSettings settings;
    QString fileName;
    QFileDialog fileSelect(this);
    fileSelect.setNameFilter(tr("All files (*);;"
                "complex<float> file (*.cfile *.cf32 *.fc32);;"
                "complex<int8> HackRF file (*.cs8 *.sc8 *.c8);;"
                "complex<int16> Fancy file (*.cs16 *.sc16 *.c16);;"
                "complex<uint8> RTL-SDR file (*.cu8 *.uc8)"));

    // Try and load a saved state
    {
        QByteArray savedState = settings.value("OpenFileState").toByteArray();
        fileSelect.restoreState(savedState);

        // Filter doesn't seem to be considered part of the saved state
        QString lastUsedFilter = settings.value("OpenFileFilter").toString();
        if(lastUsedFilter.size())
            fileSelect.selectNameFilter(lastUsedFilter);
    }

    if(fileSelect.exec())
    {
        fileName = fileSelect.selectedFiles()[0];

        // Remember the state of the dialog for the next time
        QByteArray dialogState = fileSelect.saveState();
        settings.setValue("OpenFileState", dialogState);
        settings.setValue("OpenFileFilter", fileSelect.selectedNameFilter());
    }

    if (!fileName.isEmpty())
        emit openFile(fileName);
}

void SpectrogramControls::timeSelectionChanged(float time)
{
    if (cursorsCheckBox->checkState() == Qt::Checked) {
        periodLabel->setText(QString::fromStdString(formatSIValue(time)) + "s");
        rateLabel->setText(QString::fromStdString(formatSIValue(1 / time)) + "Hz");

        int symbols = cursorSymbolsSpinBox->value();
        symbolPeriodLabel->setText(QString::fromStdString(formatSIValue(time / symbols)) + "s");
        symbolRateLabel->setText(QString::fromStdString(formatSIValue(symbols / time)) + "Bd");
    }
}

void SpectrogramControls::zoomIn()
{
    zoomLevelSlider->setValue(zoomLevelSlider->value() + 1);
}

void SpectrogramControls::zoomOut()
{
    zoomLevelSlider->setValue(zoomLevelSlider->value() - 1);
}

void SpectrogramControls::enableAnnotations(bool enabled) {
    // disable annotation comments checkbox when annotations are disabled
    commentsCheckBox->setEnabled(enabled);
}

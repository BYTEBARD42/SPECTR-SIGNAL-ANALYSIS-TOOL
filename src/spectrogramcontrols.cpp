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
#include <QHBoxLayout>
#include <cmath>
#include "util.h"

SpectrogramControls::SpectrogramControls(const QString & title, QWidget * parent)
    : QDockWidget::QDockWidget(title, parent)
{
    // Scrollable container so controls never get clipped
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    widget = new QWidget(scrollArea);
    mainLayout = new QVBoxLayout(widget);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // ── Helper to create a slider row with a value label ────────────
    auto makeSliderRow = [](QFormLayout *form, const QString &label,
                            QSlider *slider, QLabel *&valueLabel) {
        QWidget *row = new QWidget();
        QHBoxLayout *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(8);
        rowLayout->addWidget(slider, 1);
        valueLabel = new QLabel("0");
        valueLabel->setObjectName("valueLabel");
        valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        valueLabel->setMinimumWidth(48);
        rowLayout->addWidget(valueLabel);
        form->addRow(new QLabel(label), row);
    };

    // ═══════════════════════════════════════════════════════════════
    // FILE group
    // ═══════════════════════════════════════════════════════════════
    QGroupBox *fileGroup = new QGroupBox(tr("File"), widget);
    QFormLayout *fileLayout = new QFormLayout(fileGroup);
    fileLayout->setSpacing(6);
    fileLayout->setContentsMargins(10, 18, 10, 10);

    fileOpenButton = new QPushButton(tr("📂  Open file..."), widget);
    fileOpenButton->setCursor(Qt::PointingHandCursor);
    fileLayout->addRow(fileOpenButton);

    sampleRate = new QLineEdit();
    auto double_validator = new QDoubleValidator(this);
    double_validator->setBottom(0.0);
    sampleRate->setValidator(double_validator);
    sampleRate->setPlaceholderText(tr("e.g. 8000000"));
    fileLayout->addRow(new QLabel(tr("Sample rate:")), sampleRate);

    mainLayout->addWidget(fileGroup);

    // ═══════════════════════════════════════════════════════════════
    // SPECTROGRAM group
    // ═══════════════════════════════════════════════════════════════
    QGroupBox *spectroGroup = new QGroupBox(tr("Spectrogram"), widget);
    QFormLayout *spectroLayout = new QFormLayout(spectroGroup);
    spectroLayout->setSpacing(6);
    spectroLayout->setContentsMargins(10, 18, 10, 10);

    fftSizeSlider = new QSlider(Qt::Horizontal, widget);
    fftSizeSlider->setRange(4, 13);
    fftSizeSlider->setPageStep(1);
    makeSliderRow(spectroLayout, tr("FFT size:"), fftSizeSlider, fftSizeValueLabel);

    zoomLevelSlider = new QSlider(Qt::Horizontal, widget);
    zoomLevelSlider->setRange(0, 10);
    zoomLevelSlider->setPageStep(1);
    makeSliderRow(spectroLayout, tr("Zoom:"), zoomLevelSlider, zoomValueLabel);

    powerMaxSlider = new QSlider(Qt::Horizontal, widget);
    powerMaxSlider->setRange(-140, 10);
    makeSliderRow(spectroLayout, tr("Power max:"), powerMaxSlider, powerMaxValueLabel);

    powerMinSlider = new QSlider(Qt::Horizontal, widget);
    powerMinSlider->setRange(-140, 10);
    makeSliderRow(spectroLayout, tr("Power min:"), powerMinSlider, powerMinValueLabel);

    scalesCheckBox = new QCheckBox(tr("Show scales"), widget);
    scalesCheckBox->setCheckState(Qt::Checked);
    spectroLayout->addRow(scalesCheckBox);

    mainLayout->addWidget(spectroGroup);

    // ═══════════════════════════════════════════════════════════════
    // TIME SELECTION group
    // ═══════════════════════════════════════════════════════════════
    QGroupBox *timeGroup = new QGroupBox(tr("Time Selection"), widget);
    QFormLayout *timeLayout = new QFormLayout(timeGroup);
    timeLayout->setSpacing(6);
    timeLayout->setContentsMargins(10, 18, 10, 10);

    cursorsCheckBox = new QCheckBox(tr("Enable cursors"), widget);
    timeLayout->addRow(cursorsCheckBox);

    cursorSymbolsSpinBox = new QSpinBox();
    cursorSymbolsSpinBox->setMinimum(1);
    cursorSymbolsSpinBox->setMaximum(99999);
    timeLayout->addRow(new QLabel(tr("Symbols:")), cursorSymbolsSpinBox);

    rateLabel = new QLabel();
    rateLabel->setObjectName("valueLabel");
    timeLayout->addRow(new QLabel(tr("Rate:")), rateLabel);

    periodLabel = new QLabel();
    periodLabel->setObjectName("valueLabel");
    timeLayout->addRow(new QLabel(tr("Period:")), periodLabel);

    symbolRateLabel = new QLabel();
    symbolRateLabel->setObjectName("valueLabel");
    timeLayout->addRow(new QLabel(tr("Symbol rate:")), symbolRateLabel);

    symbolPeriodLabel = new QLabel();
    symbolPeriodLabel->setObjectName("valueLabel");
    timeLayout->addRow(new QLabel(tr("Symbol period:")), symbolPeriodLabel);

    mainLayout->addWidget(timeGroup);

    // ═══════════════════════════════════════════════════════════════
    // SIGMF CONTROL group
    // ═══════════════════════════════════════════════════════════════
    QGroupBox *sigmfGroup = new QGroupBox(tr("SigMF Control"), widget);
    QFormLayout *sigmfLayout = new QFormLayout(sigmfGroup);
    sigmfLayout->setSpacing(6);
    sigmfLayout->setContentsMargins(10, 18, 10, 10);

    annosCheckBox = new QCheckBox(tr("Display annotations"), widget);
    sigmfLayout->addRow(annosCheckBox);

    commentsCheckBox = new QCheckBox(tr("Annotation comment tooltips"), widget);
    sigmfLayout->addRow(commentsCheckBox);

    mainLayout->addWidget(sigmfGroup);

    // ═══════════════════════════════════════════════════════════════
    // SIGNAL ANALYSIS group
    // ═══════════════════════════════════════════════════════════════
    QGroupBox *analysisGroup = new QGroupBox(tr("Signal Analysis"), widget);
    QFormLayout *analysisLayout = new QFormLayout(analysisGroup);
    analysisLayout->setSpacing(6);
    analysisLayout->setContentsMargins(10, 18, 10, 10);

    signalPowerLabel = new QLabel(tr("—"));
    signalPowerLabel->setObjectName("valueLabel");
    analysisLayout->addRow(new QLabel(tr("Signal power:")), signalPowerLabel);

    noiseFloorLabel = new QLabel(tr("—"));
    noiseFloorLabel->setObjectName("valueLabel");
    analysisLayout->addRow(new QLabel(tr("Noise floor:")), noiseFloorLabel);

    snrValueLabel = new QLabel(tr("—"));
    snrValueLabel->setObjectName("valueLabel");
    analysisLayout->addRow(new QLabel(tr("SNR:")), snrValueLabel);

    QLabel *snrHint = new QLabel(tr("Enable cursors to measure"));
    snrHint->setStyleSheet("color: #585b70; font-size: 7pt; font-style: italic;");
    analysisLayout->addRow(snrHint);

    mainLayout->addWidget(analysisGroup);

    // Push remaining space to the bottom
    mainLayout->addStretch(1);

    widget->setLayout(mainLayout);
    scrollArea->setWidget(widget);
    setWidget(scrollArea);

    // ── Connections ─────────────────────────────────────────────────
    connect(fftSizeSlider, &QSlider::valueChanged, this, &SpectrogramControls::fftSizeChanged);
    connect(zoomLevelSlider, &QSlider::valueChanged, this, &SpectrogramControls::zoomLevelChanged);
    connect(fileOpenButton, &QPushButton::clicked, this, &SpectrogramControls::fileOpenButtonClicked);
    connect(cursorsCheckBox, &QCheckBox::stateChanged, this, &SpectrogramControls::cursorsStateChanged);
    connect(powerMinSlider, &QSlider::valueChanged, this, &SpectrogramControls::powerMinChanged);
    connect(powerMaxSlider, &QSlider::valueChanged, this, &SpectrogramControls::powerMaxChanged);

    // Value label update connections
    connect(fftSizeSlider, &QSlider::valueChanged, this, &SpectrogramControls::updateFFTSizeLabel);
    connect(zoomLevelSlider, &QSlider::valueChanged, this, &SpectrogramControls::updateZoomLabel);
    connect(powerMaxSlider, &QSlider::valueChanged, this, &SpectrogramControls::updatePowerMaxLabel);
    connect(powerMinSlider, &QSlider::valueChanged, this, &SpectrogramControls::updatePowerMinLabel);
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

void SpectrogramControls::updateFFTSizeLabel(int value)
{
    int fftSize = pow(2, value);
    fftSizeValueLabel->setText(QString::number(fftSize));
}

void SpectrogramControls::updateZoomLabel(int value)
{
    int zoom = pow(2, value);
    zoomValueLabel->setText(QString::number(zoom) + "x");
}

void SpectrogramControls::updatePowerMaxLabel(int value)
{
    powerMaxValueLabel->setText(QString::number(value) + " dB");
}

void SpectrogramControls::updatePowerMinLabel(int value)
{
    powerMinValueLabel->setText(QString::number(value) + " dB");
}

void SpectrogramControls::updateSNRAnalysis(float signalPower, float noisePower, float snr)
{
    signalPowerLabel->setText(QString::number(signalPower, 'f', 1) + " dBFS");
    noiseFloorLabel->setText(QString::number(noisePower, 'f', 1) + " dBFS");

    // Color the SNR value based on quality
    QString color;
    if (snr >= 20.0f)
        color = "#a6e3a1";  // Green - good
    else if (snr >= 10.0f)
        color = "#f9e2af";  // Yellow - moderate
    else
        color = "#f38ba8";  // Red - poor

    snrValueLabel->setText(QString::number(snr, 'f', 1) + " dB");
    snrValueLabel->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 9pt;").arg(color));
}

void SpectrogramControls::clearSNRLabels()
{
    signalPowerLabel->setText(tr("—"));
    noiseFloorLabel->setText(tr("—"));
    snrValueLabel->setText(tr("—"));
    snrValueLabel->setStyleSheet("");  // Reset to default style
}

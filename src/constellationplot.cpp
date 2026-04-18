/*
 *  Constellation diagram plot for IQ signal visualization.
 *  Plots In-phase (I) vs Quadrature (Q) samples as a scatter diagram.
 *
 *  This file is part of spectr.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 */

#include "constellationplot.h"
#include <QPainterPath>
#include <cmath>

ConstellationPlot::ConstellationPlot(std::shared_ptr<AbstractSampleSource> source)
    : Plot(source)
{
    setHeight(300); // Taller than default to give a nice square-ish plot area
}

void ConstellationPlot::paintBack(QPainter &painter, QRect &rect, range_t<size_t> sampleRange)
{
    // Dark background for the constellation area
    painter.save();
    painter.fillRect(rect, QColor(10, 10, 18));
    painter.restore();
}

void ConstellationPlot::paintMid(QPainter &painter, QRect &rect, range_t<size_t> sampleRange)
{
    if (sampleRange.length() == 0) return;

    // Cast to complex sample source
    auto src = dynamic_cast<SampleSource<std::complex<float>>*>(sampleSource.get());
    if (!src) return;

    // Determine how many samples to grab (cap for performance)
    size_t count = std::min((size_t)sampleRange.length(), maxSamples);
    size_t start = sampleRange.minimum;

    // If we have more samples than maxSamples, take evenly spaced ones
    // by adjusting the start to center the selection
    if (sampleRange.length() > maxSamples) {
        start = sampleRange.minimum + (sampleRange.length() - maxSamples) / 2;
    }

    auto samples = src->getSamples(start, count);
    if (samples == nullptr) return;

    // Calculate a square plot area centered in the rect
    int plotSize = std::min(rect.width(), rect.height()) - 20;
    if (plotSize < 50) return;

    QRect plotArea(
        rect.x() + (rect.width() - plotSize) / 2,
        rect.y() + (rect.height() - plotSize) / 2,
        plotSize,
        plotSize
    );

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    drawGrid(painter, plotArea);
    drawConstellation(painter, plotArea, samples.get(), count);

    // Draw title
    painter.setPen(QColor(86, 212, 200));
    QFont titleFont = painter.font();
    titleFont.setPointSize(9);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(plotArea.left(), plotArea.top() - 6, "I/Q Constellation");

    // Draw axis labels
    QFont labelFont = painter.font();
    labelFont.setPointSize(7);
    labelFont.setBold(false);
    painter.setFont(labelFont);
    painter.setPen(QColor(166, 173, 200));
    painter.drawText(plotArea.right() - 30, plotArea.center().y() + plotSize / 2 + 14, "I →");
    painter.drawText(plotArea.center().x() - plotSize / 2 - 4, plotArea.top() - 2, "Q ↑");

    painter.restore();
}

void ConstellationPlot::drawGrid(QPainter &painter, const QRect &plotArea)
{
    // Semi-transparent grid background
    painter.fillRect(plotArea, QColor(18, 18, 30));

    // Border
    QPen borderPen(QColor(49, 50, 68), 1);
    painter.setPen(borderPen);
    painter.drawRect(plotArea);

    // Grid lines
    QPen gridPen(QColor(49, 50, 68, 120), 1, Qt::DotLine);
    painter.setPen(gridPen);

    int cx = plotArea.center().x();
    int cy = plotArea.center().y();
    int halfW = plotArea.width() / 2;
    int halfH = plotArea.height() / 2;

    // Draw quarter grid lines
    for (int i = 1; i <= 3; i++) {
        int offsetX = (halfW * i) / 4;
        int offsetY = (halfH * i) / 4;

        // Vertical
        painter.drawLine(cx + offsetX, plotArea.top(), cx + offsetX, plotArea.bottom());
        painter.drawLine(cx - offsetX, plotArea.top(), cx - offsetX, plotArea.bottom());
        // Horizontal
        painter.drawLine(plotArea.left(), cy + offsetY, plotArea.right(), cy + offsetY);
        painter.drawLine(plotArea.left(), cy - offsetY, plotArea.right(), cy - offsetY);
    }

    // Draw center cross (axes) more prominently
    QPen axisPen(QColor(69, 71, 90), 1, Qt::SolidLine);
    painter.setPen(axisPen);
    painter.drawLine(cx, plotArea.top(), cx, plotArea.bottom());    // Q axis (vertical)
    painter.drawLine(plotArea.left(), cy, plotArea.right(), cy);     // I axis (horizontal)

    // Draw unit circle
    QPen circlePen(QColor(86, 212, 200, 60), 1, Qt::DashLine);
    painter.setPen(circlePen);
    int radius = std::min(halfW, halfH);
    painter.drawEllipse(QPoint(cx, cy), radius, radius);

    // Draw half-unit circle
    QPen halfCirclePen(QColor(86, 212, 200, 30), 1, Qt::DotLine);
    painter.setPen(halfCirclePen);
    painter.drawEllipse(QPoint(cx, cy), radius / 2, radius / 2);
}

void ConstellationPlot::drawConstellation(QPainter &painter, const QRect &plotArea,
                                           std::complex<float> *samples, size_t count)
{
    if (count == 0) return;

    int cx = plotArea.center().x();
    int cy = plotArea.center().y();
    int halfW = plotArea.width() / 2;
    int halfH = plotArea.height() / 2;

    // Find the maximum amplitude for scaling
    float maxAmp = 0.0f;
    for (size_t i = 0; i < count; i++) {
        float amp = std::abs(samples[i]);
        if (amp > maxAmp) maxAmp = amp;
    }

    if (maxAmp < 1e-10f) return; // All zeros, nothing to plot

    // Scale factor: map [-maxAmp, maxAmp] to [-halfW, halfW]
    // Use 90% of the area to leave a small margin
    float scale = 0.9f * std::min(halfW, halfH) / maxAmp;

    // Draw each sample point as a small dot
    // Use a semi-transparent color for density visualization
    for (size_t i = 0; i < count; i++) {
        float I = samples[i].real();
        float Q = samples[i].imag();

        int px = cx + (int)(I * scale);
        int py = cy - (int)(Q * scale);  // Flip Y because screen coordinates are inverted

        // Clip to plot area
        if (px < plotArea.left() || px > plotArea.right() ||
            py < plotArea.top() || py > plotArea.bottom())
            continue;

        // Color based on amplitude (brighter = higher amplitude)
        float amp = std::abs(samples[i]) / maxAmp;
        int alpha = 80 + (int)(175 * amp);
        int green = 180 + (int)(75 * amp);
        int blue = 200 + (int)(55 * amp);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(86, green, blue, alpha));
        painter.drawEllipse(QPoint(px, py), 2, 2);
    }

    // Draw sample count info
    QFont infoFont = painter.font();
    infoFont.setPointSize(7);
    painter.setFont(infoFont);
    painter.setPen(QColor(166, 173, 200, 180));
    painter.setBrush(Qt::NoBrush);
    QString info = QString("Samples: %1 | Max amp: %2")
                       .arg(count)
                       .arg(QString::number(maxAmp, 'f', 3));
    painter.drawText(plotArea.left(), plotArea.bottom() + 14, info);
}

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

#pragma once

#include <complex>
#include <memory>
#include "abstractsamplesource.h"
#include "plot.h"
#include "samplesource.h"
#include "util.h"

class ConstellationPlot : public Plot
{
    Q_OBJECT

public:
    ConstellationPlot(std::shared_ptr<AbstractSampleSource> source);

    void paintBack(QPainter &painter, QRect &rect, range_t<size_t> sampleRange) override;
    void paintMid(QPainter &painter, QRect &rect, range_t<size_t> sampleRange) override;

private:
    // Maximum number of samples to plot (for performance)
    static const size_t maxSamples = 8192;

    void drawGrid(QPainter &painter, const QRect &plotArea);
    void drawConstellation(QPainter &painter, const QRect &plotArea,
                           std::complex<float> *samples, size_t count);
};

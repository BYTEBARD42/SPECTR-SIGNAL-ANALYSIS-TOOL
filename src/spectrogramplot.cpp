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

#include "spectrogramplot.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmapCache>
#include <QRect>
#include <liquid/liquid.h>
#include <algorithm>
#include <functional>
#include <cstdlib>
#include <limits>
#include "stylesheet.h"
#include "util.h"


SpectrogramPlot::SpectrogramPlot(std::shared_ptr<SampleSource<std::complex<float>>> src) : Plot(src), inputSource(src), fftSize(512), tuner(fftSize, this)
{
    setFFTSize(fftSize);
    zoomLevel = 1;
    powerMax = 0.0f;
    powerMin = -50.0f;
    sampleRate = 0;
    frequencyScaleEnabled = false;
    sigmfAnnotationsEnabled = true;

    // Viridis perceptually-uniform colormap (256 entries)
    // Dark purple → teal → yellow — industry standard for scientific visualization
    static const uint8_t viridis_r[256] = {
        68,68,69,69,69,69,69,69,69,70,70,70,70,70,70,70,71,71,71,71,71,71,71,71,71,71,71,71,71,72,72,72,
        72,72,72,72,72,72,72,72,71,71,71,71,71,71,71,71,71,71,70,70,70,70,70,69,69,69,69,68,68,68,67,67,
        67,66,66,66,65,65,64,64,63,63,63,62,62,61,61,60,60,59,59,58,58,57,57,56,55,55,54,54,53,52,52,51,
        51,50,49,49,48,48,47,46,46,45,45,44,43,43,42,42,41,41,40,40,39,39,38,38,37,37,37,36,36,36,35,35,
        35,35,35,35,35,35,35,35,35,35,36,36,36,37,37,38,38,39,40,40,41,42,43,44,45,46,47,48,49,50,52,53,
        54,56,57,59,60,62,63,65,67,68,70,72,74,76,78,79,81,83,86,88,90,92,94,96,99,101,103,106,108,110,113,115,
        117,120,122,125,127,130,132,135,137,140,142,145,148,150,153,155,158,161,163,166,169,171,174,177,179,182,185,188,190,193,196,199,
        201,204,207,210,212,215,218,221,224,226,229,232,235,238,240,243,246,248,250,253,254,254,254,253,252,251,250,248,246,245,243,241
    };
    static const uint8_t viridis_g[256] = {
        1,2,3,5,6,8,9,11,12,14,15,17,18,20,21,22,24,25,26,28,29,30,32,33,34,35,36,37,39,40,41,42,
        43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,63,64,65,66,67,68,69,69,70,71,72,
        72,73,74,74,75,76,76,77,78,78,79,79,80,81,81,82,82,83,83,84,84,85,85,86,86,87,87,88,88,88,89,89,
        90,90,90,91,91,91,92,92,92,93,93,93,94,94,94,94,95,95,95,96,96,96,97,97,97,97,98,98,98,99,99,99,
        100,100,101,101,101,102,102,103,103,103,104,104,105,105,106,106,107,107,108,108,109,110,110,111,111,112,113,113,114,115,115,116,
        117,117,118,119,120,120,121,122,123,123,124,125,126,127,127,128,129,130,131,132,132,133,134,135,136,137,138,139,140,141,142,143,
        144,145,146,147,148,149,150,151,152,153,154,155,156,158,159,160,161,162,163,164,166,167,168,169,170,172,173,174,175,177,178,179,
        181,182,183,185,186,187,189,190,191,193,194,196,197,199,200,201,203,204,206,207,209,210,211,212,213,214,215,215,216,217,218,219
    };
    static const uint8_t viridis_b[256] = {
        84,85,87,88,89,91,92,93,94,95,97,98,99,100,101,102,103,104,105,106,107,108,108,109,110,111,111,112,113,113,114,114,
        115,115,116,116,116,117,117,117,118,118,118,118,118,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,119,118,118,118,
        118,118,117,117,117,117,116,116,116,115,115,115,114,114,113,113,113,112,112,111,111,110,110,109,109,108,107,107,106,106,105,104,
        104,103,102,102,101,100,100,99,98,97,96,96,95,94,93,92,92,91,90,89,88,87,86,86,85,84,83,82,81,80,79,78,
        77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,60,59,58,57,56,55,54,53,51,50,49,48,47,46,45,43,
        42,41,40,39,38,37,36,35,34,33,32,31,30,30,29,28,28,27,27,26,26,26,25,25,25,25,25,25,25,25,26,26,
        27,27,28,29,29,30,31,32,33,35,36,37,39,40,42,44,45,47,49,51,53,55,57,59,61,63,66,68,70,73,75,78,
        80,83,85,88,91,93,96,99,102,104,107,110,113,116,119,122,125,128,131,134,137,139,142,144,146,148,149,151,152,154,155,156
    };

    for (int i = 0; i < 256; i++) {
        colormap[i] = qRgb(viridis_r[i], viridis_g[i], viridis_b[i]);
    }

    tunerTransform = std::make_shared<TunerTransform>(src);
    connect(&tuner, &Tuner::tunerMoved, this, &SpectrogramPlot::tunerMoved);
}

void SpectrogramPlot::invalidateEvent()
{
    // HACK: this makes sure we update the height for real signals (as InputSource is passed here before the file is opened)
    setFFTSize(fftSize);

    pixmapCache.clear();
    fftCache.clear();
    emit repaint();
}

void SpectrogramPlot::paintFront(QPainter &painter, QRect &rect, range_t<size_t> sampleRange)
{
    if (tunerEnabled())
        tuner.paintFront(painter, rect, sampleRange);

    if (frequencyScaleEnabled)
        paintFrequencyScale(painter, rect);

    if (sigmfAnnotationsEnabled)
        paintAnnotations(painter, rect, sampleRange);
}

void SpectrogramPlot::paintFrequencyScale(QPainter &painter, QRect &rect)
{
    if (sampleRate == 0) {
        return;
    }

    if (sampleRate / 2 > UINT64_MAX) {
        return;
    }

    // At which pixel is F_+sampleRate/2
    int y = rect.y();

    int plotHeight = rect.height();
    if (inputSource->realSignal())
        plotHeight *= 2;

    double bwPerPixel = (double)sampleRate / plotHeight;
    int tickHeight = 50;

    uint64_t bwPerTick = 10 * pow(10, floor(log(bwPerPixel * tickHeight) / log(10)));

    if (bwPerTick < 1) {
        return;
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    // Semi-transparent background strip for readability
    painter.fillRect(QRect(0, rect.y(), 100, rect.height()), QColor(10, 10, 20, 160));

    QPen pen(Theme::accent, 1, Qt::SolidLine);
    painter.setPen(pen);
    QFont scaleFont = painter.font();
    scaleFont.setPointSize(scaleFont.pointSize() - 1);
    painter.setFont(scaleFont);
    QFontMetrics fm(painter.font());


    uint64_t tick = 0;

    while (tick <= sampleRate / 2) {

        int tickpy = plotHeight / 2 - tick / bwPerPixel + y;
        int tickny = plotHeight / 2 + tick / bwPerPixel + y;

        if (!inputSource->realSignal())
            painter.drawLine(0, tickny, 30, tickny);
        painter.drawLine(0, tickpy, 30, tickpy);

        if (tick != 0) {
            char buf[128];

            if (bwPerTick % 1000000000 == 0) {
                snprintf(buf, sizeof(buf), "-%lu GHz", tick / 1000000000);
            } else if (bwPerTick % 1000000 == 0) {
                snprintf(buf, sizeof(buf), "-%lu MHz", tick / 1000000);
            } else if(bwPerTick % 1000 == 0) {
                snprintf(buf, sizeof(buf), "-%lu kHz", tick / 1000);
            } else {
                snprintf(buf, sizeof(buf), "-%lu Hz", tick);
            }

            painter.setPen(Theme::textPrimary);
            if (!inputSource->realSignal())
                painter.drawText(5, tickny - 5, buf);

            buf[0] = ' ';
            painter.drawText(5, tickpy + 15, buf);
            painter.setPen(pen);
        }

        tick += bwPerTick;
    }

    // Draw small ticks
    QPen smallTickPen(Theme::accentDim, 1, Qt::SolidLine);
    painter.setPen(smallTickPen);
    bwPerTick /= 10;

    if (bwPerTick >= 1 ) {
        tick = 0;
        while (tick <= sampleRate / 2) {

            int tickpy = plotHeight / 2 - tick / bwPerPixel + y;
            int tickny = plotHeight / 2 + tick / bwPerPixel + y;

            if (!inputSource->realSignal())
                painter.drawLine(0, tickny, 3, tickny);
            painter.drawLine(0, tickpy, 3, tickpy);

            tick += bwPerTick;
        }
    }
    painter.restore();
}

void SpectrogramPlot::paintAnnotations(QPainter &painter, QRect &rect, range_t<size_t> sampleRange)
{
    // Pixel (from the top) at which 0 Hz sits
    int zero = rect.y() + rect.height() / 2;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    QFontMetrics fm(painter.font());

    visibleAnnotationLocations.clear();

    for (int i = 0; i < inputSource->annotationList.size(); i++) {
        Annotation a = inputSource->annotationList.at(i);

        size_t labelLength = fm.boundingRect(a.label).width() * getStride();

        // Check if:
        //  (1) End of annotation (might be maximum, or end of label text) is still visible in time
        //  (2) Part of the annotation is already visible in time
        //
        // Currently there is no check if the annotation is visible in frequency. This is a
        // possible performance improvement
        //
        size_t start = a.sampleRange.minimum;
        size_t end = std::max(a.sampleRange.minimum + labelLength, a.sampleRange.maximum);

        if(start <= sampleRange.maximum && end >= sampleRange.minimum) {

            double frequency = a.frequencyRange.maximum - inputSource->getFrequency();
            int x = (a.sampleRange.minimum - sampleRange.minimum) / getStride();
            int y = zero - frequency / sampleRate * rect.height();
            int height = (a.frequencyRange.maximum - a.frequencyRange.minimum) / sampleRate * rect.height();
            int width = (a.sampleRange.maximum - a.sampleRange.minimum) / getStride();

            // Semi-transparent filled box with accent border
            painter.fillRect(QRect(x, y, width, height), QColor(0, 212, 170, 25));
            QPen annoPen(Theme::accent, 1.5, Qt::SolidLine);
            painter.setPen(annoPen);
            painter.drawRect(x, y, width, height);

            // Label with text shadow for readability
            painter.setPen(QColor(0, 0, 0, 160));
            painter.drawText(x + 1, y - 1, a.label);
            painter.setPen(Theme::textPrimary);
            painter.drawText(x, y - 2, a.label);

            visibleAnnotationLocations.emplace_back(a, x, y, width, height);
        }
    }

    painter.restore();
}

QString *SpectrogramPlot::mouseAnnotationComment(const QMouseEvent *event) {
    auto pos = event->pos();
    int mouse_x = pos.x();
    int mouse_y = pos.y();

    for (auto& a : visibleAnnotationLocations) {
        if (!a.annotation.comment.isEmpty() && a.isInside(mouse_x, mouse_y)) {
            return &a.annotation.comment;
        }
    }
    return nullptr;
}

void SpectrogramPlot::paintMid(QPainter &painter, QRect &rect, range_t<size_t> sampleRange)
{
    if (!inputSource || inputSource->count() == 0)
        return;

    size_t sampleOffset = sampleRange.minimum % (getStride() * linesPerTile());
    size_t tileID = sampleRange.minimum - sampleOffset;
    int xoffset = sampleOffset / getStride();

    // Paint first (possibly partial) tile
    painter.drawPixmap(QRect(rect.left(), rect.y(), linesPerTile() - xoffset, height()), *getPixmapTile(tileID), QRect(xoffset, 0, linesPerTile() - xoffset, height()));
    tileID += getStride() * linesPerTile();

    // Paint remaining tiles
    for (int x = linesPerTile() - xoffset; x < rect.right(); x += linesPerTile()) {
        // TODO: don't draw past rect.right()
        // TODO: handle partial final tile
        painter.drawPixmap(QRect(x, rect.y(), linesPerTile(), height()), *getPixmapTile(tileID), QRect(0, 0, linesPerTile(), height()));
        tileID += getStride() * linesPerTile();
    }
}

QPixmap* SpectrogramPlot::getPixmapTile(size_t tile)
{
    QPixmap *obj = pixmapCache.object(TileCacheKey(fftSize, zoomLevel, tile));
    if (obj != 0)
        return obj;

    float *fftTile = getFFTTile(tile);
    obj = new QPixmap(linesPerTile(), fftSize);
    QImage image(linesPerTile(), fftSize, QImage::Format_RGB32);
    float powerRange = -1.0f / std::abs(int(powerMin - powerMax));
    for (int y = 0; y < fftSize; y++) {
        auto scanLine = (QRgb*)image.scanLine(fftSize - y - 1);
        for (int x = 0; x < linesPerTile(); x++) {
            float *fftLine = &fftTile[x * fftSize];
            float normPower = (fftLine[y] - powerMax) * powerRange;
            normPower = clamp(normPower, 0.0f, 1.0f);

            scanLine[x] = colormap[(uint8_t)(normPower * (256 - 1))];
        }
    }
    obj->convertFromImage(image);
    pixmapCache.insert(TileCacheKey(fftSize, zoomLevel, tile), obj);
    return obj;
}

float* SpectrogramPlot::getFFTTile(size_t tile)
{
    std::array<float, tileSize>* obj = fftCache.object(TileCacheKey(fftSize, zoomLevel, tile));
    if (obj != nullptr)
        return obj->data();

    std::array<float, tileSize>* destStorage = new std::array<float, tileSize>;
    float *ptr = destStorage->data();
    size_t sample = tile;
    while ((ptr - destStorage->data()) < tileSize) {
        getLine(ptr, sample);
        sample += getStride();
        ptr += fftSize;
    }
    fftCache.insert(TileCacheKey(fftSize, zoomLevel, tile), destStorage);
    return destStorage->data();
}

void SpectrogramPlot::getLine(float *dest, size_t sample)
{
    if (inputSource && fft) {
        // Make sample be the midpoint of the FFT, unless this takes us
        // past the beginning of the inputSource (if we remove the
        // std::max(·, 0), then an ugly red bar appears at the beginning
        // of the spectrogram with large zooms and FFT sizes).
        const auto first_sample = std::max(static_cast<ssize_t>(sample) - fftSize / 2,
                        static_cast<ssize_t>(0));
        auto buffer = inputSource->getSamples(first_sample, fftSize);
        if (buffer == nullptr) {
            auto neg_infinity = -1 * std::numeric_limits<float>::infinity();
            for (int i = 0; i < fftSize; i++, dest++)
                *dest = neg_infinity;
            return;
        }

        for (int i = 0; i < fftSize; i++) {
            buffer[i] *= window[i];
        }

        fft->process(buffer.get(), buffer.get());
        const float invFFTSize = 1.0f / fftSize;
        const float logMultiplier = 10.0f / log2f(10.0f);
        for (int i = 0; i < fftSize; i++) {
            // Start from the middle of the FFTW array and wrap
            // to rearrange the data
            int k = i ^ (fftSize >> 1);
            auto s = buffer[k] * invFFTSize;
            float power = s.real() * s.real() + s.imag() * s.imag();
            float logPower = log2f(power) * logMultiplier;
            *dest = logPower;
            dest++;
        }
    }
}

int SpectrogramPlot::getStride()
{
    return fftSize / zoomLevel;
}

float SpectrogramPlot::getTunerPhaseInc()
{
    auto freq = 0.5f - tuner.centre() / (float)fftSize;
    return freq * Tau;
}

std::vector<float> SpectrogramPlot::getTunerTaps()
{
    float cutoff = tuner.deviation() / (float)fftSize;
    float gain = pow(10.0f, powerMax / -10.0f);
    auto atten = 60.0f;
    auto len = estimate_req_filter_len(std::min(cutoff, 0.05f), atten);
    auto taps = std::vector<float>(len);
    liquid_firdes_kaiser(len, cutoff, atten, 0.0f, taps.data());
    std::transform(taps.begin(), taps.end(), taps.begin(),
                   std::bind(std::multiplies<float>(), std::placeholders::_1, gain));
    return taps;
}

int SpectrogramPlot::linesPerTile()
{
    return tileSize / fftSize;
}

bool SpectrogramPlot::mouseEvent(QEvent::Type type, QMouseEvent *event)
{
    if (tunerEnabled())
        return tuner.mouseEvent(type, event);

    return false;
}

void SpectrogramPlot::leaveEvent()
{
    if (tunerEnabled())
        tuner.leaveEvent();
}

std::shared_ptr<AbstractSampleSource> SpectrogramPlot::output()
{
    return tunerTransform;
}

void SpectrogramPlot::setFFTSize(int size)
{
    float sizeScale = float(size) / float(fftSize);
    fftSize = size;
    fft.reset(new FFT(fftSize));

    window.reset(new float[fftSize]);
    for (int i = 0; i < fftSize; i++) {
        window[i] = 0.5f * (1.0f - cos(Tau * i / (fftSize - 1)));
    }

    if (inputSource->realSignal()) {
        setHeight(fftSize/2);
    } else {
        setHeight(fftSize);
    }
    auto dev = tuner.deviation();
    auto centre = tuner.centre();
    tuner.setHeight(height());
    tuner.setDeviation( dev * sizeScale );
    tuner.setCentre( centre * sizeScale );
}

void SpectrogramPlot::setPowerMax(int power)
{
    powerMax = power;
    pixmapCache.clear();
    tunerMoved();
}

void SpectrogramPlot::setPowerMin(int power)
{
    powerMin = power;
    pixmapCache.clear();
}

void SpectrogramPlot::setZoomLevel(int zoom)
{
    zoomLevel = zoom;
}

void SpectrogramPlot::setSampleRate(double rate)
{
    sampleRate = rate;
}

void SpectrogramPlot::enableScales(bool enabled)
{
   frequencyScaleEnabled = enabled;
}

void SpectrogramPlot::enableAnnotations(bool enabled)
{
   sigmfAnnotationsEnabled = enabled;
}

bool SpectrogramPlot::isAnnotationsEnabled(void)
{
    return sigmfAnnotationsEnabled;
}

bool SpectrogramPlot::tunerEnabled()
{
    return (tunerTransform->subscriberCount() > 0);
}

void SpectrogramPlot::tunerMoved()
{
    tunerTransform->setFrequency(getTunerPhaseInc());
    tunerTransform->setTaps(getTunerTaps());
    tunerTransform->setRelativeBandwith(tuner.deviation() * 2.0 / height());

    // TODO: for invalidating traceplot cache, this shouldn't really go here
    QPixmapCache::clear();

    emit repaint();
}

uint qHash(const TileCacheKey &key, uint seed)
{
    return key.fftSize ^ key.zoomLevel ^ key.sample ^ seed;
}

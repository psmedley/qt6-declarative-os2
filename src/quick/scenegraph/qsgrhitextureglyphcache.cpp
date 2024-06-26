/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsgrhitextureglyphcache_p.h"
#include "qsgdefaultrendercontext_p.h"
#include <qrgb.h>
#include <private/qdrawhelper_p.h>

QT_BEGIN_NAMESPACE

QSGRhiTextureGlyphCache::QSGRhiTextureGlyphCache(QSGDefaultRenderContext *rc,
                                                 QFontEngine::GlyphFormat format, const QTransform &matrix,
                                                 const QColor &color)
    : QImageTextureGlyphCache(format, matrix, color),
      m_rc(rc),
      m_rhi(rc->rhi())
{
    // Some OpenGL implementations, for instance macOS, have issues with
    // GL_ALPHA render targets. Similarly, BGRA may be problematic on GLES 2.0.
    // So stick with plain image uploads on GL.
    m_resizeWithTextureCopy = m_rhi->backend() != QRhi::OpenGLES2;
}

QSGRhiTextureGlyphCache::~QSGRhiTextureGlyphCache()
{
    // A plain delete should work, but just in case commitResourceUpdates was
    // not called and something is enqueued on the update batch for m_texture,
    // defer until the end of the frame.
    if (m_texture)
        m_texture->deleteLater();

    // should be empty, but just in case
    for (QRhiTexture *t : qAsConst(m_pendingDispose))
        t->deleteLater();
}

QRhiTexture *QSGRhiTextureGlyphCache::createEmptyTexture(QRhiTexture::Format format)
{
    QRhiTexture *t = m_rhi->newTexture(format, m_size, 1, QRhiTexture::UsedAsTransferSource);
    if (!t->create()) {
        qWarning("Failed to build new glyph cache texture of size %dx%d", m_size.width(), m_size.height());
        return nullptr;
    }

    QRhiResourceUpdateBatch *resourceUpdates = m_rc->glyphCacheResourceUpdates();

    // The new texture must be cleared to 0 always, this cannot be avoided
    // otherwise artifacts will occur around the glyphs.
    QByteArray data;
    if (format == QRhiTexture::RED_OR_ALPHA8)
        data.fill(0, m_size.width() * m_size.height());
    else
        data.fill(0, m_size.width() * m_size.height() * 4);
    QRhiTextureSubresourceUploadDescription subresDesc(data.constData(), data.size());
    subresDesc.setSourceSize(m_size);
    resourceUpdates->uploadTexture(t, QRhiTextureUploadEntry(0, 0, subresDesc));

    return t;
}

void QSGRhiTextureGlyphCache::createTextureData(int width, int height)
{
    width = qMax(128, width);
    height = qMax(32, height);

    if (!m_resizeWithTextureCopy)
        QImageTextureGlyphCache::createTextureData(width, height);

    m_size = QSize(width, height);
}

void QSGRhiTextureGlyphCache::resizeTextureData(int width, int height)
{
    width = qMax(128, width);
    height = qMax(32, height);

    if (m_size.width() >= width && m_size.height() >= height)
        return;

    m_size = QSize(width, height);

    if (m_texture) {
        QRhiTexture *t = createEmptyTexture(m_texture->format());
        if (!t)
            return;

        QRhiResourceUpdateBatch *resourceUpdates = m_rc->glyphCacheResourceUpdates();
        if (m_resizeWithTextureCopy) {
            resourceUpdates->copyTexture(t, m_texture);
        } else {
            QImageTextureGlyphCache::resizeTextureData(width, height);
            QImage img = image();
            prepareGlyphImage(&img);
            QRhiTextureSubresourceUploadDescription subresDesc(img);
            const QSize oldSize = m_texture->pixelSize();
            subresDesc.setSourceSize(QSize(qMin(oldSize.width(), width), qMin(oldSize.height(), height)));
            resourceUpdates->uploadTexture(t, QRhiTextureUploadEntry(0, 0, subresDesc));
        }

        m_pendingDispose.insert(m_texture);
        m_texture = t;
    }
}

void QSGRhiTextureGlyphCache::beginFillTexture()
{
    Q_ASSERT(m_uploads.isEmpty());
}

void QSGRhiTextureGlyphCache::prepareGlyphImage(QImage *img)
{
    const int maskWidth = img->width();
    const int maskHeight = img->height();
#if Q_BYTE_ORDER != Q_BIG_ENDIAN
    const bool supportsBgra = m_rhi->isTextureFormatSupported(QRhiTexture::BGRA8);
#endif
    m_bgra = false;

    if (img->format() == QImage::Format_Mono) {
        *img = img->convertToFormat(QImage::Format_Grayscale8);
    } else if (img->depth() == 32) {
        if (img->format() == QImage::Format_RGB32 || img->format() == QImage::Format_ARGB32_Premultiplied) {
            // We need to make the alpha component equal to the average of the RGB values.
            // This is needed when drawing sub-pixel antialiased text on translucent targets.
            for (int y = 0; y < maskHeight; ++y) {
                QRgb *src = (QRgb *) img->scanLine(y);
                for (int x = 0; x < maskWidth; ++x) {
                    int r = qRed(src[x]);
                    int g = qGreen(src[x]);
                    int b = qBlue(src[x]);
                    int avg;
                    if (img->format() == QImage::Format_RGB32)
                        avg = (r + g + b + 1) / 3; // "+1" for rounding.
                    else // Format_ARGB_Premultiplied
                        avg = qAlpha(src[x]);

                    src[x] = qRgba(r, g, b, avg);
#if Q_BYTE_ORDER != Q_BIG_ENDIAN
                    if (supportsBgra) {
                        m_bgra = true;
                    } else {
                        // swizzle the bits to accommodate for the RGBA upload.
                        src[x] = ARGB2RGBA(src[x]);
                        m_bgra = false;
                    }
#endif
                }
            }
        }
    }
}

void QSGRhiTextureGlyphCache::fillTexture(const Coord &c, glyph_t glyph, const QFixedPoint &subPixelPosition)
{
    QRhiTextureSubresourceUploadDescription subresDesc;
    QImage mask;

    if (!m_resizeWithTextureCopy) {
        QImageTextureGlyphCache::fillTexture(c, glyph, subPixelPosition);
        mask = image();
        subresDesc.setSourceTopLeft(QPoint(c.x, c.y));
        subresDesc.setSourceSize(QSize(c.w, c.h));
    } else {
        mask = textureMapForGlyph(glyph, subPixelPosition);
    }

    prepareGlyphImage(&mask);

    subresDesc.setImage(mask);
    subresDesc.setDestinationTopLeft(QPoint(c.x, c.y));
    m_uploads.append(QRhiTextureUploadEntry(0, 0, subresDesc));
}

void QSGRhiTextureGlyphCache::endFillTexture()
{
    if (m_uploads.isEmpty())
        return;

    if (!m_texture) {
        QRhiTexture::Format texFormat;
        if (m_format == QFontEngine::Format_A32 || m_format == QFontEngine::Format_ARGB)
            texFormat = m_bgra ? QRhiTexture::BGRA8 : QRhiTexture::RGBA8;
        else // should be R8, but there is the OpenGL ES 2.0 nonsense
            texFormat = QRhiTexture::RED_OR_ALPHA8;

        m_texture = createEmptyTexture(texFormat);
        if (!m_texture)
            return;
    }

    QRhiResourceUpdateBatch *resourceUpdates = m_rc->glyphCacheResourceUpdates();
    QRhiTextureUploadDescription desc;
    desc.setEntries(m_uploads.cbegin(), m_uploads.cend());
    resourceUpdates->uploadTexture(m_texture, desc);
    m_uploads.clear();
}

int QSGRhiTextureGlyphCache::glyphPadding() const
{
    if (m_format == QFontEngine::Format_Mono)
        return 8;
    else
        return 1;
}

int QSGRhiTextureGlyphCache::maxTextureWidth() const
{
    return m_rhi->resourceLimit(QRhi::TextureSizeMax);
}

int QSGRhiTextureGlyphCache::maxTextureHeight() const
{
    if (!m_resizeWithTextureCopy)
        return qMin(1024, m_rhi->resourceLimit(QRhi::TextureSizeMax));

    return m_rhi->resourceLimit(QRhi::TextureSizeMax);
}

void QSGRhiTextureGlyphCache::commitResourceUpdates(QRhiResourceUpdateBatch *mergeInto)
{
    if (QRhiResourceUpdateBatch *resourceUpdates = m_rc->maybeGlyphCacheResourceUpdates()) {
        mergeInto->merge(resourceUpdates);
        m_rc->releaseGlyphCacheResourceUpdates();
    }

    // now let's assume the resource updates will be committed in this frame
    for (QRhiTexture *t : qAsConst(m_pendingDispose))
        t->deleteLater(); // will be deleted after the frame is submitted -> safe

    m_pendingDispose.clear();
}

bool QSGRhiTextureGlyphCache::eightBitFormatIsAlphaSwizzled() const
{
    // return true when the shaders for 8-bit formats need .a instead of .r
    // when sampling the texture
    return !m_rhi->isFeatureSupported(QRhi::RedOrAlpha8IsRed);
}

QT_END_NAMESPACE

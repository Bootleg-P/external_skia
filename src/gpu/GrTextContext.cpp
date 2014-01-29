/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextContext.h"
#include "GrContext.h"

#include "SkAutoKern.h"
#include "SkGlyphCache.h"
#include "SkGr.h"

GrTextContext::GrTextContext(GrContext* context, const GrPaint& paint,
                             const SkPaint& skPaint, const SkDeviceProperties& properties) :
                            fContext(context), fPaint(paint), fSkPaint(skPaint),
                            fDeviceProperties(properties) {

    const GrClipData* clipData = context->getClip();

    SkRect devConservativeBound;
    clipData->fClipStack->getConservativeBounds(
                                     -clipData->fOrigin.fX,
                                     -clipData->fOrigin.fY,
                                     context->getRenderTarget()->width(),
                                     context->getRenderTarget()->height(),
                                     &devConservativeBound);

    devConservativeBound.roundOut(&fClipRect);

    fDrawTarget = context->getTextTarget();
}

//*** change to output positions?
void GrTextContext::MeasureText(SkGlyphCache* cache, SkDrawCacheProc glyphCacheProc,
                                const char text[], size_t byteLength, SkVector* stopVector) {
    SkFixed     x = 0, y = 0;
    const char* stop = text + byteLength;

    SkAutoKern  autokern;

    while (text < stop) {
        // don't need x, y here, since all subpixel variants will have the
        // same advance
        const SkGlyph& glyph = glyphCacheProc(cache, &text, 0, 0);

        x += autokern.adjust(glyph) + glyph.fAdvanceX;
        y += glyph.fAdvanceY;
    }
    stopVector->set(SkFixedToScalar(x), SkFixedToScalar(y));

    SkASSERT(text == stop);
}

static void GlyphCacheAuxProc(void* data) {
    GrFontScaler* scaler = (GrFontScaler*)data;
    SkSafeUnref(scaler);
}

GrFontScaler* GrTextContext::GetGrFontScaler(SkGlyphCache* cache) {
    void* auxData;
    GrFontScaler* scaler = NULL;

    if (cache->getAuxProcData(GlyphCacheAuxProc, &auxData)) {
        scaler = (GrFontScaler*)auxData;
    }
    if (NULL == scaler) {
        scaler = SkNEW_ARGS(SkGrFontScaler, (cache));
        cache->setAuxProc(GlyphCacheAuxProc, scaler);
    }

    return scaler;
}

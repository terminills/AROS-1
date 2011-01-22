#ifndef VG_MANGLE_H
#define VG_MANGLE_H
#define vegaGetError mvgGetError
#define vegaFlush mvgFlush
#define vegaFinish mvgFinish
#define vegaSetf mvgSetf
#define vegaSeti mvgSeti
#define vegaSetfv mvgSetfv
#define vegaSetiv mvgSetiv
#define vegaGetf mvgGetf
#define vegaGeti mvgGeti
#define vegaGetVectorSize mvgGetVectorSize
#define vegaGetfv mvgGetfv
#define vegaGetiv mvgGetiv
#define vegaSetParameterf mvgSetParameterf
#define vegaSetParameteri mvgSetParameteri
#define vegaSetParameterfv mvgSetParameterfv
#define vegaSetParameteriv mvgSetParameteriv
#define vegaGetParameterf mvgGetParameterf
#define vegaGetParameteri mvgGetParameteri
#define vegaGetParameterVectorSize mvgGetParameterVectorSize
#define vegaGetParameterfv mvgGetParameterfv
#define vegaGetParameteriv mvgGetParameteriv
#define vegaLoadIdentity mvgLoadIdentity
#define vegaLoadMatrix mvgLoadMatrix
#define vegaGetMatrix mvgGetMatrix
#define vegaMultMatrix mvgMultMatrix
#define vegaTranslate mvgTranslate
#define vegaScale mvgScale
#define vegaShear mvgShear
#define vegaRotate mvgRotate
#define vegaMask mvgMask
#define vegaRenderToMask mvgRenderToMask
#define vegaCreateMaskLayer mvgCreateMaskLayer
#define vegaDestroyMaskLayer mvgDestroyMaskLayer
#define vegaFillMaskLayer mvgFillMaskLayer
#define vegaCopyMask mvgCopyMask
#define vegaClear mvgClear
#define vegaCreatePath mvgCreatePath
#define vegaClearPath mvgClearPath
#define vegaDestroyPath mvgDestroyPath
#define vegaRemovePathCapabilities mvgRemovePathCapabilities
#define vegaGetPathCapabilities mvgGetPathCapabilities
#define vegaAppendPath mvgAppendPath
#define vegaAppendPathData mvgAppendPathData
#define vegaModifyPathCoords mvgModifyPathCoords
#define vegaTransformPath mvgTransformPath
#define vegaInterpolatePath mvgInterpolatePath
#define vegaPathLength mvgPathLength
#define vegaPointAlongPath mvgPointAlongPath
#define vegaPathBounds mvgPathBounds
#define vegaPathTransformedBounds mvgPathTransformedBounds
#define vegaDrawPath mvgDrawPath
#define vegaCreatePaint mvgCreatePaint
#define vegaDestroyPaint mvgDestroyPaint
#define vegaSetPaint mvgSetPaint
#define vegaGetPaint mvgGetPaint
#define vegaSetColor mvgSetColor
#define vegaGetColor mvgGetColor
#define vegaPaintPattern mvgPaintPattern
#define vegaCreateImage mvgCreateImage
#define vegaDestroyImage mvgDestroyImage
#define vegaClearImage mvgClearImage
#define vegaImageSubData mvgImageSubData
#define vegaGetImageSubData mvgGetImageSubData
#define vegaChildImage mvgChildImage
#define vegaGetParent mvgGetParent
#define vegaCopyImage mvgCopyImage
#define vegaDrawImage mvgDrawImage
#define vegaSetPixels mvgSetPixels
#define vegaWritePixels mvgWritePixels
#define vegaGetPixels mvgGetPixels
#define vegaReadPixels mvgReadPixels
#define vegaCopyPixels mvgCopyPixels
#define vegaCreateFont mvgCreateFont
#define vegaDestroyFont mvgDestroyFont
#define vegaSetGlyphToPath mvgSetGlyphToPath
#define vegaSetGlyphToImage mvgSetGlyphToImage
#define vegaClearGlyph mvgClearGlyph
#define vegaDrawGlyph mvgDrawGlyph
#define vegaDrawGlyphs mvgDrawGlyphs
#define vegaColorMatrix mvgColorMatrix
#define vegaConvolve mvgConvolve
#define vegaSeparableConvolve mvgSeparableConvolve
#define vegaGaussianBlur mvgGaussianBlur
#define vegaLookup mvgLookup
#define vegaLookupSingle mvgLookupSingle
#define vegaHardwareQuery mvgHardwareQuery
#define vegaGetString mvgGetString
#define vguLine mvguLine
#define vguPolygon mvguPolygon
#define vguRect mvguRect
#define vguRoundRect mvguRoundRect
#define vguEllipse mvguEllipse
#define vguArc mvguArc
#define vguComputeWarpQuadToSquare mvguComputeWarpQuadToSquare
#define vguComputeWarpSquareToQuad mvguComputeWarpSquareToQuad
#endif

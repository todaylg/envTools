/* -*-c++-*- */

#include <vector>
#include "Light"
#include "Math"
#include "SummedAreaTableRegion"

uchar debugColorR;
uchar debugColorG;
uchar debugColorB;
uchar debugColorA;

/**
 * Draw a light source from each region on its centroid
 */
void drawDebug(uchar* d, const int ci, const uint nc) {
    if (ci < 0) return;

    d[ci + 0] = debugColorR;
    d[ci + 1] = debugColorG;
    d[ci + 2] = debugColorB;
    if (nc > 3) d[ci + 3] = debugColorA;
}

/**
 * Draw a region
 */
void drawRegions(uchar* rgba, int width, int height,
                 const SatRegionVector& regions, uint nc) {
    uint pixelPos = 0;
    for (int i = 0; i < height; ++i) {
        for (int p = 0; p < width; ++p) {
            for (SatRegionVector::const_iterator region = regions.begin();
                 region != regions.end(); ++region) {
                if (((p >= region->_x) && (p <= region->_x + region->_w) &&
                     (i == region->_y)) ||
                    ((i <= region->_y + region->_h) && (i >= region->_y) &&
                     (p == region->_x))) {
                    // draw once is sufficient
                    drawDebug(rgba, pixelPos, nc);
                    break;
                }
            }

            pixelPos += nc;
        }
    }
}

/**
 * Draw a cross at position l into image rgba
 */
void drawLight(uchar* rgba, int width, int height, const Light& l,
               const uint nc) {
    int ci;

    const uint xCenter = static_cast<uint>(l._centroidPosition[0] * width);
    const uint yCenter = static_cast<uint>(l._centroidPosition[1] * height);

    const uint m = width * height * nc;

    // draw light center cross
    uint pixelPos;
    uint pixelCount;

    const uint xLeft = l._x * width;
    const uint widthPixel = l._w * width;
    const uint xRight = xLeft + widthPixel;

    const uint yBottom = l._y * height;
    const uint heightPixel = l._h * height;
    const uint yTop = yBottom + heightPixel;

    // draw light area
    // width lines
    pixelCount = widthPixel;
    // up
    pixelPos = (yBottom * width + xLeft) * nc;
    for (int p = 0; p < pixelCount && pixelPos < m; ++p) {
        drawDebug(rgba, pixelPos, nc);
        pixelPos += nc;
    }

    // bottom
    pixelPos = (yTop * width + xLeft) * nc;
    for (int p = 0; p < pixelCount && pixelPos < m; ++p) {
        drawDebug(rgba, pixelPos, nc);
        pixelPos += nc;
    }
    // center
    pixelPos = (yCenter * width + xCenter - widthPixel * 0.5) * nc;
    for (int p = 0; p < pixelCount && pixelPos < m; ++p) {
        drawDebug(rgba, pixelPos, nc);
        pixelPos += nc;
    }

    // Height lines
    pixelCount = heightPixel;
    // left
    pixelPos = (yBottom * width + xLeft) * nc;
    for (int p = 0; p < pixelCount && pixelPos < m; ++p) {
        drawDebug(rgba, pixelPos, nc);
        pixelPos += nc * width;
    }
    // right
    pixelPos = (yBottom * width + xRight) * nc;
    for (int p = 0; p < pixelCount && pixelPos < m; ++p) {
        drawDebug(rgba, pixelPos, nc);
        pixelPos += nc * width;
    }

    // center

    pixelCount = heightPixel * 2.0;
    pixelPos = ((yCenter)*width + xCenter - (heightPixel * width)) * nc;
    for (int p = 0; p < pixelCount && pixelPos < m; ++p) {
        drawDebug(rgba, pixelPos, nc);
        pixelPos += nc * width;
    }
}

void debugDrawLight(const SatRegionVector& regions, const LightVector& lights,
                    const LightVector& mainLights, float* rgba,
                    const uint width, const uint height, const uint nc,
                    const double maxLum, const double minLum,
                    const int numLights) {
    assert(nc >= 3);

    size_t lightNum = numLights > 0 ? numLights : lights.size();
    uint i = 0;

    // save image with marked samples
    std::vector<uchar> conv;
    conv.resize(width * height * nc);

    // convert to luminance only
    for (i = 0; i < width * height * nc;) {
        const float r = rgba[i + 0];
        const float g = rgba[i + 1];
        const float b = rgba[i + 2];

        double ixy = (luminance(r, g, b));
        const uchar luminanceByte = static_cast<uchar>(ixy * 255);

        if (nc > 0) conv[i++] = luminanceByte;
        if (nc > 1) conv[i++] = luminanceByte;
        if (nc > 2) conv[i++] = luminanceByte;
        if (nc > 3) conv[i++] = 255;
    }

    // drawRegions first

    debugColorR = 0;
    debugColorG = 255;
    debugColorB = 0;
    debugColorA = 255;

#ifdef _DRAW_REGIONS
    drawRegions(&conv[0], width, height, regions, nc);
#endif

#ifdef DRAW_LIGHT_ABOVE_REGIONS
    // draw Light above regions
    i = 0;

    debugColorR = 0;
    debugColorG = 255;
    debugColorB = 0;
    debugColorA = 255;

    for (LightVector::const_iterator l = lights.begin();
         l != lights.end() && i < lightNum; ++l) {
        drawLight(&conv[0], width, height, *l, nc);
        i++;
    }
#endif

    const size_t mainLightNum = std::min(3, (int)lightNum);

    debugColorR = 255;
    debugColorG = 0;
    debugColorB = 0;
    debugColorA = 255;

    i = 0;
    for (LightVector::const_iterator l = mainLights.begin();
         l != mainLights.end() && i < mainLightNum; ++l) {
        if (l->_y >= 0.5) continue;

        drawLight(&conv[0], width, height, *l, nc);
        i++;
    }

    debugColorR = 0;
    debugColorG = 0;
    debugColorB = 255;
    debugColorA = 255;
    lightNum = mainLights.size();

    for (LightVector::const_iterator l = mainLights.begin();
         l != mainLights.end() && i < lightNum; ++l) {
        if (l->_y >= 0.5) continue;

        drawLight(&conv[0], width, height, *l, nc);
        i++;
    }

    ImageOutput* out = ImageOutput::create("out/test_variance.png");
    ImageSpec specOut(width, height, nc, TypeDesc::UINT8);
    ImageOutput::OpenMode appendmode = ImageOutput::Create;
    out->open("out/debug_variance.png", specOut, appendmode);
    out->write_image(TypeDesc::UINT8, &conv[0]);
}

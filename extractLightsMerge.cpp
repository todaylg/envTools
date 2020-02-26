/* -*-c++-*- */

/**
 * convert Env map Regions to Lights
 */
void createLightsFromRegions(const SatRegionVector& regions,
                             LightVector& lights, const float* rgba,
                             const double maxLum, const int width,
                             const int height, const int nc,
                             const SummedAreaTable& lumSat) {
    const double maxR = lumSat.getMaxR();
    const double maxG = lumSat.getMaxG();
    const double maxB = lumSat.getMaxB();
    const double weigth = lumSat.getWeightAccumulation();

    const uint imgSize = width * height;
    double weight = (4.0 * PI) / ((double)(imgSize));

    // convert region into lights
    for (SatRegionVector::const_iterator region = regions.begin();
         region != regions.end(); ++region) {
        Light l;

        // init values
        l._merged = false;
        l._mergedNum = 0;

        l._x = region->_x;
        l._y = region->_y;
        l._w = region->_w;
        l._h = region->_h;

        // set light at centroid
        l._centroidPosition = region->centroid();

        // light area Size
        l._areaSize = region->areaSize();

        const uint i = static_cast<uint>(l._centroidPosition[1] * width +
                                         l._centroidPosition[0]);

        // compute area values, as SAT introduce precision errors
        // due to high sum values against small data values
        // we use here less error inducing computations
        double r = rgba[i * nc + 0];
        double g = rgba[i * nc + 1];
        double b = rgba[i * nc + 2];
        {
            double y =
                ((double)l._centroidPosition[1] + 1.0) / (double)(height + 1);
            double solidAngle = cos(PI * (y - 0.5)) * weight;
            l._luminancePixel = luminance(r, g, b) * solidAngle;
        }

        double rSum = 0.0;
        double gSum = 0.0;
        double bSum = 0.0;
        double lumSum = 0.0;

        for (int y1 = l._y; y1 < l._y + l._h; ++y1) {
            const double posY = ((double)y1 + 1.0) / (double)(height + 1.0);
            const double solidAngle = cos(PI * (posY - 0.5)) * weight;

            for (int x1 = l._x; x1 < l._x + l._w; ++x1) {
                const uint i = (x1 + (y1 * width)) * nc;

                r = rgba[i];
                g = rgba[i + 1];
                b = rgba[i + 2];

                lumSum += luminance(r, g, b) * solidAngle;

                rSum += r;
                gSum += g;
                bSum += b;
            }
        }

        // normalize
        lumSum *= (4.0 * PI) / weigth;
        l._sum = lumSum;

        l._variance =
            ((l._sum * l._sum) / l._areaSize) - (l._lumAverage * l._lumAverage);

        // Colors
        l._rAverage = rSum / l._areaSize;
        l._gAverage = gSum / l._areaSize;
        l._bAverage = bSum / l._areaSize;
        l._lumAverage = lumSum / l._areaSize;

        // make all value 0..1 now
        l._x = static_cast<double>(l._x) / (double)width;
        l._y = static_cast<double>(l._y) / (double)height;
        l._w = static_cast<double>(l._w) / (double)width;
        l._h = static_cast<double>(l._h) / (double)height;
        l._areaSize = l._w * l._h;

        l._centroidPosition[0] = l._centroidPosition[0] / (double)width;
        l._centroidPosition[1] = l._centroidPosition[1] / (double)height;

        // if value out of bounds
        l._error = l._sum > maxLum;
        l._sortCriteria = l._areaSize;

        lights.push_back(l);
    }
}

/*
 *  Merge a light into another and store a copy inside the parent
 */
void mergeLight(Light& lightParent, Light& lightChild) {
    // exclude from next merges
    lightChild._merged = true;

    const double x = lightParent._x;
    const double y = lightParent._y;
    const double w = lightParent._w;
    const double h = lightParent._h;

    lightParent._x = std::min(x, lightChild._x);
    lightParent._y = std::min(y, lightChild._y);

    lightParent._w =
        std::max(x + w, (lightChild._x + lightChild._w)) - lightParent._x;
    lightParent._h =
        std::max(y + h, (lightChild._y + lightChild._h)) - lightParent._y;

    lightParent.childrenLights.push_back(lightChild);

    // light is bigger, better candidate to main light
    lightParent._mergedNum++;

    lightParent._sum += lightChild._sum;

    double newAreaSize = lightParent._areaSize + lightChild._areaSize;
    double ratioParent = lightParent._areaSize / newAreaSize;
    double ratioChild = lightChild._areaSize / newAreaSize;

    lightParent._rAverage =
        lightParent._rAverage * ratioParent + lightChild._rAverage * ratioChild;
    lightParent._gAverage =
        lightParent._gAverage * ratioParent + lightChild._gAverage * ratioChild;
    lightParent._bAverage =
        lightParent._bAverage * ratioParent + lightChild._bAverage * ratioChild;

    // newAreaSize = lightParent._w * lightParent._h;
    lightParent._areaSize = newAreaSize;
    lightParent._lumAverage = lightParent._sum / newAreaSize;

    // lightChild._sortCriteria = lightChild._lumAverage;
    lightChild._sortCriteria = lightChild._sum;
    // lightChild._sortCriteria = lightChild._areaSize;
    // lightParent._sortCriteria = lightParent._sum;
}

/*
 * not a constructor, not struct member as it's specific for merge
 *  we copy only parts
 */
void lightCopy(Light& lDest, const Light& lSrc) {
    lDest._error = lSrc._error;
    lDest._merged = lSrc._merged;
    lDest._mergedNum = lSrc._mergedNum;
    lDest._x = lSrc._x;
    lDest._y = lSrc._y;
    lDest._w = lSrc._w;
    lDest._h = lSrc._h;
    lDest._centroidPosition[0] = lSrc._centroidPosition[0];
    lDest._centroidPosition[1] = lSrc._centroidPosition[1];
    lDest._areaSize = lSrc._areaSize;
    lDest._sum = lSrc._sum;
    lDest._variance = lSrc._variance;
    lDest._lumAverage = lSrc._lumAverage;
    lDest._rAverage = lSrc._rAverage;
    lDest._gAverage = lSrc._gAverage;
    lDest._bAverage = lSrc._bAverage;
    lDest._luminancePixel = lSrc._luminancePixel;
}
/*
 * intersect lights with another light
 */
bool intersectLightAgainstLights2D(const LightVector& lights,
                                   const Light& lightCandidate,
                                   const double border) {
    if (lights.empty()) return true;

    double x1 = lightCandidate._x - border;
    double y1 = lightCandidate._y - border;
    double x2 = x1 + lightCandidate._w + border;
    ;
    double y2 = y1 + lightCandidate._h + border;
    ;

    for (LightVector::const_iterator l = lights.begin(); l != lights.end();
         ++l) {
        if (!(l->_y > y2 || l->_y + l->_h < y1 || l->_x > x2 ||
              l->_x + l->_w < x1)) {
            return true;
        }
    }

    return false;
}

/**
 * Merge small area light neighbour with small area light neighbours
 */
uint mergeLights(LightVector& lights, LightVector& newLights, const uint width,
                 const uint height, const double areaSizeMax,
                 const double lengthSizeMax, const double luminanceMaxLight,
                 const double degreeMerge) {
    // discard or keep Light too near an current light
    const double border = degreeMerge * PI / 360.0;

    uint numMergedLightTotal;

    numMergedLightTotal = 0;

    // for each light we try to merge with all other intersecting lights
    // that are in the same neighborhood of the sorted list of lights
    // where neighbors are of near same values
    for (LightVector::iterator lightIt = lights.begin();
         lightIt != lights.end(); ++lightIt) {
        // already merged in a previous light
        // we do nothing
        if (lightIt->_merged) continue;

        Light lCurrent;
        lightCopy(lCurrent, *lightIt);
        double x1 = lCurrent._x - border;
        double y1 = lCurrent._y - border;
        double x2 = x1 + lCurrent._w + border;
        double y2 = y1 + lCurrent._h + border;

        uint numMergedLight;

        // current area Size will change when getting merges
        // we store initial values to prevent merging
        // with light too low
        const double areaSizeCurrent = lCurrent._areaSize;

        do {
            numMergedLight = 0;

            // could start at current light
            // lights is sorted by areasize from small to big
            for (LightVector::iterator l = lights.begin(); l != lights.end();
                 ++l) {
                // ignore already merged into another
                if (l->_merged) continue;

                // ignore itself
                if (l == lightIt) continue;

                // if merged do new size will be problematic
                const double newX = std::min(lCurrent._x, l->_x);
                const double newY = std::min(lCurrent._y, l->_y);

                const double newParentSizeW =
                    std::max(lCurrent._x + lCurrent._w, (l->_x + l->_w)) - newX;
                if (lengthSizeMax < newParentSizeW) continue;

                const double newParentSizeH =
                    std::max(lCurrent._y + lCurrent._h, (l->_y + l->_h)) - newY;

                if (lengthSizeMax < newParentSizeH) continue;

                bool intersect2D = !(l->_y > y2 || l->_y + l->_h < y1 ||
                                     l->_x > x2 || l->_x + l->_w < x1);
                // try left/right border as it's a env wrap
                // complexity arise, how to merge...and then retest after
                /*
                  if (!intersect2D ){
                  if( x == 0 ){
                  //check left borders
                  intersect2D = !(l->_y-border > y+h || l->_y+l->_h+border < y
                  || l->_x-border > width + w || l->_x+l->_w+border < width);
                  }else if( x+w == width ){
                  //check right borders
                  intersect2D = !(l->_y-border > y+h || l->_y+l->_h+border < y
                  || l->_x-border > w + (width - x) || l->_x+l->_w+border <
                  (width - x));
                  }
                  }
                */

                //  share borders
                if (intersect2D) {
                    // remove from previous merge if any
                    for (LightVector::iterator lBigIt = newLights.begin();
                         lBigIt != newLights.end(); ++lBigIt) {
                        if (l == lBigIt) {
                            newLights.erase(lBigIt);
                            break;
                        }
                    }

                    mergeLight(lCurrent, *l);

                    x1 = lCurrent._x - border;
                    y1 = lCurrent._y - border;
                    x2 = x1 + lCurrent._w + border;
                    y2 = y1 + lCurrent._h + border;

                    numMergedLight++;
                    numMergedLightTotal++;
                }
            }

            // if we're merging we're changing borders
            // means we have new neighbours
            // or light now included inside our area
        } while (numMergedLight > 0);

        if (lCurrent._mergedNum > 0) {
            lCurrent._sortCriteria = lCurrent._areaSize;
            lCurrent._sortCriteria = lCurrent._sum;
            newLights.push_back(lCurrent);
        }
    }

    // count merged light
    numMergedLightTotal = 0;
    for (LightVector::iterator lCurrent = newLights.begin();
         lCurrent != newLights.end(); ++lCurrent) {
        if (!lCurrent->_merged) numMergedLightTotal += lCurrent->_mergedNum;
    }

    if (1) {
        // fill new array with light that wasn't merged at all
        for (LightVector::iterator lCurrent = lights.begin();
             lCurrent != lights.end(); ++lCurrent) {
            // add remaining non merged lights
            if (!lCurrent->_merged && lCurrent->_mergedNum == 0) {
                lCurrent->_lumAverage = lCurrent->_sum / lCurrent->_areaSize;

                // lCurrent->_sortCriteria = lCurrent->_lumAverage;
                lCurrent->_sortCriteria = lCurrent->_sum;

                newLights.push_back(*lCurrent);
            }
        }
    }

    return numMergedLightTotal;
}

// we now have merged big area light, which are the zone with the most light
// possible now we mush restrict those to smallest possible significant light
// possible reducing it to a near directional light as much as possible
uint selectLights(LightVector& mergedLights, LightVector& newLights,
                  const uint width, const uint height, const double areaSizeMax,
                  const double luminanceMaxLight, const double envLuminanceSum,
                  const double degreeMerge) {
    // discard or keep light too near an current light
    const double border = degreeMerge * PI / 360.0;
    ;  // static_cast <uint> (sqrt (areaSizeMax) / 2.0);

    uint numMergedLightTotal = 0;

    for (LightVector::iterator lightIt = mergedLights.begin();
         lightIt != mergedLights.end(); ++lightIt) {
        // already merged, we find in lights the light intersecting
        uint numMergedLight = 0;

        // if light "splittable"
        // and light already over ratio, need to cut it
        if (lightIt->_mergedNum > 0) {
            LightVector& lights = lightIt->childrenLights;

            // sort to get most powerful light first
            std::sort(lights.begin(), lights.end());

            // take biggest and merge a bit
            Light lCurrent;
            lightCopy(lCurrent, lights[0]);
            double x1 = lCurrent._x - border;
            double y1 = lCurrent._y - border;
            double x2 = x1 + lCurrent._w + border;
            double y2 = y1 + lCurrent._h + border;

            // reset children lights to start over merge process
            for (LightVector::iterator l = lights.begin() + 1;
                 l != lights.end(); ++l) {
                l->_merged = false;
            }

            do {
                numMergedLight = 0;

                for (LightVector::iterator l = lights.begin() + 1;
                     l != lights.end(); ++l) {
                    // ignore already merged or itself
                    if (l->_merged) continue;

                    bool intersect2D = !(l->_y > y2 || l->_y + l->_h < y1 ||
                                         l->_x > x2 || l->_x + l->_w < x1);

                    if (intersect2D &&
                        intersectLightAgainstLights2D(lCurrent.childrenLights,
                                                      *l, border)) {
                        mergeLight(lCurrent, *l);

                        x1 = lCurrent._x - border;
                        y1 = lCurrent._y - border;
                        x2 = x1 + lCurrent._w + border;
                        y2 = y1 + lCurrent._h + border;

                        numMergedLight++;
                    }
                }
            } while (numMergedLight > 0);

            lCurrent._sortCriteria = lCurrent._sum;
            newLights.push_back(lCurrent);

            numMergedLightTotal += lCurrent._mergedNum;

        } else {
            lightIt->_sortCriteria = lightIt->_lumAverage;
            lightIt->_sortCriteria = lightIt->_sum;
            newLights.push_back(*lightIt);
            numMergedLightTotal += lightIt->_mergedNum;
        }
    }

    return numMergedLightTotal;
}

uint mergeNearLights(LightVector& mergedLights, const double areaSizeMax,
                     const double lengthSizeMax, const double degreeMerge) {
    // discard or keep light too near an current light
    const double border = degreeMerge * PI / 180.0;
    ;  // static_cast <uint> (sqrt (areaSizeMax) / 2.0);
    uint numMergedLightTotal;

    numMergedLightTotal = 0;

    for (int i = 0; i < mergedLights.size(); ++i) {
        Light* lightCurrent = &(mergedLights[i]);  //.at(i);

        // already merged light into another
        if (lightCurrent->_merged) continue;

        double x1 = lightCurrent->_x - border;
        double y1 = lightCurrent->_y - border;
        double x2 = x1 + lightCurrent->_w + border;
        double y2 = y1 + lightCurrent->_h + border;

        // reset children lights to start over merge process
        int j = 0;
        for (LightVector::iterator l = mergedLights.begin();
             l != mergedLights.end(); ++j) {
            if (i == j) {
                ++l;
                continue;
            }

            // pick only smallest & near lights,
            // grow until not bigger than areaSize
            // if merged, what would be new Size
            // only test merged size (allow for intersections of merged)
            const double newX = std::min(lightCurrent->_x, l->_x);
            const double newY = std::min(lightCurrent->_y, l->_y);

            const double newParentSizeW =
                std::max(lightCurrent->_x + lightCurrent->_w, (l->_x + l->_w)) -
                newX;
            if (lengthSizeMax < newParentSizeW) continue;
            const double newParentSizeH =
                std::max(lightCurrent->_y + lightCurrent->_h, (l->_y + l->_h)) -
                newY;
            if (lengthSizeMax < newParentSizeH) continue;

            bool intersect2D = !(l->_y > y2 || l->_y + l->_h < y1 ||
                                 l->_x > x2 || l->_x + l->_w < x1);

            if (intersect2D) {
                mergeLight(*lightCurrent, *l);

                x1 = lightCurrent->_x - border;
                y1 = lightCurrent->_y - border;
                x2 = x1 + lightCurrent->_w + border;
                y2 = y1 + lightCurrent->_h + border;

                l = mergedLights.erase(l);

                numMergedLightTotal++;
            } else {
                ++l;
            }
        }
    }
}

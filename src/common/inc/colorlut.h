//
// begin license header
//
// This file is part of Pixy CMUcam5 or "Pixy" for short
//
// All Pixy source code is provided under the terms of the
// GNU General Public License v2 (http://www.gnu.org/licenses/gpl-2.0.html).
// Those wishing to use Pixy source code, software and/or
// technologies under different licensing terms should contact us at
// cmucam@cs.cmu.edu. Such licensing terms are available for
// all portions of the Pixy codebase presented here.
//
// end license header
//
#ifndef COLORLUT_H
#define COLORLUT_H	  

#include <inttypes.h>
#include "simplevector.h"
#include "pixytypes.h"
#include "experimentalsignature.h"
#ifndef PIXY
#include "debug.h"
#endif

#define CL_NUM_SIGNATURES               7
#define CL_LUT_COMPONENT_SCALE          6
#define CL_LUT_SIZE                     (1<<(CL_LUT_COMPONENT_SCALE*2))
#define CL_LUT_ENTRY_SCALE              15
#define CL_GROW_INC                     4
#define CL_MIN_Y_F                      0.05 // for when generating signatures, etc
#define CL_MIN_Y                        (int32_t)(3*((1<<8)-1)*CL_MIN_Y_F)
#define CL_MIN_RATIO                    0.25f
#define CL_DEFAULT_MINY                 0.1f
#define CL_DEFAULT_SIG_RANGE			2.5f
#define CL_MAX_DIST                     2000
#define CL_DEFAULT_TOL                  0.80f
#define CL_DEFAULT_CCGAIN               1.5f
#define CL_MODEL_TYPE_COLORCODE         1

#ifndef PIXY
extern bool g_logExp;
//#define EXPLOG printf
#ifndef PIXY
#define EXPLOG(...)  if(g_logExp)qDebug(__VA_ARGS__)
#else
#define EXPLOG(...)
#endif
#endif

struct ColorSignature
{
	ColorSignature()
	{
		m_uMin = m_uMax = m_uMean = m_vMin = m_vMax = m_vMean = m_type = 0;
	}	

    int32_t m_uMin;
    int32_t m_uMax;
    int32_t m_uMean;
    int32_t m_vMin;
    int32_t m_vMax;
    int32_t m_vMean;
	uint32_t m_rgb;
    uint32_t m_type;
};


struct RuntimeSignature
{
    int32_t m_uMin;
    int32_t m_uMax;
    int32_t m_vMin;
    int32_t m_vMax;
    uint32_t m_rgbSat;
};

typedef SimpleVector<Point16> Points;

class IterPixel
{
public:
    IterPixel(const Frame8 &frame, const RectA &region);
    IterPixel(const Frame8 &frame, const Points *points);
    bool next(UVPixel *uv, RGBPixel *rgb=NULL, bool omitCutOnY=false);
    bool reset(bool cleari=true);
	uint32_t averageRgb(uint32_t *pixels=NULL);

private:
    bool nextHelper(UVPixel *uv, RGBPixel *rgb, bool omitCutOnY);

    Frame8 m_frame;
    RectA m_region;
    uint32_t m_x, m_y;
    uint8_t *m_pixels;
    const Points *m_points;
    int m_i;
};

class ColorLUT
{
public:
    ColorLUT(uint8_t *lut);
    ~ColorLUT();

    int generateSignature(const Frame8 &frame, const RectA &region, uint8_t signum);
    int generateSignature(const Frame8 &frame, const Point16 &point, Points *points, uint8_t signum);
	ColorSignature *getSignature(uint8_t signum);
	int setSignature(uint8_t signum, const ColorSignature &sig);

    int generateLUT();

    void clearLUT(uint8_t signum=0);
	void updateSignature(uint8_t signum);
    void growRegion(const Frame8 &frame, const Point16 &seed, Points *points);

	void setSigRange(uint8_t signum, float range);
	void setMinBrightness(float miny);
	void setGrowDist(uint32_t dist);
    void setCCGain(float gain);
    uint32_t getType(uint8_t signum);

    // these should be in little access methods, but they're here to speed things up a tad
    ColorSignature m_signatures[CL_NUM_SIGNATURES];
    RuntimeSignature m_runtimeSigs[CL_NUM_SIGNATURES];

    uint32_t m_miny;
    uint16_t m_expYMin; // lowest accepted y=r+g+b in LUT generation, used in Blobs::runlengthAnalysis to speed things up

    /// non const access to experimental signature of given ID [1-7]
    inline ExperimentalSignature& accExpSig(uint8_t idx) {return m_expSigs[(idx-1)&0x7];}
    /// returns experimental signature of given ID [1-7]
    inline const ExperimentalSignature& expSig(uint8_t idx) const {return m_expSigs[(idx-1)&0x7];}

private:
    ExperimentalSignature m_expSigs[CL_NUM_SIGNATURES];

    bool growRegion(RectA *region, const Frame8 &frame, uint8_t dir);
    float testRegion(const RectA &region, const Frame8 &frame, UVPixel *mean, Points *points);

    void calcRatios(IterPixel *ip, ColorSignature *sig, float ratios[]);
    void iterate(IterPixel *ip, ColorSignature *sig);
    void getMean(const RectA &region ,const Frame8 &frame, UVPixel *mean);

    uint8_t *m_lut;
    uint32_t m_maxDist;
    float m_ratio;
    float m_minRatio;
    float m_ccGain;
    float m_sigRanges[CL_NUM_SIGNATURES];

    // experimental signature stuff
    public: //evillive
    bool m_useExpSigs;
#ifndef PIXY
    bool m_useExpLut;
#endif
};

/// just a static wrapper to group the experimental color lookup table related stuff
struct ColorLutCalculatorExp{

    /// calculate u and v [0,63] from r,g and b [0..255] as needed for the signature lookup
    static void calcUV(int16_t r, int16_t g, int16_t b, int16_t& u, int16_t& v);

    /// setup the division lookup table
    static void setupDivLut();

    /// A lookup table for an add and shift approximation of a division
    /// of a signed int16 (not too big) by an unsigned uint8
    /// Usage Example a/b:
    /// unsigned char lut=divLUT[b];
    /// unsigned char s1=lut&0xf;
    /// if(s1) a+=a>>s1;
    /// unsigned char s2=lut>>4;
    /// a>>=s2
    /// Has been tweaked! Now it divides by b<<2
    /// to get directly within the range of the color LUT
    static uint8_t s_divLut[256];
};


#endif // COLORLUT_H

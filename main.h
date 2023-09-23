#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED
 
// CIEXYZTRIPLE stuff
typedef int FXPT2DOT30;
 
typedef struct {
    FXPT2DOT30 ciexyzX;
    FXPT2DOT30 ciexyzY;
    FXPT2DOT30 ciexyzZ;
} CIEXYZ;
 
typedef struct {
    CIEXYZ  ciexyzRed; 
    CIEXYZ  ciexyzGreen; 
    CIEXYZ  ciexyzBlue; 
} CIEXYZTRIPLE;
 
// bitmap file header
typedef struct {
    unsigned short bfType;
    unsigned int   bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int   bfOffBits;
} BITMAPFILEHEADER;
 
// bitmap info header
typedef struct {
    unsigned int   biSize;
    unsigned int   biWidth;
    unsigned int   biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int   biCompression;
    unsigned int   biSizeImage;
    unsigned int   biXPelsPerMeter;
    unsigned int   biYPelsPerMeter;
    unsigned int   biClrUsed;
    unsigned int   biClrImportant;
    unsigned int   biRedMask;
    unsigned int   biGreenMask;
    unsigned int   biBlueMask;
    unsigned int   biAlphaMask;
    unsigned int   biCSType;
    CIEXYZTRIPLE   biEndpoints;
    unsigned int   biGammaRed;
    unsigned int   biGammaGreen;
    unsigned int   biGammaBlue;
    unsigned int   biIntent;
    unsigned int   biProfileData;
    unsigned int   biProfileSize;
    unsigned int   biReserved;
} BITMAPINFOHEADER;
 
// rgb quad
typedef struct {
    unsigned char  rgbBlue;
    unsigned char  rgbGreen;
    unsigned char  rgbRed;
    unsigned char  rgbReserved;
} RGBQUAD;
 
// read bytes
template <typename Type>
void read(std::ifstream &fp, Type &result, std::size_t size) {
    fp.read(reinterpret_cast<char*>(&result), size);
}
 
// bit extract
unsigned char bitextract(const unsigned int byte, const unsigned int mask);
 
#endif // MAIN_H_INCLUDEDs
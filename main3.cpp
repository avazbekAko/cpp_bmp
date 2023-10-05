#include <iostream>
#include <stdio.h>
#include <windows.h>
#define WIN32_MEAN_AND_LEAN
using namespace std;
 
int main()
{
    setlocale(0, "rus");
    //заголовки необходимые для работы с bmp
    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;
    RGBTRIPLE pixel;
    FILE *input, *output;
 
    //открываем файл для чтения
    input = fopen("bmp24_1.bmp", "rb");
    output = fopen("bmp24_out.bmp", "wb");
    if (!input)
    {
        cerr << "Файл bmp не найден\n";
        system("pause");
        return 1;
    }
    /*чтение BMPFILEHEADER и  BMPINFOHEADER*/
    fread(&file_header, sizeof(BITMAPFILEHEADER), 1, input);
    fread(&info_header, sizeof(BITMAPINFOHEADER), 1, input);
 
    if (info_header.biCompression != 0 || info_header.biBitCount != 24 || info_header.biPlanes != 1)
    {
        cerr << "Файл не BMP24\n";
        system("pause");
        return 1;
    }
    
    int Width = info_header.biWidth;
    int Height = info_header.biHeight;
 
    /*длина строки должны быть кратна 4, иначе файл будет поврежден.
    Будем проверять это условие.
    padding - количество байт, недостающих до того чтобы кол-во байты было кратно 4*/
    unsigned char padding = 0;
    if ((Width * 3) % 4)
        padding = 4 - (Width * 3) % 4;
 
    /*выделяем память для массива данных о пикселях*/
    RGBTRIPLE** img = new RGBTRIPLE*[Height];
    for (size_t i = 0; i < Height; i++)
        img[i] = new RGBTRIPLE[Width];
    
 
    /*чтение пикселей*/
    for (int i = 0; i < Height; i++)
    {
        for (int j = 0; j < Width; j++)
        {
            fread(&pixel, sizeof(RGBTRIPLE), 1, input);
            img[i][j] = pixel;
        }
        fseek(input, padding, SEEK_CUR); // пропуск "выравнивающих" байтов
    }
    fclose(input);
    
 
    /*запись нового файла*/
    info_header.biHeight = Width;
    info_header.biWidth = Height;
    fwrite(&file_header, sizeof(BITMAPFILEHEADER), 1, output);
    fwrite(&info_header, sizeof(BITMAPINFOHEADER), 1, output);
 
    //расчет нового значения padding 
    padding = 0;
    if ((Height* 3) % 4)
        padding = 4 - (Height * 3) % 4;
 
    /*---------транспонирование картинки--------------*/
    for(int j = Width - 1; j >= 0; j--)
    {
        for (int i = 0; i < Height; i++)
            fwrite(&img[i][j], sizeof(RGBTRIPLE), 1, output);
        
        if (padding != 0)
        {
            BYTE filler = 0;
            fwrite(&filler, sizeof(BYTE), padding, output);
        }
    }
    
    //system("pause");
    return 0;
}
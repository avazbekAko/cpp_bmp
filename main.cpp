#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>

using namespace std;
#pragma pack(push, 1)

struct BMPFileHeader {
	uint16_t file_type{ 0x4D42 };
	uint32_t file_size{ 0 };
	uint16_t reserved1{ 0 };
	uint16_t reserved2{ 0 };
	uint32_t offset_data{ 0 };

	uint32_t size{ 0 };
	int32_t width{ 0 };
	int32_t height{ 0 };
	uint16_t planes{ 1 };
	uint16_t bit_count{ 0 };
	uint32_t compression{ 0 };
	uint32_t size_image{ 0 };
	int32_t x_pixels_per_meter{ 0 };
	int32_t y_pixels_per_meter{ 0 };
	uint32_t colors_used{ 0 };
	uint32_t colors_important{ 0 };

	uint32_t red_mask{ 0x00ff0000 };
	uint32_t green_mask{ 0x0000ff00 };
	uint32_t blue_mask{ 0x000000ff };
	uint32_t alpha_mask{ 0xff000000 };
	uint32_t color_space_type{ 0x73524742 };
	uint32_t unused[16]{ 0 };
};

#pragma pack(pop)

BMPFileHeader fileheader;

unsigned char* read_file(const char* filename){
    FILE* SourceImage = fopen(filename, "rb");
    if (!SourceImage){
        cout << "Файл не был открыт\n";
        return 0;
    }

    size_t FileHeaderSize = fread(&fileheader, sizeof(char), sizeof(BMPFileHeader), SourceImage);
    if (FileHeaderSize != sizeof(BMPFileHeader)){
        fclose(SourceImage);
        cout << "Заголовок BMP был прочитан неправильно (проблема с размером)!\n";
        return 0;
    }
    cout << FileHeaderSize << " = размер заголовка\n";

    int width = fileheader.width;
    int height = fileheader.height;
    int padding = (4 - width * 3 % 4) % 4;
    
    unsigned char* PixelInfo = new unsigned char[(3 * width + padding) * height];
    size_t BytesRead = fread(PixelInfo, sizeof(char), (3 * width + padding) * height, SourceImage);
    if (BytesRead != (3 * width + padding) * height){
        delete[] PixelInfo;
        fclose(SourceImage);
        cout << BytesRead << " - байтов прочитано, должно быть " << (3 * width + padding) * height << " байтов\n";
        cout << "Информация о пикселях прочитана неправильно\n";
        return 0;
    }
    fclose(SourceImage);
    return PixelInfo;
}

unsigned char* write_file(const char* filename, BMPFileHeader FileHeader, unsigned char* data){
    FILE* ImageWrite = fopen(filename, "wb");
    if (!ImageWrite){
        cout << "Файл не был открыт\n";
        return 0;
    }

    size_t FileHeaderSize = fwrite(&FileHeader, sizeof(char), sizeof(BMPFileHeader), ImageWrite);
    if (FileHeaderSize != sizeof(BMPFileHeader)){
        fclose(ImageWrite);
        cout << "Заголовок записан неправильно\n";
        return 0;
    }

    int padding = (4 - FileHeader.width * 3 % 4) % 4;

    size_t WrittenBytes = fwrite(data, sizeof(char), (3 * FileHeader.width + padding) * FileHeader.height, ImageWrite);
    if (WrittenBytes != (3 * FileHeader.width + padding) * FileHeader.height){
        cout << WrittenBytes << " / " << (3 * FileHeader.width + padding) * FileHeader.height << " байтов записано\n";
        fclose(ImageWrite);
    }
    else {
        cout << "Файл успешно записан\n";
    }

    fclose(ImageWrite);
    return 0;
}

unsigned char* turn_left(BMPFileHeader FileHeader, unsigned char* data){
    int w = FileHeader.width, h = FileHeader.height;
    int PaddingCurr = (4 - 3 * w % 4) % 4, PaddingSource = (4 - 3 * h % 4) % 4;
    unsigned char* Result = new unsigned char[(3 * w + PaddingCurr) * h];
    for (int y = 0; y < h; y++){
        for (int x = 0; x < w; x++){
            Result[(y * w + x) * 3 + y * PaddingCurr] = data[((w - 1 - x) * h + y) * 3 + (w - 1 - x) * PaddingSource];
            Result[(y * w + x) * 3 + 1 + y * PaddingCurr] = data[((w - 1 - x) * h + y) * 3 + 1 + (w - 1 - x) * PaddingSource];
            Result[(y * w + x) * 3 + 2 + y * PaddingCurr] = data[((w - 1 - x) * h + y) * 3 + 2 + (w - 1 - x) * PaddingSource];
        }
    }

    return Result;
}

unsigned char* turn_right(BMPFileHeader FileHeader, unsigned char* data){
    int w = FileHeader.width, h = FileHeader.height;
    int PaddingCurr = (4 - 3 * w % 4) % 4, PaddingSource = (4 - 3 * h % 4) % 4;
    unsigned char* Result = new unsigned char[(3 * w + PaddingCurr) * h];
    for (int y = 0; y < h; y++){
        for (int x = 0; x < w; x++){
            Result[(y * w + x) * 3 + y * PaddingCurr] = data[(x * h + h - 1 - y) * 3 + x * PaddingSource];
            Result[(y * w + x) * 3 + 1 + y * PaddingCurr] = data[(x * h + h - 1 - y) * 3 + 1 + x * PaddingSource];
            Result[(y * w + x) * 3 + 2 + y * PaddingCurr] = data[(x * h + h - 1 - y) * 3 + 2 + x * PaddingSource];
        }
    }

    return Result;
}

unsigned char* gaussian_blur(BMPFileHeader FileHeader, unsigned char* data){
    const int radius = 10, MatrixSize = 2 * radius + 1;
    const double pi = 3.14159, sigma = radius / 3;
    double sum = 0.0;

    double GausKernel[MatrixSize][MatrixSize];
    for (int y = -radius; y <= radius; y++){
        for (int x = -radius; x <= radius; x++){
            GausKernel[y + radius][x + radius] = exp(-(x * x + y * y) / (2 * sigma * sigma)) / (2 * pi * sigma * sigma);
            sum += GausKernel[y + radius][x + radius];
        }
    }
    cout << "Сумма элементов ядра Гаусса: " << sum << endl;

    for (int y = 0; y < MatrixSize; y++){
        for (int x = 0; x < MatrixSize; x++){
            GausKernel[y][x] /= sum;
        }
    }

    int padding = (4 - 3 * FileHeader.width % 4) % 4;
    unsigned char* Result = new unsigned char[(3 * FileHeader.width + padding) * FileHeader.height];
    for (size_t y = 0; y < FileHeader.height; y++){
        for (size_t x = 0; x < FileHeader.width; x++){
            double r = 0, g = 0, b = 0;
            for (int ShiftHeight = 0; ShiftHeight < MatrixSize; ShiftHeight++){
                for (int ShiftWidth = 0; ShiftWidth < MatrixSize; ShiftWidth++){
                    if (y + ShiftHeight - radius > 0 and y + ShiftHeight - radius < FileHeader.height and x + ShiftWidth - radius > 0 and x + ShiftWidth - radius < FileHeader.width){
                        r += GausKernel[ShiftHeight][ShiftWidth] * data[((y + ShiftHeight - radius) * FileHeader.width + x + ShiftWidth - radius) * 3 + padding * (y + ShiftHeight - radius)];
                        g += GausKernel[ShiftHeight][ShiftWidth] * data[((y + ShiftHeight - radius) * FileHeader.width + x + ShiftWidth - radius) * 3 + 1 + padding * (y + ShiftHeight - radius)];
                        b += GausKernel[ShiftHeight][ShiftWidth] * data[((y + ShiftHeight - radius) * FileHeader.width + x + ShiftWidth - radius) * 3 + 2 + padding * (y + ShiftHeight - radius)];
                    }
                    else{
                        r += GausKernel[ShiftHeight][ShiftWidth] * data[(y * FileHeader.width + x) * 3 + y * padding];
                        g += GausKernel[ShiftHeight][ShiftWidth] * data[(y * FileHeader.width + x) * 3 + 1 + y * padding];
                        b += GausKernel[ShiftHeight][ShiftWidth] * data[(y * FileHeader.width + x) * 3 + 2 + y * padding];
                    }
                }
            }
            Result[(y * FileHeader.width + x) * 3 + y * padding] = (int)r;
            Result[(y * FileHeader.width + x) * 3 + 1 + y * padding] = (int)g;
            Result[(y * FileHeader.width + x) * 3 + 2 + y * padding] = (int)b;
        }
    }

    return Result;
}

int main() {
    auto start_time = chrono::high_resolution_clock::now();

	unsigned char* PixelInfo = read_file("./img/640X426.bmp");
	cout << fileheader.file_size << " байт = размер файла\n";

	BMPFileHeader LeftRHeader = fileheader;
	swap(LeftRHeader.width, LeftRHeader.height);
	unsigned char* LeftRData = turn_left(LeftRHeader, PixelInfo);
	write_file("./img/Result-TurnedLeft.bmp", LeftRHeader, LeftRData);
	delete[] LeftRData;

	BMPFileHeader RightRHeader = fileheader;
	swap(RightRHeader.width, RightRHeader.height);
	unsigned char* RightRData = turn_right(RightRHeader, PixelInfo);
	write_file("./img/Result-TurnedRight.bmp", RightRHeader, RightRData);
	delete[] RightRData;

	BMPFileHeader GausHeader = fileheader;
	unsigned char* GausData = gaussian_blur(GausHeader, PixelInfo);
	write_file("./img/Result-GaussianFilter.bmp", fileheader, GausData);
	delete[] GausData;

	delete[] PixelInfo;

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    cout << "Время выполнения: " << duration.count() << " миллисекунд" << endl;

	return 0;
}

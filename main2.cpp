#include <iostream>
#include <fstream>
#include <vector>


#include <thread> // Для работы с потоками
#include <chrono> // Для работы с временем

// Отключаем выравнивание памяти для структур, чтобы гарантировать соответствие с BMP-файлами
#pragma pack(push, 1)

// Структура для хранения информации о пикселе в формате RGBQUAD
struct RGBQUAD {
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
};

// Структура для хранения информации заголовка BMP-файла (BITMAPFILEHEADER)
struct BITMAPFILEHEADER {
    uint16_t bfType;       // Сигнатура BMP-файла (должна быть "BM")
    uint32_t bfSize;       // Размер BMP-файла в байтах
    uint16_t bfReserved1;  // Зарезервировано (должно быть 0)
    uint16_t bfReserved2;  // Зарезервировано (должно быть 0)
    uint32_t bfOffBits;    // Смещение начала данных изображения
};

// Структура для хранения информации о заголовке BMP-изображения (BITMAPINFOHEADER)
struct BITMAPINFOHEADER {
    uint32_t biSize;          // Размер данной структуры (должен быть 40)
    int32_t biWidth;          // Ширина изображения в пикселях
    int32_t biHeight;         // Высота изображения в пикселях
    uint16_t biPlanes;        // Количество плоскостей (должно быть 1)
    uint16_t biBitCount;      // Битов на пиксель (24 для RGB)
    uint32_t biCompression;   // Тип сжатия (0 для несжатого изображения)
    uint32_t biSizeImage;     // Размер изображения в байтах (можно установить 0)
    int32_t biXPelsPerMeter;  // Горизонтальное разрешение в пикселях на метр (можно установить 0)
    int32_t biYPelsPerMeter;  // Вертикальное разрешение в пикселях на метр (можно установить 0)
    uint32_t biClrUsed;       // Количество цветов в палитре (можно установить 0)
    uint32_t biClrImportant;  // Количество "важных" цветов (можно установить 0)
};

// Включаем выравнивание памяти обратно
#pragma pack(pop)

// Функция для чтения BMP-файла и сохранения данных в структуры
void read_bmp(const char* fileName, BITMAPFILEHEADER& fileHeader, BITMAPINFOHEADER& fileInfoHeader, std::vector<std::vector<RGBQUAD>>& rgbInfo) {
    std::ifstream fileStream(fileName, std::ifstream::binary);
    if (!fileStream) {
        std::cout << "Error opening file '" << fileName << "'." << std::endl;
        return;
    }

    // Чтение заголовков файла и изображения
    fileStream.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    fileStream.read(reinterpret_cast<char*>(&fileInfoHeader), sizeof(fileInfoHeader));

    // Проверка на сигнатуру BMP-файла
    if (fileHeader.bfType != 0x4D42) {
        std::cout << "Error: '" << fileName << "' is not a BMP file." << std::endl;
        return;
    }

    const int width = fileInfoHeader.biWidth;
    const int height = fileInfoHeader.biHeight;

    // Выделение памяти для хранения данных об изображении
    rgbInfo.resize(height, std::vector<RGBQUAD>(width));

    const int padding = (4 - (width * sizeof(RGBQUAD)) % 4) % 4;

    // Чтение данных об изображении
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fileStream.read(reinterpret_cast<char*>(&rgbInfo[i][j]), sizeof(RGBQUAD));
        }
        // Пропуск дополнительных байтов, если они есть (отступы)
        fileStream.seekg(padding, std::ios::cur);
    }

    fileStream.close();
}

// Функция для записи данных изображения в BMP-файл
void write_bmp(const char* fileName, BITMAPFILEHEADER& fileHeader, BITMAPINFOHEADER& fileInfoHeader, const std::vector<std::vector<RGBQUAD>>& rgbInfo) {
    std::ofstream fileStream(fileName, std::ofstream::binary);
    if (!fileStream) {
        std::cout << "Error opening file '" << fileName << "'." << std::endl;
        return;
    }

    // Запись заголовков файла и изображения
    fileStream.write(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    fileStream.write(reinterpret_cast<char*>(&fileInfoHeader), sizeof(fileInfoHeader));
    
    const int width = fileInfoHeader.biWidth;
    const int height = fileInfoHeader.biHeight;

    // const int padding = (4 - (width * sizeof(RGBQUAD)) % 4) % 4;

    // Запись данных об изображении
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fileStream.write(reinterpret_cast<const char*>(&rgbInfo[i][j]), sizeof(RGBQUAD));
        }
        // Запись отступов, если они есть
        // for (int k = 0; k < padding; k++) {
        //     unsigned char paddingByte = 0;
        //     fileStream.write(reinterpret_cast<const char*>(&paddingByte), sizeof(unsigned char));
        // }
    }

    fileStream.close();
}

void rotate_image_90_degrees(const char* fileName, BITMAPFILEHEADER& fileHeader, std::vector<std::vector<RGBQUAD>>& rgbInfo, BITMAPINFOHEADER& fileInfoHeader) {
    const int height = rgbInfo.size();
    const int width = rgbInfo[0].size();
    std::vector<std::vector<RGBQUAD>> rotated(height, std::vector<RGBQUAD>(width));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            rotated[j][i] = rgbInfo[i][j];
        }
    }
    
    fileInfoHeader.biWidth = height;
    fileInfoHeader.biHeight = width;
    rgbInfo = std::move(rotated);
    write_bmp(fileName, fileHeader, fileInfoHeader, rgbInfo);
}


void rotate_image(const char* fileName, BITMAPFILEHEADER& fileHeader, std::vector<std::vector<RGBQUAD>>& rgbInfo, BITMAPINFOHEADER& fileInfoHeader) {
    const int height = rgbInfo.size();
    const int width = rgbInfo[0].size();
    std::vector<std::vector<RGBQUAD>> rotated(width, std::vector<RGBQUAD>(height));
    // RGBQUAD rotated[w][]
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            rotated[j][i] = rgbInfo[j][i];
            // if(rotated[j][i] == rgbInfo[i][j]){
            //     std::cout<<"True";
            //     std::cout<<"True";
            // }
            // if(int(rgbInfo[i][j].rgbBlue) != 0)
            //     std::wcout<<rgbInfo[j][i].rgbBlue<<" = "<<rgbInfo[j][i].rgbGreen<<" = "<<rgbInfo[j][i].rgbRed<<" => "<<rotated[i][j].rgbBlue<<" = "<<rotated[i][j].rgbGreen<<" = "<<rotated[i][j].rgbRed<<"\n";
            // Поставим задержку в 1 секунды
            // std::chrono::seconds duration(1);
            // std::this_thread::sleep_for(duration);
        // return;
        // std::cout<<i<<" "<<j<<"\n";
        }
    }
    
    // fileInfoHeader.biWidth = height;
    // fileInfoHeader.biHeight = width;
    // return rotated;
    // rgbInfo = std::move(rotated);
    // rgbInfo = (rotated);

    // if (rgbInfo[0].size() == (height)){
    //     std::cout<<"True";
    // }

    write_bmp(fileName, fileHeader, fileInfoHeader, rotated);
}

int main(int argc, char* argv[]) {
     std::locale::global(std::locale("en_US.UTF-8"));

    std::wcout << L"Привет, мир!" << std::endl;
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " input_file output_file" << std::endl;
        return 0;
    }

    const char* inputFileName = argv[1];
    const char* outputFileName = argv[2];

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER fileInfoHeader;
    std::vector<std::vector<RGBQUAD>> rgbInfo;

    // Чтение данных из входного BMP-файла
    read_bmp(inputFileName, fileHeader, fileInfoHeader, rgbInfo);

    // Поворот изображения на 90 градусов
    // rotate_image_90_degrees(outputFileName, fileHeader, rgbInfo, fileInfoHeader);
    rotate_image(outputFileName, fileHeader, rgbInfo, fileInfoHeader);

    // Запись обработанных данных в выходной BMP-файл
    // write_bmp(outputFileName, fiheightleHeader, fileInfoHeader, rgbInfo);

    return 0;
}



// --run-- sudo g++ -std=c++11 -o main main.cpp && ./main ./640X426.bmp ./out.bmp 


// --run-- g++ -std=c++11 -o main main.cpp -g 

// r ./640X426.bmp ./out.bmp

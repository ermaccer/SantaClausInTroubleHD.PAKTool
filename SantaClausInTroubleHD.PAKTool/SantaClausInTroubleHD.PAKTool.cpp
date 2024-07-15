// SantaClausInTroubleHD.PAKTool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>


struct pak_header {
    int header; //PKBE
    int field4;
    int field8;
    int fileDataStart;
    int directoryAmount;
    int unk;

};

struct pak_directory {
    int directoryID;
    int folderNameLen;
    int files;
    int field12;
    // char name[folderNameLen];
};

struct pak_file {
    int fileNameLen;
    int field4;
    int fileSize;
    int field12;
    int fileOffset;
    int field20;
    int field24;
    //char fileName[fileNameLen];
};


void changeEndINT(int* value)
{
    *value = (*value & 0x000000FFU) << 24 | (*value & 0x0000FF00U) << 8 | (*value & 0x00FF0000U) >> 8 | (*value & 0xFF000000U) >> 24;
}

int main(int argc, char* argv[])
{
    if (argc < 2 || argc == 1)
    {
        std::cout << "SantaClausInTroubleHD.PAKTool by ermaccer\n"
            << "Usage: scithdpaktool <file>\n";
        return 1;
    }
    std::string inputFile = argv[argc - 1];

    std::ifstream pFile(inputFile, std::ifstream::binary);

    if (!pFile)
    {
        std::cout << "ERROR: Could not open " << inputFile << "!" << std::endl;;
        return 1;
    }

    pak_header header;
    pFile.read((char*)&header, sizeof(pak_header));

    if (!(header.header == 'EBKP'))
    {
        std::cout << "ERROR: " << inputFile << "is not a valid SCIT HD .PAK file!" << std::endl;;
        return 1;
    }

    changeEndINT(&header.directoryAmount);

    std::cout << "Directories: " << header.directoryAmount << std::endl;

    for (int i = 0; i < header.directoryAmount; i++)
    {
        pak_directory directory;
        char folderName[256] = {};

        pFile.read((char*)&directory, sizeof(pak_directory));
        changeEndINT(&directory.files);
        changeEndINT(&directory.directoryID);
        changeEndINT(&directory.folderNameLen);

        pFile.read(folderName, directory.folderNameLen);

        std::string folder(folderName, directory.folderNameLen);
        if (folder[0] == '\\')
            folder = folder.substr(1, folder.length() - 1);

        std::filesystem::create_directories(folder);
        std::filesystem::path lastPath = std::filesystem::current_path();
        std::filesystem::current_path(folder);

        for (int a = 0; a < directory.files; a++)
        {
            pak_file file;
            pFile.read((char*)&file, sizeof(pak_file));
            changeEndINT(&file.fileNameLen);
            changeEndINT(&file.fileOffset);
            changeEndINT(&file.fileSize);

            char fileName[256] = {};
            pFile.read(fileName, file.fileNameLen);
            std::cout << "Processing: " << folderName << fileName << std::endl;

            unsigned int lastOffset = (unsigned int)pFile.tellg();

            int dataSize = file.fileSize;
            std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);

            pFile.seekg(file.fileOffset, pFile.beg);
            pFile.read(dataBuff.get(), dataSize);
            std::ofstream oFile(fileName, std::ofstream::binary);
            oFile.write(dataBuff.get(), dataSize);
            oFile.close();
            pFile.seekg(lastOffset, pFile.beg);

        }
        std::filesystem::current_path(lastPath);
    }
    pFile.close();

    return 0;
}

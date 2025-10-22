#include "FileManager.h"
#include <sstream>
#include <fstream>
#include <iostream>

// Constructors/Destructors


FileNamespace::FileManager::FileManager()
{
}

FileNamespace::FileManager::~FileManager()
{
}

// Methods

bool FileNamespace::FileManager::write(std::string fileName, std::string data, bool addMode)
{
    std::ofstream file;
    if (addMode)
    {
        // Abrir el archivo en modo de añadir (append)
        file.open(fileName, std::ios::app);
    }
    else
    {
        // Abrir el archivo en modo de sobrescritura (trunc)
        file.open(fileName, std::ios::trunc);
    }

    if (!file.is_open())
    {
        std::cerr << "Error: No se pudo abrir '" + fileName + "' para escritura." << std::endl;
        return false;
    }
    file << data;
    file.close();
    return true;
}

std::string FileNamespace::FileManager::read(std::string fileName) {
    std::ifstream archivo(fileName);
    if (!archivo.is_open())
    {
        // No lanzamos una excepción aquí, porque es normal que un archivo no exista.
        // Devolver un string vacío es un comportamiento esperado en este caso.
        return "";
    }
    std::stringstream buffer;
    buffer << archivo.rdbuf();
    archivo.close();
    return buffer.str();
}

std::string FileNamespace::FileManager::getCreationDate(std::string fileName) {
#ifdef stat
#undef stat
#endif
    struct stat result;
    if (stat(fileName.c_str(), &result) == 0) {
        return formatDate(result.st_ctime);
    }
    return "Fecha no disponible";
}

std::string FileNamespace::FileManager::getModificationDate(std::string fileName) {
#ifdef stat
#undef stat
#endif
    struct stat result;
    if (stat(fileName.c_str(), &result) == 0) {
        return formatDate(result.st_mtime);
    }
    return "Fecha no disponible";
}

std::string FileNamespace::FileManager::formatDate(time_t time)
{
    char buffer[80];
    struct tm *timeinfo = localtime(&time);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

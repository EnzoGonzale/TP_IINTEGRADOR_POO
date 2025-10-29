#include "TaskManager.h"
#include <fstream>
#include "FileManager.h"
#include "Logger.h"

TaskManager::TaskManager(const std::string& tasksFilePath)
    : tasksFilePath_(tasksFilePath) {}

bool TaskManager::loadTasks() {
    FileNamespace::FileManager fileManager;
    std::string fileContent = fileManager.read(tasksFilePath_);

    if (fileContent.empty()) {
        Logger::getInstance().log(LogLevel::ERROR, "[TaskManager] No se pudo leer el archivo de tareas o está vacío: " + tasksFilePath_);
        return false;
    }

    try {
        json tasksJson = json::parse(fileContent);
        
        // Limpiamos las tareas anteriores antes de cargar las nuevas
        tasks_.clear();

        for (const auto& item : tasksJson["tasks"]) {
            Task newTask;
            newTask.id = item.at("id").get<std::string>();
            newTask.name = item.at("name").get<std::string>();
            newTask.description = item.at("description").get<std::string>();
            newTask.gcode = item.at("gcode").get<std::vector<std::string>>();
            tasks_.push_back(newTask);
        }
        
        tasksLoaded_ = true;
        Logger::getInstance().log(LogLevel::INFO, "[TaskManager] " + std::to_string(tasks_.size()) + " tareas cargadas exitosamente desde " + tasksFilePath_);
        return true;

    } catch (json::parse_error& e) {
        Logger::getInstance().log(LogLevel::ERROR, "[TaskManager] Error al parsear JSON en " + tasksFilePath_ + ": " + e.what());
        return false;
    } catch (json::out_of_range& e) {
        Logger::getInstance().log(LogLevel::ERROR, "[TaskManager] Error: Atributo faltante en una tarea del JSON: " + std::string(e.what()));
        return false;
    }
}

const std::vector<Task>& TaskManager::getAvailableTasks() const {
    return tasks_;
}

std::optional<Task> TaskManager::getTaskById(const std::string& taskId) const {
    for (const auto& task : tasks_) {
        if (task.id == taskId) {
            return task;
        }
    }
    return std::nullopt; // Tarea no encontrada
}

bool TaskManager::addTask(const Task& newTask) {
    // Forzamos una recarga para asegurar que partimos del estado más reciente del archivo
    loadTasks();

    // Verificar si ya existe una tarea con el mismo ID
    for (const auto& task : tasks_) {
        if (task.id == newTask.id) {
            Logger::getInstance().log(LogLevel::ERROR, "[TaskManager] Ya existe una tarea con el ID '" + newTask.id + "'.");
            return false;
        }
    }

    // Añadir la nueva tarea a la lista en memoria
    tasks_.push_back(newTask);

    // Ahora, reconstruir el objeto JSON completo y guardarlo en el archivo
    json tasksJson;
    json tasksArray = json::array();
    for (const auto& task : tasks_) {
        json taskObj;
        taskObj["id"] = task.id;
        taskObj["name"] = task.name;
        taskObj["description"] = task.description;
        taskObj["gcode"] = task.gcode;
        tasksArray.push_back(taskObj);
    }
    tasksJson["tasks"] = tasksArray;

    // Guardar el JSON actualizado en el archivo
    FileNamespace::FileManager fileManager;
    return fileManager.write(tasksFilePath_, tasksJson.dump(2)); // Usamos dump(2) para que el JSON se guarde formateado
}

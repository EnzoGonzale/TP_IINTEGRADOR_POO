#include "TaskManager.h"
#include <iostream>
#include <fstream>
#include "FileManager.h"

TaskManager::TaskManager(const std::string& tasksFilePath)
    : tasksFilePath_(tasksFilePath) {}

bool TaskManager::loadTasks() {
    FileNamespace::FileManager fileManager;
    std::string fileContent = fileManager.read(tasksFilePath_);

    if (fileContent.empty()) {
        std::cerr << "[TaskManager] Error: No se pudo leer el archivo de tareas o está vacío: " << tasksFilePath_ << std::endl;
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
        std::cout << "[TaskManager] " << tasks_.size() << " tareas cargadas exitosamente desde " << tasksFilePath_ << std::endl;
        return true;

    } catch (json::parse_error& e) {
        std::cerr << "[TaskManager] Error al parsear JSON: " << e.what() << std::endl;
        return false;
    } catch (json::out_of_range& e) {
        std::cerr << "[TaskManager] Error: Atributo faltante en una tarea del JSON: " << e.what() << std::endl;
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
            std::cerr << "[TaskManager] Error: Ya existe una tarea con el ID '" << newTask.id << "'." << std::endl;
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

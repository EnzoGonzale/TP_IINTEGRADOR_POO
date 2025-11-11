#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <string>
#include <vector>
#include <optional>
#include "json.hpp" // Incluimos la librería para parsear JSON

// Usamos el alias 'json' para nlohmann::json para que sea más corto
using json = nlohmann::json;

/// @brief Representa una única tarea predefinida con su secuencia de G-Code.
struct Task {
    std::string id;
    std::string name;
    std::string description;
    std::vector<std::string> gcode;
};

/// @brief Gestiona la carga y el acceso a las tareas predefinidas desde un archivo JSON.
class TaskManager {
public:
    /// @brief Constructor que inicializa el gestor con la ruta al archivo de tareas.
    /// @param tasksFilePath La ruta al archivo JSON que contiene las tareas.
    TaskManager(const std::string& tasksFilePath);

    /// @brief Carga y parsea las tareas desde el archivo JSON.
    /// @return True si la carga fue exitosa, false en caso contrario.
    bool loadTasks();

    /// @brief Devuelve una lista de todas las tareas disponibles.
    /// @return Un vector con todas las tareas cargadas.
    const std::vector<Task>& getAvailableTasks() const;

    /// @brief Busca y devuelve una tarea específica por su ID.
    /// @param taskId El identificador único de la tarea a buscar.
    /// @return Un std::optional que contiene la tarea si se encuentra, o está vacío si no.
    std::optional<Task> getTaskById(const std::string& taskId) const;

    /// @brief Añade una nueva tarea a la lista y guarda el archivo JSON.
    /// @param newTask La tarea a añadir.
    /// @return True si la tarea fue añadida y guardada exitosamente, false en caso contrario.
    bool addTask(const Task& newTask);

private:
    std::string tasksFilePath_;
    std::vector<Task> tasks_;
    bool tasksLoaded_ = false;
};

#endif // TASKMANAGER_H

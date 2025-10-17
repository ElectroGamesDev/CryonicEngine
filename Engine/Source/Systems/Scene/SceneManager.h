#pragma once

#include "Scene.h"

class SceneManager {
public:
    SceneManager(); 
    static Scene* GetActiveScene();
    static void SetActiveScene(Scene* scene);
    static std::deque<Scene>* GetScenes();
    static bool SaveScene(Scene* scene);
    static bool LoadScene(std::filesystem::path filePath); // Todo: Should this return the scene?
    static void UnloadScene(Scene* scene);
    static void AddScene(Scene scene);
    static void CreateScene(std::filesystem::path path);
    static void ResetScene(Scene* scene);
    static void BackupScene(Scene* scene);

private:
    static std::deque<Scene> m_scenes;
    static Scene* m_activeScene;
};


/**
 * Spawns a game object from a sprite/template file at a specified position and rotation (Quaternion).
 *
 * @param path [std::string] - Relative path to the sprite/template file.
 * @param position [Vector3] - The world position to spawn the object at.
 * @param rotation [Quaternion] - The world rotation of the object as a quaternion.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnGameObject(std::string path, Vector3 position, Quaternion rotation);

/**
 * Spawns a game object from a sprite/template file.
 *
 * @param path [std::string] - Relative path to the sprite/template file.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnGameObject(std::string path);

/**
 * Spawns a game object from a sprite/template file at a specified position.
 *
 * @param path [std::string] - Relative path to the sprite/template file.
 * @param position [Vector3] - The world position to spawn the object at.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnGameObject(std::string path, Vector3 position);

/**
 * Spawns a game object from a sprite/template file at a specified position and rotation (Euler angle in degrees).
 *
 * @param path [std::string] - Relative path to the sprite/template file.
 * @param position [Vector3] - The world position to spawn the object at.
 * @param rotation [Vector3] - The world rotation of the object as an euler angle in degrees.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnGameObject(std::string path, Vector3 position, Vector3 rotation);

/**
 * Spawns a copy of an existing game object at a specified position and rotation (Quaternion).
 *
 * @param gameObject [GameObject*] - A pointer to the game object to clone..
 * @param position [Vector3] - The world position to spawn the cloned object at.
 * @param rotation [Quaternion] - The world rotation of the cloned object as a quaternion.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnGameObject(GameObject* gameObject, Vector3 position, Quaternion rotation);

/**
 * Spawns a copy of an existing game object.
 *
 * @param gameObject [GameObject*] - A pointer to the game object to clone.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnGameObject(GameObject* gameObject);

/**
 * Spawns a copy of an existing game object at a specified position.
 *
 * @param gameObject [GameObject*] - A pointer to the game object to clone.
 * @param position [Vector3] - The world position to spawn the cloned object at.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnGameObject(GameObject* gameObject, Vector3 position);

/**
 * Spawns a copy of an existing game object at a specified position and rotation (Euler angle in degrees).
 *
 * @param gameObject [GameObject*] - A pointer to the game object to clone..
 * @param position [Vector3] - The world position to spawn the cloned object at.
 * @param rotation [Vector3] - The world rotation of the cloned object as an euler angle in degrees.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnGameObject(GameObject* gameObject, Vector3 position, Vector3 rotation);


/**
 * Alias for SpawnGameObject(): Spawns a game object from a sprite/template file.
 *
 * @param path [std::string] - Relative path to the sprite/template file.
 * @return [GameObject*] - A pointer to the created game object.
 *
 */
static GameObject* SpawnObject(std::string path);

/**
 * Alias for SpawnGameObject(): Spawns a game object from a sprite/template file at a specified position.
 *
 * @param path [std::string] - Relative path to the sprite/template file.
 * @param position [Vector3] - The world position to spawn the object at.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnObject(std::string path, Vector3 position);

/**
 * Alias for SpawnGameObject(): Spawns a game object from a sprite/template file at a specified position and rotation (Quaternion).
 *
 * @param path [std::string] - Relative path to the sprite/template file.
 * @param position [Vector3] - The world position to spawn the object at.
 * @param rotation [Quaternion] - The world rotation of the object as a quaternion.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnObject(std::string path, Vector3 position, Quaternion rotation);

/**
 * Alias for SpawnGameObject(): Spawns a game object from a sprite/template file at a specified position and rotation (Euler angle in degrees).
 *
 * @param path [std::string] - Relative path to the sprite/template file.
 * @param position [Vector3] - The world position to spawn the object at.
 * @param rotation [Vector3] - The world rotation of the object as an euler angle in degrees.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnObject(std::string path, Vector3 position, Vector3 rotation);

/**
 * Alias for SpawnGameObject(): Spawns a copy of an existing game object.
 *
 * @param gameObject [GameObject*] - A pointer to the game object to clone.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnObject(GameObject* gameObject);

/**
 * Alias for SpawnGameObject(): Spawns a copy of an existing game object at a specified position.
 *
 * @param gameObject [GameObject*] - A pointer to the game object to clone.
 * @param position [Vector3] - The world position to spawn the cloned object at.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnObject(GameObject* gameObject, Vector3 position);

/**
 * Alias for SpawnGameObject(): Spawns a copy of an existing game object at a specified position and rotation (Quaternion).
 *
 * @param gameObject [GameObject*] - A pointer to the game object to clone..
 * @param position [Vector3] - The world position to spawn the cloned object at.
 * @param rotation [Quaternion] - The world rotation of the cloned object as a quaternion.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnObject(GameObject* gameObject, Vector3 position, Quaternion rotation);

/**
 * Alias for SpawnGameObject(): Spawns a copy of an existing game object at a specified position and rotation (Euler angle in degrees).
 *
 * @param gameObject [GameObject*] - A pointer to the game object to clone..
 * @param position [Vector3] - The world position to spawn the cloned object at.
 * @param rotation [Vector3] - The world rotation of the cloned object as an euler angle in degrees.
 * @return [GameObject*] - A pointer to the created game object.
 */
static GameObject* SpawnObject(GameObject* gameObject, Vector3 position, Vector3 rotation);

/**
 * Destroys the specified game object.
 *
 * @param gameObject [GameObject*] - A pointer to the game object.
 */
static void Destroy(GameObject* gameObject);
/**
 * Destroys the specified game component.
 *
 * @param component [Component*] - A pointer to the component.
 */
static void Destroy(Component* component);
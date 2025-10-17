#pragma once

#include "Core/GameObject.h"
#include <vector>
#include <deque>
#include <filesystem>
#include <algorithm>

class Scene {
public:
    Scene(const std::filesystem::path& path = {}, std::deque<GameObject*> gameObjects = {});
    ~Scene();

    std::filesystem::path GetPath();
    void SetPath(std::filesystem::path path);

    GameObject* AddGameObject(int id = 0);
    void RemoveGameObject(GameObject* gameObject);
    void DestroyGameObject(GameObject* gameObject);
    void Destroy(GameObject* gameObject);
    std::deque<GameObject*>& GetGameObjects();
    GameObject* GetGameObject(const std::string& name); 

    /**
     * Spawns a game object from a sprite/template file at a specified position and rotation (Quaternion).
     *
     * @param path [std::string] - Relative path to the sprite/template file.
     * @param position [Vector3] - The world position to spawn the object at.
     * @param rotation [Quaternion] - The world rotation of the object as a quaternion.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnGameObject(std::string path, Vector3 position, Quaternion rotation);

    /**
     * Spawns a game object from a sprite/template file.
     *
     * @param path [std::string] - Relative path to the sprite/template file.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnGameObject(std::string path);

    /**
     * Spawns a game object from a sprite/template file at a specified position.
     *
     * @param path [std::string] - Relative path to the sprite/template file.
     * @param position [Vector3] - The world position to spawn the object at.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnGameObject(std::string path, Vector3 position);

    /**
     * Spawns a game object from a sprite/template file at a specified position and rotation (Euler angle in degrees).
     *
     * @param path [std::string] - Relative path to the sprite/template file.
     * @param position [Vector3] - The world position to spawn the object at.
     * @param rotation [Vector3] - The world rotation of the object as an euler angle in degrees.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnGameObject(std::string path, Vector3 position, Vector3 rotation);

    /**
     * Spawns a copy of an existing game object at a specified position and rotation (Quaternion).
     *
     * @param gameObject [GameObject*] - A pointer to the game object to clone..
     * @param position [Vector3] - The world position to spawn the cloned object at.
     * @param rotation [Quaternion] - The world rotation of the cloned object as a quaternion.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnGameObject(GameObject* gameObject, Vector3 position, Quaternion rotation);

    /**
     * Spawns a copy of an existing game object.
     *
     * @param gameObject [GameObject*] - A pointer to the game object to clone.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnGameObject(GameObject* gameObject);

    /**
     * Spawns a copy of an existing game object at a specified position.
     *
     * @param gameObject [GameObject*] - A pointer to the game object to clone.
     * @param position [Vector3] - The world position to spawn the cloned object at.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnGameObject(GameObject* gameObject, Vector3 position);

    /**
     * Spawns a copy of an existing game object at a specified position and rotation (Euler angle in degrees).
     *
     * @param gameObject [GameObject*] - A pointer to the game object to clone..
     * @param position [Vector3] - The world position to spawn the cloned object at.
     * @param rotation [Vector3] - The world rotation of the cloned object as an euler angle in degrees.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnGameObject(GameObject* gameObject, Vector3 position, Vector3 rotation);


    /**
     * Alias for SpawnGameObject(): Spawns a game object from a sprite/template file.
     *
     * @param path [std::string] - Relative path to the sprite/template file.
     * @return [GameObject*] - A pointer to the created game object.
     * 
     */
    GameObject* SpawnObject(std::string path);

    /**
     * Alias for SpawnGameObject(): Spawns a game object from a sprite/template file at a specified position.
     *
     * @param path [std::string] - Relative path to the sprite/template file.
     * @param position [Vector3] - The world position to spawn the object at.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnObject(std::string path, Vector3 position);

    /**
     * Alias for SpawnGameObject(): Spawns a game object from a sprite/template file at a specified position and rotation (Quaternion).
     *
     * @param path [std::string] - Relative path to the sprite/template file.
     * @param position [Vector3] - The world position to spawn the object at.
     * @param rotation [Quaternion] - The world rotation of the object as a quaternion.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnObject(std::string path, Vector3 position, Quaternion rotation);

    /**
     * Alias for SpawnGameObject(): Spawns a game object from a sprite/template file at a specified position and rotation (Euler angle in degrees).
     *
     * @param path [std::string] - Relative path to the sprite/template file.
     * @param position [Vector3] - The world position to spawn the object at.
     * @param rotation [Vector3] - The world rotation of the object as an euler angle in degrees.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnObject(std::string path, Vector3 position, Vector3 rotation);

    /**
     * Alias for SpawnGameObject(): Spawns a copy of an existing game object.
     *
     * @param gameObject [GameObject*] - A pointer to the game object to clone.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnObject(GameObject* gameObject);

    /**
     * Alias for SpawnGameObject(): Spawns a copy of an existing game object at a specified position.
     *
     * @param gameObject [GameObject*] - A pointer to the game object to clone.
     * @param position [Vector3] - The world position to spawn the cloned object at.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnObject(GameObject* gameObject, Vector3 position);

    /**
     * Alias for SpawnGameObject(): Spawns a copy of an existing game object at a specified position and rotation (Quaternion).
     *
     * @param gameObject [GameObject*] - A pointer to the game object to clone..
     * @param position [Vector3] - The world position to spawn the cloned object at.
     * @param rotation [Quaternion] - The world rotation of the cloned object as a quaternion.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnObject(GameObject* gameObject, Vector3 position, Quaternion rotation);

    /**
     * Alias for SpawnGameObject(): Spawns a copy of an existing game object at a specified position and rotation (Euler angle in degrees).
     *
     * @param gameObject [GameObject*] - A pointer to the game object to clone..
     * @param position [Vector3] - The world position to spawn the cloned object at.
     * @param rotation [Vector3] - The world rotation of the cloned object as an euler angle in degrees.
     * @return [GameObject*] - A pointer to the created game object.
     */
    GameObject* SpawnObject(GameObject* gameObject, Vector3 position, Vector3 rotation);


    bool operator==(const Scene& other) const {
        return m_Path == other.m_Path;
    }

private:
    std::filesystem::path m_Path;
    std::deque<GameObject*> m_GameObjects;
};

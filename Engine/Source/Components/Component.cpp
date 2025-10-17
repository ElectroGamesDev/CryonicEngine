#include "Component.h"
#include "Utilities/ConsoleLogger.h"
#include <ctime>

std::vector<Component*> Component::markedForDeletion;

Component::Component(GameObject* obj, int id)
    : gameObject(obj), id(id)
{
    if (id == -1)
    {
        static bool seeded = false;
        if (!seeded)
        {
            srand(static_cast<unsigned int>(time(0)));
            seeded = true;
        }

        this->id = 100000 + rand() % 900000; // Todo: Make sure this is actually unique
    }
}

//Component::~Component()
//{
//}
//
//
//void Component::Update()
//{
//}
//
//void Component::Start()
//{
//}
//
//void Component::Destroy()
//{
//}
//
//GameObject& Component::GetGameObject() {
//    return _gameObject;
//}
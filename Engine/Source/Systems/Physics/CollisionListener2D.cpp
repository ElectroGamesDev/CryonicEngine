#include "CollisionListener2D.h"
#include "Components/Physics/Collider2D.h"
#include "Utilities/ConsoleLogger.h"
#include <deque>
#include <algorithm>
#include <list>

std::list<b2Contact*> continuedContact;

void CollisionListener2D::BeginContact(b2Contact* contact)
{
    Collider2D* colliderA = reinterpret_cast<Collider2D*>(contact->GetFixtureA()->GetUserData().pointer);
    Collider2D* colliderB = reinterpret_cast<Collider2D*>(contact->GetFixtureB()->GetUserData().pointer);

    if (colliderA && colliderB)
    {
        for (Component* component : colliderA->gameObject->GetComponents())
            component->OnCollisionEnter2D(colliderB);
        for (Component* component : colliderB->gameObject->GetComponents())
            component->OnCollisionEnter2D(colliderA);
    }

    continuedContact.push_back(contact);
}

void CollisionListener2D::EndContact(b2Contact* contact)
{
    Collider2D* colliderA = reinterpret_cast<Collider2D*>(contact->GetFixtureA()->GetUserData().pointer);
    Collider2D* colliderB = reinterpret_cast<Collider2D*>(contact->GetFixtureB()->GetUserData().pointer);

    // Checking if they're null separately since they should still callback even if the other collider is now null. Components are responsible for ensuring they're valid
    if (colliderA)
        for (Component* component : colliderA->gameObject->GetComponents())
            component->OnCollisionExit2D(colliderB);

    if (colliderB)
        for (Component* component : colliderB->gameObject->GetComponents())
            component->OnCollisionExit2D(colliderA);

    auto it = std::find(continuedContact.begin(), continuedContact.end(), contact);
    if (it != continuedContact.end())
        continuedContact.erase(it);
}

void CollisionListener2D::ContinueContact()
{
    for (b2Contact* contact : continuedContact)
    {
        Collider2D* colliderA = reinterpret_cast<Collider2D*>(contact->GetFixtureA()->GetUserData().pointer);
        Collider2D* colliderB = reinterpret_cast<Collider2D*>(contact->GetFixtureB()->GetUserData().pointer);

        if (colliderA && colliderB)
        {
            for (Component* component : colliderA->gameObject->GetComponents())
                component->OnCollisionStay2D(colliderB);
            for (Component* component : colliderB->gameObject->GetComponents())
                component->OnCollisionStay2D(colliderA);
        }                           
    }
}

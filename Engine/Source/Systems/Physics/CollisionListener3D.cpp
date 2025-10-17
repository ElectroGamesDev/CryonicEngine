#include "CollisionListener3D.h"
#include "Components/Physics/Collider3D.h"
#include "Components/Physics/Rigidbody3D.h"

// BodyID hashing doesn't seem to work
//std::unordered_map<std::pair<JPH::BodyID, JPH::BodyID>, std::pair<Collider3D*, Collider3D*>> contactMap;

void CollisionListener3D::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
    Collider3D* colliderA;
    Collider3D* colliderB;

    if (inBody1.GetUserData() != 0)
        colliderA = reinterpret_cast<Collider3D*>(inBody1.GetUserData());
    else
        colliderA = Rigidbody3D::colliderMap[inBody1.GetShape()->GetSubShapeUserData(inManifold.mSubShapeID1)];

    if (inBody2.GetUserData() != 0)
        colliderB = reinterpret_cast<Collider3D*>(inBody2.GetUserData());
    else
        colliderB = Rigidbody3D::colliderMap[inBody2.GetShape()->GetSubShapeUserData(inManifold.mSubShapeID2)];

    //contactMap[std::make_pair(inBody1.GetID(), inBody2.GetID())] = std::make_pair(colliderA, colliderB);

    if (colliderA && colliderB)
    {
        for (Component* component : colliderA->gameObject->GetComponents())
            component->OnCollisionEnter3D(colliderB);
        for (Component* component : colliderB->gameObject->GetComponents())
            component->OnCollisionEnter3D(colliderA);
    }
}

void CollisionListener3D::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
    Collider3D* colliderA;
    Collider3D* colliderB;

    if (inBody1.GetUserData() != 0)
        colliderA = reinterpret_cast<Collider3D*>(inBody1.GetUserData());
    else
        colliderA = Rigidbody3D::colliderMap[inBody1.GetShape()->GetSubShapeUserData(inManifold.mSubShapeID1)];

    if (inBody2.GetUserData() != 0)
        colliderB = reinterpret_cast<Collider3D*>(inBody2.GetUserData());
    else
        colliderB = Rigidbody3D::colliderMap[inBody2.GetShape()->GetSubShapeUserData(inManifold.mSubShapeID2)];

    if (colliderA && colliderB)
    {
        for (Component* component : colliderA->gameObject->GetComponents())
            component->OnCollisionExit3D(colliderB);
        for (Component* component : colliderB->gameObject->GetComponents())
            component->OnCollisionExit3D(colliderA);
    }
}

void CollisionListener3D::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
{
    //if (auto it = contactMap.find(std::make_pair(inSubShapePair.GetBody1ID(), inSubShapePair.GetBody2ID())); it != contactMap.end())
    //{
    //    Collider3D* colliderA = it->second.first;
    //    Collider3D* colliderB = it->second.second;

    //    // Checking if they're null separately since they should still callback even if the other collider is now null. Components are responsible for ensuring they're valid
    //    if (colliderA)
    //        for (Component* component : colliderA->gameObject->GetComponents())
    //            component->OnCollisionStay3D(colliderB);

    //    if (colliderB)
    //        for (Component* component : colliderB->gameObject->GetComponents())
    //            component->OnCollisionStay3D(colliderA);

    //    contactMap.erase(it);
    //}
}
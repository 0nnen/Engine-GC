#pragma once

#include "Component.h"
#include "Convertisseur.h"

#include <glm.hpp>

#include <btBulletDynamicsCommon.h>

class RigidBody : public Component
{
public:
    /**
    * @brief Constructeur de la classe RigidBody.
    *
    * @param _dynamicsWorld Un pointeur vers le monde dynamique de physique Bullet.
    * @param _shape La forme de collision du corps rigide.
    * @param _mass La masse du corps rigide.
    * @param _position La position initiale du corps rigide.
    */
    RigidBody(btDiscreteDynamicsWorld* _dynamicsWorld, btCollisionShape* _shape, float _mass, glm::vec3& _position);
    /**
    * @brief Ajoute une force au corps rigide.
    *
    * @param _force La force � ajouter au corps rigide.
    */
    void AddForce(glm::vec3& _force);
    /**
    * @brief D�finit la v�locit� du corps rigide.
    *
    * @param _velocity La v�locit� � d�finir pour le corps rigide.
    */
    void SetGravity(glm::vec3& _gravity);
    /**
    * @brief R�cup�re la v�locit� du corps rigide.
    *
    * @return La v�locit� actuelle du corps rigide.
    */
    void SetVelocity(glm::vec3& _velocity);
    /**
    * @brief D�finit la gravit� agissant sur le corps rigide.
    *
    * @param _gravity La gravit� � d�finir pour le corps rigide.
    */
    btVector3 GetVelocity();
    /**
    * @brief D�finit si le corps rigide est cin�matique.
    *
    * @param _isKinematic True pour d�finir le corps rigide comme cin�matique, sinon false.
    */
    void SetKinematic(bool _isKinematic);
    /**
    * @brief Verrouille ou d�verrouille la rotation du corps rigide sur les axes sp�cifi�s.
    *
    * @param _lockRotationX Si vrai, verrouille la rotation sur l'axe X, sinon la rotation sur cet axe est libre.
    * @param _lockRotationY Si vrai, verrouille la rotation sur l'axe Y, sinon la rotation sur cet axe est libre.
    * @param _lockRotationZ Si vrai, verrouille la rotation sur l'axe Z, sinon la rotation sur cet axe est libre.
    */
    void SetRotationLocked(bool _lockRotationX, bool _lockRotationY, bool _lockRotationZ);
    /**
    * @brief D�finit la position du corps rigide avec des options de verrouillage d'axes.
    *
    * @param _position La nouvelle position du corps rigide.
    * @param lockX Si vrai, verrouille la position sur l'axe X.
    * @param lockY Si vrai, verrouille la position sur l'axe Y.
    * @param lockZ Si vrai, verrouille la position sur l'axe Z.
    */
    void SetPosition(glm::vec3& _position, bool lockX, bool lockY, bool lockZ);
    /**
    * @brief Destructeur de l'objet RigidBody, lib�rant les ressources associ�es.
    */
    ~RigidBody();
private:
    btDiscreteDynamicsWorld* dynamicsWorld;
    btRigidBody* rigidBody;
};


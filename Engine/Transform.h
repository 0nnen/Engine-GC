#pragma once

#include <iostream>
#include <cmath>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Component.h"

class Transform final : public Component
{
public:
	// M�thode pour r�cup�rer et d�finir la position
	inline glm::vec3 GetPosition() const { return position; }
	inline void SetPosition(const float& _x, const float& _y, const float& _z) { position = glm::vec3(_x, _y, _z); }
	inline void SetPosition(const glm::vec3& _position) { position = _position; }

	// M�thode pour r�cup�rer et d�finir la rotation en utilisant un angle
	inline float GetRotation() const { return rotation; }
	inline void SetRotation(float angle) { rotation = angle; }

	// M�thode pour r�cup�rer et d�finir l'�chelle
	inline glm::vec3 GetScale() const { return scale; }
	inline void SetScale(const float& _scaleX, const float& _scaleY, const float& _scaleZ) { scale = glm::vec3(_scaleX, _scaleY, _scaleZ); }
	inline void SetScale(const glm::vec3& _scale) { scale = _scale; }

	glm::vec3 TransformPoint() const
	{


		// Cr�ation de la matrice de transformation
		glm::mat4 rotationMatrix = glm::mat4(1.0f);
		rotationMatrix = glm::translate(rotationMatrix, position);
		rotationMatrix = glm::rotate(rotationMatrix, rotation, glm::vec3(1.0f, 0.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
		rotationMatrix = glm::scale(rotationMatrix, scale);
		// Transformation du point
		glm::vec4 transformedPoint = rotationMatrix * glm::vec4(position, 1.0f);

		// R�cup�ration des coordonn�es transform�es
		return glm::vec3(transformedPoint.x, transformedPoint.y, transformedPoint.z);
	}
	// M�thode pour afficher les propri�t�s du Transform
	void Display();

private:
	glm::vec3 vectorZero = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 vectorOne = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 position = vectorZero;
	glm::vec3 scale = vectorOne;
	float rotation = 0.f;
};



//
//  Camera.cpp
//  Lab5
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#include "Camera.hpp"

namespace gps {
    
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget)
    {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
    
    glm::mat4 Camera::getViewMatrix()
    {
        return glm::lookAt(cameraPosition, cameraPosition + cameraDirection , glm::vec3(0.0f, 1.0f, 0.0f));
    }

	glm::vec3 Camera::getCameraTarget()
	{
		return cameraTarget;
	}

	glm::vec3 Camera::getCameraPosition()
	{
		return this->cameraPosition;
	}

	void Camera::setCameraPositionY(float y)
	{
		this->cameraPosition.y = y;
	}
    
    void Camera::move(MOVE_DIRECTION direction, float speed)
    {
        switch (direction) {
            case MOVE_FORWARD:
                cameraPosition += cameraDirection * speed;
                break;
                
            case MOVE_BACKWARD:
                cameraPosition -= cameraDirection * speed;
                break;
                
            case MOVE_RIGHT:
                cameraPosition += cameraRightDirection * speed;
                break;
                
            case MOVE_LEFT:
                cameraPosition -= cameraRightDirection * speed;
                break;
        }
    }
    
    void Camera::rotate(float pitch, float yaw)
    {
		cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		this->cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));

		cameraDirection.y = sin(glm::radians(pitch));
		cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraDirection = glm::normalize(cameraDirection);
		cameraUp = glm::normalize(cross(cameraRightDirection, cameraDirection));
    }

	glm::vec3 Camera::getBezierInterpolatedPoint(std::vector<glm::vec3> points, float t)
	{
		float var1, var2, var3;
		glm::vec3 interpolatedPoint = glm::vec3(1.0f);

		// Bernstein polynomial for 4 points:
		// B(t) = P1 * ( 1 - t )^3 + P2 * 3 * t * ( 1 - t )^2 + P3 * 3 * t^2 * ( 1 - t ) + P4 * t^3 

		// Store the (1 - t) in a variable because it is used frequently
		var1 = 1 - t;

		// Store the (1 - t)^3 into a variable to cut down computation and create clean code
		var2 = var1 * var1 * var1;

		// Store the t^3 in a variable to cut down computation and create clean code
		var3 = t * t * t;

		interpolatedPoint.x = var2 * points.at(0).x + 3 * t*var1*var1*points.at(1).x +
							3 * t*t*var1*points.at(2).x + var3 * points.at(3).x;
		interpolatedPoint.y = var2 * points.at(0).y + 3 * t*var1*var1*points.at(1).y + 
							3 * t*t*var1*points.at(2).y + var3 * points.at(3).y;
		interpolatedPoint.z = var2 * points.at(0).z + 3 * t*var1*var1*points.at(1).z + 
							3 * t*t*var1*points.at(2).z + var3 * points.at(3).z;

		// Now we should have the point on the curve, so let's return it.
		return interpolatedPoint;
	}

	void Camera::moveAlongBezierCurve(std::vector<glm::vec3> points, float t)
	{
		//for (float t = 0; t <= (1 + (1.0f / time)); t += 1.0f / time)
		{
			// Get the current point on the curve, depending on the time.
			glm::vec3 interpolatedPoint = this->getBezierInterpolatedPoint(points, t);
			glm::vec3 targetPoint = this->getBezierInterpolatedPoint(points, t + 0.1f);
			//resultPoints.push_back(interpolatedPoint);

			cameraPosition = interpolatedPoint;
			cameraTarget = targetPoint;
			cameraDirection = glm::normalize(cameraTarget - cameraPosition);
			cameraRightDirection = glm::normalize(glm::cross(cameraDirection, glm::vec3(0, 1, 0)));
		}
	}
    
}

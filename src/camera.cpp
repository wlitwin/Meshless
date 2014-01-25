#include "camera.hpp"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

const float PIOVER180 = 3.1415926f / 180.0f;

//=============================================================================
// Constructor(float, float, float, float)
//=============================================================================

Camera::Camera(const float fov, const float aspect_ratio,
				const float near, const float far) :
	m_pos(0.0f),
	m_rot(glm::vec3(0.0f))
{
	SetPerspective(fov, aspect_ratio, near, far);
}

//=============================================================================
// Constructor(default)
//=============================================================================

Camera::Camera() :
	m_pos(0.0f),
	m_rot(glm::vec3(0.0f))
{
	SetPerspective(45.0f, 4.0f/3.0f, 0.1f, 1000.0f);
}

//=============================================================================
// Constructor(float, float, float, float, float, float)
//=============================================================================

Camera::Camera(const float left, const float right,
			const float bottom, const float top,
			const float near, const float far)
{
	SetOrthographic(left, right, bottom, top, near, far);
}

//=============================================================================
// SetPos
//=============================================================================

void Camera::SetPos(const glm::vec3& pos)
{
	m_pos = pos;
}

//=============================================================================
// LookAt
//=============================================================================

void Camera::LookAt(const glm::vec3& eye, const glm::vec3& center, 
					const glm::vec3& up)
{
	m_pos = eye;

	// Easiest way is to just create a lookAt matrix and then convert
	// the rotational part to a quaternion.
	const glm::mat4 look_at = glm::lookAt(eye, center, glm::normalize(up));
	m_rot = glm::toQuat(look_at);
}

//=============================================================================
// MoveForward
//=============================================================================

void Camera::MoveForward(const float amount)
{
	m_pos -= glm::vec3(0.0f, 0.0f, amount) * m_rot;
}

//=============================================================================
// MoveUp
//=============================================================================

void Camera::MoveUp(const float amount)
{
	m_pos += glm::vec3(0.0f, amount, 0.0f) * m_rot;
}

//=============================================================================
// Strafe
//=============================================================================

void Camera::Strafe(const float amount)
{
	m_pos += glm::vec3(amount, 0.0f, 0.0f) * m_rot;
}

//=============================================================================
// Yaw
//=============================================================================

void Camera::Yaw(const float amount)
{
	Rotate(amount, 0);
}

//=============================================================================
// Pitch
//=============================================================================

void Camera::Pitch(const float amount)
{
	Rotate(0, amount);
}

//=============================================================================
// Rotate
//=============================================================================

void Camera::Rotate(const float yaw, const float pitch)
{
	glm::fquat q_yaw(glm::angleAxis(yaw, 0.f, 1.f, 0.f));
	glm::fquat q_pitch(glm::angleAxis(pitch, 1.f, 0.f, 0.f));

	m_rot = q_pitch * m_rot * q_yaw;
	m_rot = glm::normalize(m_rot);
}

//=============================================================================
// SetPerspective
//=============================================================================

void Camera::SetPerspective(const float fov, const float aspect_ratio, 
		const float near, const float far)
{
	m_proj = glm::perspective(fov, aspect_ratio, near, far);
}

//=============================================================================
// SetOrthographic
//=============================================================================

void Camera::SetOrthographic(const float left, 
						const float right, 
						const float bottom, 
						const float top,
						const float near,
						const float far)
{
	m_proj = glm::ortho(left, right, bottom, top, near, far);
}


//=============================================================================
// GetPosition
//=============================================================================

glm::vec3 Camera::GetPosition() const
{
	return m_pos;
}

//=============================================================================
// GetPosition
//=============================================================================

glm::vec3 Camera::GetForward() const
{
	glm::vec3 forward(0.0f, 0.0f, 1.0f);
	return forward * m_rot;
}

//=============================================================================
// GetView
//=============================================================================

const glm::mat4& Camera::GetView()
{
	const glm::mat4 trans = glm::translate(glm::mat4(1.0f), -m_pos);
	const glm::mat4 view = glm::toMat4(m_rot);
	m_view = view * trans;

	return m_view;
}

//=============================================================================
// GetProj
//=============================================================================

const glm::mat4& Camera::GetProj() const
{
	return m_proj;
}

//=============================================================================
// 
//=============================================================================

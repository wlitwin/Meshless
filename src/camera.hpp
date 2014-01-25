#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera
{
public:
	
	/* Create a new camera object using perspective projection.
	 *
	 * Params:
	 *   fov - The desired field of view for this camera.
	 *   aspect_ratio - The aspect ratio of the screen.
	 *   near - The distance to the near plane.
	 *   far  - The distance to the far plane.
	 */
	Camera(const float fov, const float aspect_ratio, 
			const float near, const float far);

	/* Initializes the camera to a default of:
	 *
	 * fov - 45.0 degrees
	 * aspect_ratio - 4:3
	 * near - 0.1
	 * far - 1000
	 *
	 * SetPerspective() or SetOrthographic() can be called to
	 * change these settings.
	 */
	Camera();

	/* Create a new camera object using orthographic projection.
	 *
	 * Params:
	 *   left - The left plane of the camera
	 *   right - The right plane of the camera
	 *   bottom - The bottom plane of the camera
	 *   top - The top plane of the camera
	 *   near - The near plane distance
	 *   far - The far plane distance
	 */
	Camera(const float left, const float right,
			const float bottom, const float top,
			const float near, const float far);

	/* Set the absolute position of the camera in World Space.
	 *
	 * Param:
	 *   pos - The new absolute position of the camera.
	 */
	void SetPos(const glm::vec3& pos);

	/* Move forward (or backward is a negative amount is specified)
	 * in the direction the camera is facing. In other words along
	 * the camera's Z axis.
	 *
	 * Params:
	 *   amount - How much to move the camera in world units. By
	 *            specifying a negative amount the camera will
	 *            move backwards along its Z axis.
	 */
	void MoveForward(const float amount);

	/* Translate in the direction of the camera's up vector by
	 * amount units. Specify a negative amount to move down.
	 */
	void MoveUp(const float amount);

	/* Strafe the camera (Left or Right).
	 *
	 * This strafes the camera along the camera's X axis. A positive
	 * value indicates moving along positive X (Right) and a
	 * negative value indicates moving along negative X (Left).
	 *
	 * Params:
	 *   amount - How much to move the camera in world units.
	 */
	void Strafe(const float amount);

	/* Yaw the camera around the camera's Y axis.
	 *
	 * amount - degrees to rotate
	 */
	void Yaw(const float amount);

	/* Pitch the camera around the camera's X axis.
	 *
	 * amount - degrees to rotate
	 */
	void Pitch(const float amount);

	/* Get the direction the camera is facing in world coordinates.
	 */
	glm::vec3 GetForward() const;

	/* Currently roll is disabled
	 */
	void Rotate(const float yaw, const float pitch);

	/* Moves the camera to a specified location in world coordinates and
	 * points the camera toward a target point in world space. 
	 * The orientation can also be specified.
	 *
	 * Params:
	 *   eye - The position of the camera in world coordinates.
	 *   center - The point in world space that the camera will 'Look At'.
	 *   up - Which direction is up for the camera (in world space coordinates).
	 */
	void LookAt(const glm::vec3& eye, 
				const glm::vec3& center, 
				const glm::vec3& up);

	/* Set the camera's perspective projection matrix.
	 *
	 * Params:
	 *   fov - The desired field of view for this camera.
	 *   aspect_ratio - The aspect ratio of the screen.
	 *   near - The distance to the near plane.
	 *   far  - The distance to the far plane.
	 */
	void SetPerspective(const float fov, 
						const float aspect_ratio, 
						const float near, 
						const float far);

	/* Set the camera's orthographic projection matrix
	 *
	 * Params:
	 *   left - The left plane of the camera
	 *   right - The right plane of the camera
	 *   bottom - The bottom plane of the camera
	 *   top - The top plane of the camera
	 *   near - The near plane distance
	 *   far - The far plane distance
	 */
	void SetOrthographic(const float left, 
						const float right, 
						const float bottom, 
						const float top,
						const float near,
						const float far);

	/* Get the curret position of the camera in world coordinates.
	 */
	glm::vec3 GetPosition() const;

	/* Get the camera's view matrix.
	 */
	const glm::mat4& GetView();

	/* Get the camera's projection matrix.
	 */
	const glm::mat4& GetProj() const;

// Class Variables
private:
	glm::mat4 m_proj;
	glm::mat4 m_view;
	glm::vec3 m_pos;
	glm::fquat m_rot;
};

#endif

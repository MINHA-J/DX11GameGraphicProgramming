#include "Camera/Camera.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::Camera

      Summary:  Constructor

      Modifies: [m_yaw, m_pitch, m_moveLeftRight, m_moveBackForward,
                 m_moveUpDown, m_travelSpeed, m_rotationSpeed,
                 m_padding, m_cameraForward, m_cameraRight, m_cameraUp,
                 m_eye, m_at, m_up, m_rotation, m_view].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Camera::Camera(const XMVECTOR& position)
        : m_yaw(0.0f)
        , m_pitch(0.0f)
        , m_moveLeftRight(0.0f)
        , m_moveBackForward(0.0f)
        , m_moveUpDown(0.0f)
        , m_travelSpeed(4.0f)
        , m_rotationSpeed(4.0f)
        , m_padding(0)
        , m_cameraForward(DEFAULT_FORWARD)
        , m_cameraRight(DEFAULT_RIGHT)
        , m_cameraUp(DEFAULT_UP)
        , m_eye(XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f))
        , m_at(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
        , m_up(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
        , m_rotation()
        , m_view()
    { 
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::GetEye

      Summary:  Returns the eye vector

      Returns:  const XMVECTOR&
                  The eye vector
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const XMVECTOR& Camera::GetEye() const
    {
        return m_eye;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::GetAt

      Summary:  Returns the at vector

      Returns:  const XMVECTOR&
                  The at vector
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const XMVECTOR& Camera::GetAt() const
    {
        return m_at;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::GetUp

      Summary:  Returns the up vector

      Returns:  const XMVECTOR&
                  The up vector
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const XMVECTOR& Camera::GetUp() const
    {
        return m_up;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::GetView

      Summary:  Returns the view matrix

      Returns:  const XMMATRIX&
                  The view matrix
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const XMMATRIX& Camera::GetView() const
    {
        return m_view;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::HandleInput

      Summary:  Sets the camera state according to the given input

      Args:     const DirectionsInput& directions
                  Keyboard directional input
                const MouseRelativeMovement& mouseRelativeMovement
                  Mouse relative movement input
                FLOAT deltaTime
                  Time difference of a frame

      Modifies: [m_yaw, m_pitch, m_moveLeftRight, m_moveBackForward,
                 m_moveUpDown].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Camera::HandleInput(_In_ const DirectionsInput& directions,
        _In_ const MouseRelativeMovement& mouseRelativeMovement,
        _In_ FLOAT deltaTime)
    {
        m_yaw += mouseRelativeMovement.X * deltaTime * m_rotationSpeed;
        m_pitch += mouseRelativeMovement.Y * deltaTime * m_rotationSpeed;

        float limit = (float)XM_PIDIV2 - 0.01f;
        m_pitch = (float)__max(-limit, m_pitch);
        m_pitch = (float)__min(+limit, m_pitch);

        if (directions.bRight)
            m_moveLeftRight += deltaTime * m_travelSpeed;
        if (directions.bLeft)
            m_moveLeftRight += deltaTime * m_travelSpeed * -1.0;

        if (directions.bFront)
            m_moveBackForward += deltaTime * m_travelSpeed;
        if (directions.bBack)
            m_moveBackForward += deltaTime * m_travelSpeed * -1.0;

        if (directions.bUp)
            m_moveUpDown += deltaTime * m_travelSpeed;
        if (directions.bDown)
            m_moveUpDown += deltaTime * m_travelSpeed * -1.0;

        Update(deltaTime);
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Camera::Update

      Summary:  Updates the camera based on its state

      Args:     FLOAT deltaTime
                  Time difference of a frame

      Modifies: [m_rotation, m_at, m_cameraRight, m_cameraUp,
                 m_cameraForward, m_eye, m_moveLeftRight,
                 m_moveBackForward, m_moveUpDown, m_up, m_view].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Camera::Update(_In_ FLOAT deltaTime)
    {
        // Rotating the Camera
        m_rotation = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0);
        m_at = XMVector3TransformCoord(DEFAULT_FORWARD, m_rotation);
        m_at = XMVector3Normalize(m_at);

        XMMATRIX RotateYTempMatrix;
        RotateYTempMatrix = XMMatrixRotationY(m_yaw);

        // Update the camForward, camUp, and camRight Vectors
        m_cameraForward = XMVector3TransformCoord(DEFAULT_FORWARD, RotateYTempMatrix);
        m_cameraRight = XMVector3TransformCoord(DEFAULT_RIGHT, RotateYTempMatrix);
        m_cameraUp = XMVector3TransformCoord(m_cameraUp, RotateYTempMatrix);
        
        // Moving the Camera
        m_eye += m_moveLeftRight * m_cameraRight;
        m_eye += m_moveBackForward * m_cameraForward;
        m_eye += m_moveUpDown * m_cameraUp;

        // reset movement
        m_moveLeftRight = 0.0f;
        m_moveBackForward = 0.0f;
        m_moveUpDown = 0.0f;

        // determine the view matrix
        m_at = m_eye + m_at;
        
        // Set the camView Matrix
        m_view = XMMatrixLookAtLH(m_eye, m_at, m_up);
    }

}
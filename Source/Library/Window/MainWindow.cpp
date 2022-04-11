#include "Window/MainWindow.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::Initialize

      Summary:  Initializes main window

      Args:     HINSTANCE hInstance
                  Handle to the instance
                INT nCmdShow
                    Is a flag that says whether the main application window
                    will be minimized, maximized, or shown normally
                PCWSTR pszWindowName
                    The window name

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT MainWindow::Initialize(_In_ HINSTANCE hInstance, _In_ INT nCmdShow, _In_ PCWSTR pszWindowName)
    {
        static bool raw_input_initialized = false;
        if (raw_input_initialized == false)
        {
            RAWINPUTDEVICE rid;
            rid.usUsagePage = 0x01; //Mouse
            rid.usUsage = 0x02;
            rid.dwFlags = 0;
            rid.hwndTarget = NULL;

            if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
            {
                return E_FAIL;
            }

            raw_input_initialized = true;
        }

        return initialize(hInstance, nCmdShow, pszWindowName, 0, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, 0, 0);
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetWindowClassName

      Summary:  Returns the name of the window class

      Returns:  PCWSTR
                  Name of the window class
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    PCWSTR MainWindow::GetWindowClassName() const
    {
        return m_pszWindowName;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::HandleMessage

      Summary:  Handles the messages

      Args:     UINT uMessage
                  Message code
                WPARAM wParam
                    Additional data the pertains to the message
                LPARAM lParam
                    Additional data the pertains to the message

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    LRESULT MainWindow::HandleMessage(_In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        switch (uMsg)
        {
            case WM_CLOSE:
            {
                PostQuitMessage(0);
                return  E_FAIL;
            }
            case WM_DESTROY:
            {
                DestroyWindow(m_hWnd);
                return E_FAIL;
            }
            case WM_INPUT:
            {
                RECT rc;
                POINT p1, p2;

                GetClientRect(m_hWnd, &rc);    // 클라이언트 크기
                // 클라이언트 크기를 좌표로 변환
                p1.x = rc.left;
                p1.y = rc.top;
                p2.x = rc.right;
                p2.y = rc.bottom;
                // 클라이언트 크기를 스크린 크기로 변환
                ClientToScreen(m_hWnd, &p1);
                ClientToScreen(m_hWnd, &p2);
                rc.left = p1.x;
                rc.top = p1.y;
                rc.right = p2.x;
                rc.bottom = p2.y;
                //해당 좌표를 기준으로 커서를 고정
                ClipCursor(&rc);

                WCHAR szDebugMessage[64];

                UINT dataSize;
                GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));

                if (dataSize > 0)
                {
                    std::unique_ptr<BYTE[]> rawdata = std::make_unique<BYTE[]>(dataSize);
                    if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawdata.get(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
                    {
                        RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get()); 
                        if (raw->header.dwType == RIM_TYPEMOUSE)
                        {
                            m_mouseRelativeMovement.X = raw->data.mouse.lLastX;
                            m_mouseRelativeMovement.Y = raw->data.mouse.lLastY;
                        }
                    }
                }
                return 0;
            }

            case WM_KEYDOWN:
            {
                switch (wParam)
                {
                    // WM_KEYDOWN 메시지는 wParam에 문자 코드가 아닌 가상 키코드라는 것을 전달
                    case 0x41:
                    {// Left
                        m_directions.bLeft = TRUE;
                        break;
                    }
                    case 0x44:
                    {// Right
                        m_directions.bRight = TRUE;
                        break;
                    }
                    case 0x57:
                    {// front
                        m_directions.bFront = TRUE;
                        break;
                    }
                    case 0x53:
                    {// Back
                        m_directions.bBack = TRUE;
                        break;
                    }
                    case VK_SPACE:
                    {// Up
                        m_directions.bUp = TRUE;
                        break;
                    }
                    case VK_SHIFT:
                    {//Down
                        m_directions.bDown = TRUE;
                        break;
                    }
                }
                return 0;
            }

            case WM_KEYUP:
            {
                switch (wParam)
                {
                case 0x41:
                {// Left
                    m_directions.bLeft = FALSE;
                    break;
                }
                case 0x44:
                {// Right
                    m_directions.bRight = FALSE;
                    break;
                }
                case 0x57:
                {// front
                    m_directions.bFront = FALSE;
                    break;
                }
                case 0x53:
                {// Back
                    m_directions.bBack = FALSE;
                    break;
                }
                case VK_SPACE:
                {// Up
                    m_directions.bUp = FALSE;
                    break;
                }
                case VK_SHIFT:
                {//Down
                    m_directions.bDown = FALSE;
                    break;
                }
                }
                return 0;
            }
            default:
                return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetDirections
      Summary:  Returns the keyboard direction input
      Returns:  const DirectionsInput&
                  Keyboard direction input
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const DirectionsInput& MainWindow::GetDirections() const
    {
        return m_directions;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::GetMouseRelativeMovement
      Summary:  Returns the mouse relative movement
      Returns:  const MouseRelativeMovement&
                  Mouse relative movement
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const MouseRelativeMovement& MainWindow::GetMouseRelativeMovement() const
    {
        return m_mouseRelativeMovement;
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   MainWindow::ResetMouseMovement
      Summary:  Reset the mouse relative movement to zero
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void MainWindow::ResetMouseMovement()
    {
        m_mouseRelativeMovement.X = 0.0;
        m_mouseRelativeMovement.Y = 0.0;
    }

}

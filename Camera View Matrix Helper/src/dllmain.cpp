// dllmain.cpp
#include "pch.h"

// Helper to compute a view matrix from camera position and center, since the game doesn't provide one directly.
Ogre::Matrix4 ComputeCameraViewMatrix(
    const Ogre::Vector3& eye,
    const Ogre::Vector3& center,
    const Ogre::Vector3& up = Ogre::Vector3::UNIT_Y
)
{
    Ogre::Vector3 forward = center - eye;
    if (forward.squaredLength() < 1e-6f)
    {
        DebugLog("[Camera View Matrix Helper] ComputeCameraViewMatrix: forward too small, returning identity");
        return Ogre::Matrix4::IDENTITY;
    }

    Ogre::Vector3 F = forward.normalisedCopy();
    Ogre::Vector3 R = F.crossProduct(up);

    if (R.squaredLength() < 1e-6f)
    {
        Ogre::Vector3 fallbackUp = Ogre::Vector3::UNIT_Z;
        R = F.crossProduct(fallbackUp);
        if (R.squaredLength() < 1e-6f)
        {
            DebugLog("[Camera View Matrix Helper] ComputeCameraViewMatrix: forward parallel to up and fallback, returning identity");
            return Ogre::Matrix4::IDENTITY;
        }
    }

    R.normalise();
    Ogre::Vector3 U = R.crossProduct(F);

    Ogre::Matrix4 view = Ogre::Matrix4::IDENTITY;
    // Rotation rows (each basis vector occupies one row)
    view[0][0] = R.x;  view[0][1] = R.y;  view[0][2] = R.z;
    view[1][0] = U.x;  view[1][1] = U.y;  view[1][2] = U.z;
    view[2][0] = -F.x; view[2][1] = -F.y; view[2][2] = -F.z;

    // Translation in column 3 (Ogre is row-major, m[row][col], translation at m[row][3])
    view[0][3] = -R.dotProduct(eye);
    view[1][3] = -U.dotProduct(eye);
    view[2][3] = F.dotProduct(eye);

    return view;
}

// MyGUI overlay/window
namespace CameraOverlay
{
    struct State
    {
        MyGUI::Window* window;
        MyGUI::TextBox* text;

        Ogre::Vector3 cameraCenter;
        Ogre::Vector3 cameraPos;
        Ogre::Matrix4 viewMatrix;

        State()
            : window(NULL)
            , text(NULL)
            , cameraCenter(Ogre::Vector3::ZERO)
            , cameraPos(Ogre::Vector3::ZERO)
            , viewMatrix(Ogre::Matrix4::IDENTITY)
        {
        }
    };

    inline State& Get()
    {
        static State s;
        return s;
    }
}

void CreateCameraOverlay()
{
    CameraOverlay::State& s = CameraOverlay::Get();
    if (s.window != NULL)
        return;

    MyGUI::Gui* gui = MyGUI::Gui::getInstancePtr();
    if (!gui)
        return;

    s.window = gui->createWidgetReal<MyGUI::Window>(
        "Kenshi_WindowCX",
        MyGUI::FloatCoord(0.02f, 0.02f, 0.25f, 0.3f),
        MyGUI::Align::Default,
        "Window",
        "CameraOverlay");

    if (!s.window)
        return;

    s.window->setCaption("Camera Debug");

    s.text = s.window->createWidgetReal<MyGUI::TextBox>(
        "Kenshi_GenericTextBox",
        MyGUI::FloatCoord(0.05f, 0.05f, 0.9f, 0.9f),
        MyGUI::Align::Stretch);

    if (s.text)
        s.text->setCaption("Waiting for camera data...");
}

void UpdateCameraOverlay()
{
    CameraOverlay::State& s = CameraOverlay::Get();
    if (!s.text)
        return;

    s.viewMatrix = ComputeCameraViewMatrix(s.cameraPos, s.cameraCenter);

    std::ostringstream ss;
    ss << "Center:\n(" << s.cameraCenter.x << ", " << s.cameraCenter.y << ", " << s.cameraCenter.z << ")\n\n";
    ss << "Pos:\n(" << s.cameraPos.x << ", " << s.cameraPos.y << ", " << s.cameraPos.z << ")\n\n";
    ss << "View Matrix:\n";

    for (int row = 0; row < 4; ++row)
    {
        ss << "[ ";
        for (int col = 0; col < 4; ++col)
            ss << s.viewMatrix[row][col] << " ";
        ss << "]\n";
    }

    s.text->setCaption(ss.str());
}

// Hooks
static void* (*getCameraCenter_orig)(const GameWorld*, Ogre::Vector3*) = NULL;
static void* getCameraCenter_hook(const GameWorld* thisptr, Ogre::Vector3* result)
{
    void* ret = getCameraCenter_orig(thisptr, result);

    if (result)
    {
        CameraOverlay::Get().cameraCenter = *result;

        std::ostringstream ss;
        ss << "[Camera View Matrix Helper] getCameraCenter: (" << result->x << ", " << result->y << ", " << result->z << ")";
        DebugLog(ss.str());

        UpdateCameraOverlay();
    }

    return ret;
}

static void* (*getCameraPos_orig)(const GameWorld*, Ogre::Vector3*) = NULL;
static void* getCameraPos_hook(const GameWorld* thisptr, Ogre::Vector3* result)
{
    void* ret = getCameraPos_orig(thisptr, result);

    if (result)
    {
        CameraOverlay::Get().cameraPos = *result;

        std::ostringstream ss;
        ss << "[Camera View Matrix Helper] getCameraPos: (" << result->x << ", " << result->y << ", " << result->z << ")";
        DebugLog(ss.str());

        UpdateCameraOverlay();
    }

    return ret;
}

bool g_isCameraOverlayInitialized = false;

static void(*InputHandler_keyDownEvent_orig)(InputHandler*, OIS::KeyCode) = NULL;
static void InputHandler_keyDownEvent_hook(InputHandler* thisptr, OIS::KeyCode keyCode)
{
    InputHandler_keyDownEvent_orig(thisptr, keyCode);

    if (keyCode == OIS::KC_F1)
    {
        if (!g_isCameraOverlayInitialized)
        {
            CreateCameraOverlay();
            g_isCameraOverlayInitialized = true;
            DebugLog("[Camera View Matrix Helper] CameraOverlay created via F1");
		}
        else
        {
            UpdateCameraOverlay();
            DebugLog("[Camera View Matrix Helper] CameraOverlay refreshed via F1");
        }
    }
}

// Plugin entry
__declspec(dllexport) void startPlugin()
{
    DebugLog("[Camera View Matrix Helper] startPlugin called.");

    if (KenshiLib::SUCCESS != KenshiLib::AddHook(
        KenshiLib::GetRealAddress(&InputHandler::keyDownEvent),
        InputHandler_keyDownEvent_hook,
        &InputHandler_keyDownEvent_orig))
    {
        ErrorLog("[Camera View Matrix Helper] Failed to hook InputHandler::keyDownEvent.");
    }

    if (KenshiLib::SUCCESS != KenshiLib::AddHook(
        KenshiLib::GetRealAddress(&GameWorld::getCameraCenter),
        getCameraCenter_hook,
        &getCameraCenter_orig))
    {
        ErrorLog("[Camera View Matrix Helper] Failed to hook GameWorld::getCameraCenter.");
    }

    if (KenshiLib::SUCCESS != KenshiLib::AddHook(
        KenshiLib::GetRealAddress(&GameWorld::getCameraPos),
        getCameraPos_hook,
        &getCameraPos_orig))
    {
        ErrorLog("[Camera View Matrix Helper] Failed to hook GameWorld::getCameraPos.");
    }
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

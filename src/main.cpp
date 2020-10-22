#include <chrono>
  
#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "modloader/shared/modloader.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp" 
#include "beatsaber-hook/shared/utils/typedefs.h"

static ModInfo modInfo;

const Logger& getLogger() {
  static const Logger& logger(modInfo);
  return logger;
}

#define WAIT_TIME 175

static bool isInMenu = false;
static long long LastThumbstickVertical = 0;

Il2CppObject* GetFirstObjectOfType(Il2CppClass* klass) {
    auto* sysType = RET_0_UNLESS(il2cpp_utils::GetSystemType(klass));
    auto* objects = RET_0_UNLESS(il2cpp_utils::RunMethod<Array<Il2CppObject*>*>(
        "UnityEngine", "Resources", "FindObjectsOfTypeAll", sysType));
    RET_0_UNLESS(objects);
    return objects->values[0];
}

MAKE_HOOK_OFFSETLESS(SceneManager_SetActiveScene, bool, int scene) {
    bool value = SceneManager_SetActiveScene(scene);
    auto* nameObject = RET_UNLESS(value, il2cpp_utils::RunMethod<Il2CppString*>("UnityEngine.SceneManagement", "Scene", "GetNameInternal", scene));
    RET_UNLESS(value, nameObject);
    std::string name = to_utf8(csstrtostr(nameObject));
    getLogger().info("Scene: %s", name.c_str());
    isInMenu = name == "MenuViewControllers";
    return value;
}

MAKE_HOOK_OFFSETLESS(VRPlatformHelper_Update, void, Il2CppObject* self) {
    VRPlatformHelper_Update(self);
    if (!isInMenu) return;

    long long currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();;
    float thumbstickVertical = il2cpp_utils::RunMethod<float>("UnityEngine", "Input", "GetAxis", il2cpp_utils::createcsstr("VerticalRightHand")).value_or(0);
    if (thumbstickVertical == 0.0f) {
        thumbstickVertical = RET_V_UNLESS(il2cpp_utils::RunMethod<float>("UnityEngine", "Input", "GetAxis", il2cpp_utils::createcsstr("VerticalLeftHand")));
    }

    auto* levelCollectionViewController = RET_V_UNLESS(GetFirstObjectOfType(il2cpp_utils::GetClassFromName("", "LevelCollectionViewController")));
    auto* levelCollectionTableView = RET_V_UNLESS(il2cpp_utils::GetFieldValue(levelCollectionViewController, "_levelCollectionTableView"));
    auto* tableView = RET_V_UNLESS(il2cpp_utils::GetFieldValue(levelCollectionTableView, "_tableView"));

    if (thumbstickVertical < -0.5f && currentTime-LastThumbstickVertical > WAIT_TIME) {
        LastThumbstickVertical = currentTime;
        auto* pageUpButton = RET_V_UNLESS(il2cpp_utils::GetFieldValue(tableView, "_pageUpButton"));
        auto* onClick = RET_V_UNLESS(il2cpp_utils::RunMethod(pageUpButton, "get_onClick"));
        il2cpp_utils::RunMethod(onClick, "Invoke");
    } else if (thumbstickVertical > 0.5f && currentTime-LastThumbstickVertical > WAIT_TIME) {
        LastThumbstickVertical = currentTime;
        auto* pageDownButton = RET_V_UNLESS(il2cpp_utils::GetFieldValue(tableView, "_pageDownButton"));
        auto* onClick = RET_V_UNLESS(il2cpp_utils::RunMethod(pageDownButton, "get_onClick"));
        il2cpp_utils::RunMethod(onClick, "Invoke");
    } else if (thumbstickVertical == 0.0f) {
        LastThumbstickVertical = 0;
    }
}

extern "C" void setup(ModInfo& info) 
{
    modInfo.id = "SongHelper";
    modInfo.version = VERSION;
    info = modInfo;
}

extern "C" void load() {
    il2cpp_functions::Init();
    getLogger().info("Starting SongHelper installation...");
    INSTALL_HOOK_OFFSETLESS(SceneManager_SetActiveScene, il2cpp_utils::FindMethodUnsafe("UnityEngine.SceneManagement", "SceneManager", "SetActiveScene", 1));
    INSTALL_HOOK_OFFSETLESS(VRPlatformHelper_Update, il2cpp_utils::FindMethodUnsafe("", "VRPlatformHelper", "Update", 0));
    getLogger().info("Successfully installed SongHelper!");
}

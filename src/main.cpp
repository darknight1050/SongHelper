#include "../include/main.hpp"

#define WAIT_TIME 175

static bool isInMenu = false;

static long long LastThumbstickVertical = 0;

Il2CppObject* GetFirstObjectOfType(Il2CppClass* klass){
    Array<Il2CppObject*>* objects;
    il2cpp_utils::RunMethod(&objects, il2cpp_utils::GetClassFromName("UnityEngine", "Resources"), "FindObjectsOfTypeAll", il2cpp_functions::type_get_object(il2cpp_functions::class_get_type(klass)));
    if (objects != nullptr)
    {
        return objects->values[0];
    }
    else
    {
        return nullptr;
    }
}

MAKE_HOOK_OFFSETLESS(SceneManager_SetActiveScene, bool, int scene)
{
    bool value = SceneManager_SetActiveScene(scene);
    Il2CppString* nameObject;
    il2cpp_utils::RunMethod(&nameObject, "UnityEngine.SceneManagement", "Scene", "GetNameInternal", scene);
    const char* name = to_utf8(csstrtostr(nameObject)).c_str();
    log(INFO, "Scene: %s", name);
    isInMenu = strcmp(name, "MenuViewControllers") == 0;
    return value;
}

MAKE_HOOK_OFFSETLESS(VRPlatformHelper_Update, void, Il2CppObject* self)
{
    VRPlatformHelper_Update(self);
    if(isInMenu){
        long long currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();;
        float thumbstickVertical = 0.0f;
        il2cpp_utils::RunMethod(&thumbstickVertical, "UnityEngine", "Input", "GetAxis", il2cpp_utils::createcsstr("VerticalRightHand"));
        if(thumbstickVertical == 0.0f){
            il2cpp_utils::RunMethod(&thumbstickVertical, "UnityEngine", "Input", "GetAxis", il2cpp_utils::createcsstr("VerticalLeftHand"));
        }
        Il2CppObject* levelCollectionViewController = GetFirstObjectOfType(il2cpp_utils::GetClassFromName("", "LevelCollectionViewController"));    
        if(levelCollectionViewController != nullptr){        
            Il2CppObject* levelCollectionTableView = il2cpp_utils::GetFieldValue(levelCollectionViewController, "_levelCollectionTableView");
            if(levelCollectionTableView != nullptr){
                Il2CppObject* tableView = il2cpp_utils::GetFieldValue(levelCollectionTableView, "_tableView");
                if(tableView != nullptr){
                    if(thumbstickVertical < -0.5f && currentTime-LastThumbstickVertical > WAIT_TIME){
                        Il2CppObject* pageUpButton = il2cpp_utils::GetFieldValue(tableView, "_pageUpButton");
                        if(pageUpButton != nullptr){
                            Il2CppObject* onClick;
                            il2cpp_utils::RunMethod(&onClick, pageUpButton, "get_onClick");
                            il2cpp_utils::RunMethod(onClick, "Invoke");
                        }
                        LastThumbstickVertical = currentTime;
                    }else
                        if(thumbstickVertical > 0.5f && currentTime-LastThumbstickVertical > WAIT_TIME){
                        Il2CppObject* pageDownButton = il2cpp_utils::GetFieldValue(tableView, "_pageDownButton");
                        if(pageDownButton != nullptr){
                            Il2CppObject* onClick;
                            il2cpp_utils::RunMethod(&onClick, pageDownButton, "get_onClick");
                            il2cpp_utils::RunMethod(onClick, "Invoke");
                        }
                        LastThumbstickVertical = currentTime;
                    }else if(thumbstickVertical == 0.0f){
                        LastThumbstickVertical = 0;
                    }
                }
            }
        }
    }
   
}

extern "C" void load()
{
    #ifdef __aarch64__
    log(INFO, "Is 64 bit!");
    #endif
    log(INFO, "Starting SongHelper installation...");
    il2cpp_functions::Init();
    INSTALL_HOOK_OFFSETLESS(SceneManager_SetActiveScene, il2cpp_utils::FindMethodUnsafe("UnityEngine.SceneManagement", "SceneManager", "SetActiveScene", 1));
    INSTALL_HOOK_OFFSETLESS(VRPlatformHelper_Update, il2cpp_utils::FindMethodUnsafe("", "VRPlatformHelper", "Update", 0));
    log(INFO, "Successfully installed SongHelper!");
}
## features

i might have stolen it from spooks [SeOwnedDE](https://github.com/spook953/SEOwnedDE-public/blob/main/SEOwnedDE/SEOwnedDE/src/Utils/Config/Config.h) but recoded it for my use case

- automatic save/load for:
  - `bool`
  - `int`
  - `float`
  - `std::string`
  - `ImColor` (optional)
- namespace paths `"Globals::Main"` are converted to nested json automatically.
- `CFGVAR` for regular config vars.
- `CFGVAR_NOSAVE` for vars that should never be saved.
- .h only, almost no overhead, no heavy stl usage per var.

---

## example code
```cpp
#include <iostream>
#include <filesystem>
#include "configsaver.h"

std::string ConfigPath;

bool SetupPath() { // using documents
    char docs_path[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, docs_path) != S_OK)
        return false;

    ConfigPath = std::string(docs_path) + "\\ConfigSystem"; // change this to whatever you want
    std::filesystem::create_directories(ConfigPath + "\\configs");
    return true;
}

namespace Globals {
    CFGVAR(Main, ExampleBool, false);
    CFGVAR(Main::Element, ExampleInt, 24);
    CFGVAR_NOSAVE(Main::Element, RuntimeOnlyVar, 50); // if you want something to only be saved in runtime and not in the config
}

int main() {
    SetupPath();

    std::string CfgFile = ConfigPath + "\\configs\\config.json";

    // Load existing config
    Config::Load(CfgFile);
    std::cout << "Initial ExampleBool: " << Globals::Main::ExampleBool << "\n";

    // Toggle and save
    Globals::Main::ExampleBool = !Globals::Main::ExampleBool;
    std::cout << "ExampleBool after toggle: " << Globals::Main::ExampleBool << "\n";
    Config::Save(CfgFile);

    // Reset and reload
    globals::Visual::Player::Box = false;
    Config::Load(CfgFile);
    std::cout << "ExampleBool after reload: " << Globals::Main::ExampleBool << "\n";

    return 0;
}
```

## json output from example 
```json
{
    "Main": {
        "ExampleBool": false
        "Element": {
            "ExampleInt": 24
        }
    }
}
```

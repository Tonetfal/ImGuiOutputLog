# ImGui Output Log

Unreal Engine's native output log, but using Dear ImGui.

## Installation

1) Install [UnrealImGui](https://github.com/segross/UnrealImGui).
2) Clone the repository in your project's Plugin directory.

## Preview

![Preview01](imgui_outputlog_preview_01.png)

![Preview02](imgui_outputlog_preview_02.png)

## Usage

The plugin comes with Game Instance subsystem that you can activate or deactivate which will show/hide the output log 
accordingly. Once activated, it'll appear on the screen and ImGui will get the inputs, so game or UMG will not get any. 
To deactivate, press the X at the top right. Alternatively, you can call SetActive with a false.

![Use Example](imgui_outputlog_use.png)

![Settings](imgui_outputlog_settings.png)

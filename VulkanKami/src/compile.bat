@echo off
for %%f in (shaders\*.vert) do (
    X:\Vulkan\VulkanSDK\Bin\glslc.exe %%f -o %%f.spv
)
for %%f in (shaders\*.frag) do (
    X:\Vulkan\VulkanSDK\Bin\glslc.exe %%f -o %%f.spv
)
echo Compilation Complete.
pause

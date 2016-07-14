#include <windows.h>
#include "vulkan/vulkan.h"

#if defined _M_X64
#pragma comment(lib, "vklibx64/vulkan-1.lib")
#elif defined _M_IX86
#pragma comment(lib, "vklibx86/vulkan-1.lib") 
#endif

#include "MinHook/include/MinHook.h" //detour x86&x64
//add all minhook files to your project

//==========================================================================================================================

bool FirstInit = false; //init once
int countnum = -1;

#include <fstream>
using namespace std;
char dlldir[320];
char* GetDirectoryFile(char *filename)
{
	static char path[320];
	strcpy_s(path, dlldir);
	strcat_s(path, filename);
	return path;
}

void Log(const char *fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	ofstream logfile(GetDirectoryFile("log.txt"), ios::app);
	if (logfile.is_open() && text)	logfile << text << endl;
	logfile.close();
}

//==========================================================================================================================

typedef void(*func_vkCmdDrawIndexed_t) (VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
func_vkCmdDrawIndexed_t ovkCmdDrawIndexed;

void hvkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	//random log
	//commandBuffer == 271658976 && indexCount == 35496 && instanceCountt == 1 && firstIndex == 0 && vertexOffset == 0 && firstInstance == 0
	//commandBuffer == 271658976 && indexCount == 765 && instanceCountt == 1 && firstIndex == 0 && vertexOffset == 0 && firstInstance == 0

	//model rec
	//indexCount == 1698 //big health pack
	//indexCount == 1272 //big armor
	//indexCount == 35496 //monster1
	//indexCount == 45603 //monster2

	if(indexCount == 35496|| indexCount == 45603|| indexCount == 1698|| indexCount == 1272)
	{
		//does NOT work
		//vkCmdSetDepthBounds(commandBuffer, 0.0, 0.9);
		//vkCmdSetDepthBounds(commandBuffer, 0.5, 1);
		//vkCmdSetDepthBias(commandBuffer, 16.0, 16.0, 16.0);
		//ovkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
		//vkCmdSetDepthBias(commandBuffer, 1.0, 0.0, 2.5);
		//vkCmdSetDepthBounds(commandBuffer, 0, 0.5);
		//vkCmdSetDepthBounds(commandBuffer, 0.1, 1.0);
	}

	//hold down P key until a texture changes, press I to log values of those textures
	if (GetAsyncKeyState('O') & 1) //-
		countnum--;
	if (GetAsyncKeyState('P') & 1) //+
		countnum++;
	if ((GetAsyncKeyState(VK_MENU)) && (GetAsyncKeyState('9') & 1)) //reset, set to -1
		countnum = -1;
	if (countnum == indexCount/100)
		if (GetAsyncKeyState('I') & 1) //press I to log to log.txt
			Log("indexCount == %d", indexCount);

	if (countnum == indexCount/100)
	{
		return;//delete texture
	}
		

	ovkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

//==========================================================================================================================

typedef void(*func_vkCmdDrawIndexedIndirect_t) (VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
func_vkCmdDrawIndexedIndirect_t ovkCmdDrawIndexedIndirect;

void hvkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
	//random log
	//commandBuffer == 340001424 && buffer == 200070720 && offset == 1536 && drawCount == 1 && stride == 0
	//commandBuffer == 339955296 && buffer == 200070720 && offset == 0 && drawCount == 1 && stride == 0

	ovkCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

//==========================================================================================================================

typedef void(*func_vkCmdBindVertexBuffers_t) (VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
func_vkCmdBindVertexBuffers_t ovkCmdBindVertexBuffers;

void hvkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
	//if (GetAsyncKeyState(VK_F10) & 1) 
		//Log("commandBuffer == %d && firstBinding == %d && bindingCount == %d && pBuffers == %d && pOffsets == %d", commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);

	//random log
	//commandBuffer == 268025360 && firstBinding == 0 && bindingCount == 1 && pBuffers == 150469912 && pOffsets == 150469936
	//commandBuffer == 268071488 && firstBinding == 0 && bindingCount == 1 && pBuffers == 212729064 && pOffsets == 212729088

	ovkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

//==========================================================================================================================

DWORD WINAPI VulkanInit(__in  LPVOID lpParameter)
{
	while (GetModuleHandle("vulkan-1.dll") == 0)
	{
		Sleep(100);
	}

	HMODULE mod = LoadLibrary(TEXT("vulkan-1"));
	//HMODULE mod = GetModuleHandle("vulkan-1.dll");
	void* aptr = GetProcAddress(mod, "vkCmdDrawIndexed");
	void* bptr = GetProcAddress(mod, "vkCmdDrawIndexedIndirect");

	if (mod)
	{
		MH_Initialize();
		MH_CreateHook(aptr, hvkCmdDrawIndexed, reinterpret_cast<void**>(&ovkCmdDrawIndexed));
		MH_EnableHook(aptr);

		//MH_Initialize();
		MH_CreateHook(bptr, hvkCmdDrawIndexedIndirect, reinterpret_cast<void**>(&ovkCmdDrawIndexedIndirect));
		MH_EnableHook(bptr);
	}

	return 1;
}

//==========================================================================================================================

BOOL __stdcall DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls (hinstDll);
		GetModuleFileName(hinstDll, dlldir, 512);
		for (int i = strlen(dlldir); i > 0; i--) { if (dlldir[i] == '\\') { dlldir[i + 1] = 0; break; } }
			
		CreateThread(0, 0, VulkanInit, 0, 0, 0); //init
		break;

		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}


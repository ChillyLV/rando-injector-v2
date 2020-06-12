#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

unsigned long dw_find_process_id(const std::string_view process_name) noexcept {
	tagPROCESSENTRY32 process_entry;
	process_entry.dwSize = sizeof(process_entry);
	void* p_process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (p_process_snapshot != INVALID_HANDLE_VALUE) {
		Process32First(p_process_snapshot, &process_entry);

		if (!process_name.compare(process_entry.szExeFile)) {
			CloseHandle(p_process_snapshot);
			return process_entry.th32ProcessID;
		}

		while (Process32Next(p_process_snapshot, &process_entry)) {
			if (!process_name.compare(process_entry.szExeFile)) {
				CloseHandle(p_process_snapshot);
				return process_entry.th32ProcessID;
			}
		}

		CloseHandle(p_process_snapshot);
	}

	return NULL;
}

int __stdcall wWinMain(_In_ HINSTANCE__* h_instance, _In_opt_ HINSTANCE__* h_previous_instance, _In_ wchar_t* cmd_line, _In_ int i_show_cmd) {
	char dll_path[MAX_PATH];
	GetFullPathNameA("dllname.dll", MAX_PATH, dll_path, NULL);

	void* p_process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, dw_find_process_id("exename.exe"));

	if (p_process != INVALID_HANDLE_VALUE) {
		void* p_allocated_memory = VirtualAllocEx(p_process, NULL, sizeof(dll_path), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		if (p_allocated_memory != nullptr) {
			WriteProcessMemory(p_process, p_allocated_memory, dll_path, sizeof(dll_path), NULL);

			void* p_thread = CreateRemoteThread(p_process, NULL, NULL, reinterpret_cast<PTHREAD_START_ROUTINE>(LoadLibraryA), p_allocated_memory, NULL, NULL);

			if (p_thread != nullptr)
				WaitForSingleObject(p_thread, INFINITE);

			VirtualFreeEx(p_process, p_allocated_memory, NULL, MEM_RELEASE);
		}

		CloseHandle(p_process);
	}

	return NULL;
}

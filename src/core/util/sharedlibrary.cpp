/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/core/util/sharedlibrary.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/stringconversion.h>  // splitString
#include <inviwo/core/util/stdextensions.h>
#include <codecvt>
#include <locale>
#include <algorithm>

#if WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace inviwo {

LibrarySearchDirs::LibrarySearchDirs(const std::vector<std::string>& dirs)
    : addedDirs_{util::addLibrarySearchDirs(dirs)} {}

void LibrarySearchDirs::add(const std::vector<std::string>& dirs) {
    util::append(addedDirs_, util::addLibrarySearchDirs(dirs));
}

LibrarySearchDirs::~LibrarySearchDirs() {
    util::removeLibrarySearchDirs(addedDirs_);
}

SharedLibrary::SharedLibrary(const std::string& filePath) : filePath_(filePath) {
#if WIN32
    // Search for dlls in directories specified by the path environment variable
    // Need to be done since we are using a non-standard call to LoadLibrary
    static auto addDirectoriesInPath = []() {  // Lambda executed once
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        const char* environmentPath = std::getenv("PATH");
        if (environmentPath && util::hasAddLibrarySearchDirsFunction()) {
            auto elems = splitString(std::string(environmentPath), ';');
            // Reverse the order since empirically windows looks in the last one first.
            // opposite to the order in the env path.
            // According to the docs the search order for multiple calls to AddDllDirectory
            // is unspecified, so this might be quite fragile.
            std::reverse(elems.begin(), elems.end());
            util::addLibrarySearchDirs(elems);
            return true;
        } else {
            return false;
        }
    }();

    // Load library and search for dependencies in
    // 1. The directory that contains the DLL (LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR).
    // 2. Paths explicitly added with the AddDllDirectory function (LOAD_LIBRARY_SEARCH_USER_DIRS)
    // or the SetDllDirectory function. If more than one path has been added, the order in which the
    // paths are searched is unspecified.
    // 3. The System directory (LOAD_LIBRARY_SEARCH_SYSTEM32)
    //
    // Note 1: The directory containing the application executable is not searched filePath is in
    // that directory. This enables us to use a temporary directory when loading dlls and thereby
    // allow the original ones to be overwritten while the application is running.
    // Note 2: LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR and LOAD_LIBRARY_SEARCH_USER_DIRS requires KB2533623
    // to be installed.
    // Note 3: Not supported on Windows XP and Server 2003
    if (util::hasAddLibrarySearchDirsFunction()) {
        handle_ = LoadLibraryExA(filePath.c_str(), nullptr,
                                 LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32 |
                                     LOAD_LIBRARY_SEARCH_USER_DIRS);
    } else {
        // LOAD_LIBRARY_SEARCH_* flags are not supported
        // Fall back to LoadLibrary
        handle_ = LoadLibraryA(filePath.c_str());
    }

    if (!handle_) {
        auto error = GetLastError();
        std::ostringstream errorStream;
        LPVOID errorText;

        auto outputSize = FormatMessage(
            // use system message tables to retrieve error text
            FORMAT_MESSAGE_FROM_SYSTEM
                // allocate buffer on local heap for error text
                | FORMAT_MESSAGE_ALLOCATE_BUFFER
                // Important! will fail otherwise, since we're not
                // (and CANNOT) pass insertion parameters
                | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorText, 0, NULL);
        if (errorText != nullptr) {
            std::string errorString(static_cast<const char*>(errorText), outputSize + 1);
            errorStream << errorString;
            // release memory allocated by FormatMessage()
            LocalFree(errorText);
        }

        throw Exception("Failed to load library: " + filePath + "\n Error: " + errorStream.str());
    }
#else
    // RTLD_GLOBAL gives all other loaded libraries access to library
    // RTLD_LOCAL is preferred but would require each module to
    // explicitly load its dependent libraries as well.
    handle_ = dlopen(filePath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!handle_) {
        throw Exception("Failed to load library: " + filePath);
    }
#endif
}

SharedLibrary::SharedLibrary(SharedLibrary&& rhs)
    : filePath_{std::move(rhs.filePath_)}, handle_{rhs.handle_} {
    rhs.handle_ = nullptr;
}

SharedLibrary& SharedLibrary::operator=(SharedLibrary&& that) {
    if (this != &that) {
#if WIN32
        FreeLibrary(handle_);
#else
        dlclose(handle_);
#endif
        handle_ = nullptr;
        filePath_ = "";
        std::swap(handle_, that.handle_);
        std::swap(filePath_, that.filePath_);
    }
    return *this;
}

SharedLibrary::~SharedLibrary() {
#if WIN32
    FreeLibrary(handle_);
#else
    dlclose(handle_);
#endif
}

void* SharedLibrary::findSymbol(const std::string& name) {
#if WIN32
    return GetProcAddress(handle_, name.c_str());
#else
    return dlsym(handle_, name.c_str());
#endif
}

std::set<std::string> SharedLibrary::libraryFileExtensions() {
#if WIN32
    return {"dll"};
#else
    return {"so", "dylib", "bundle"};
#endif
}

namespace util {

#if WIN32
bool hasAddLibrarySearchDirsFunction() {
    // Get AddDllDirectory function.
    // This function is only available after installing KB2533623
    using addDllDirectory_func = DLL_DIRECTORY_COOKIE(WINAPI*)(PCWSTR);
    auto lpfnAdllDllDirectory = reinterpret_cast<addDllDirectory_func>(
        GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "AddDllDirectory"));
    return lpfnAdllDllDirectory != nullptr;
}

std::vector<DLL_DIRECTORY_COOKIE> addLibrarySearchDirs(const std::vector<std::string>& dirs) {
    std::vector<DLL_DIRECTORY_COOKIE> addedSearchDirectories;
    // Prevent error mode dialogs from displaying.
    SetErrorMode(SEM_FAILCRITICALERRORS);
    if (hasAddLibrarySearchDirsFunction()) {
        // Add search paths to find module dll dependencies
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        for (auto searchPath : dirs) {
            searchPath = filesystem::cleanupPath(searchPath);
            if (filesystem::directoryExists(searchPath)) {
                replaceInString(searchPath, "/", "\\");
                const auto path = converter.from_bytes(searchPath);
                const auto dlldir = AddDllDirectory(path.c_str());
                if (dlldir) {
                    addedSearchDirectories.emplace_back(dlldir);
                } else {
                    LogWarnCustom("ModuleManager",
                                  "Could not get AddDllDirectory for path " << searchPath);
                }
            }
        }
    }
    return addedSearchDirectories;
}

void removeLibrarySearchDirs(const std::vector<DLL_DIRECTORY_COOKIE>& dirs) {
    if (hasAddLibrarySearchDirsFunction()) {
        for (const auto& dir : dirs) {
            RemoveDllDirectory(dir);
        }
    }
}

#else
// dummy functions
std::vector<void*> addLibrarySearchDirs(const std::vector<std::string>&) { return {}; }
void removeLibrarySearchDirs(const std::vector<void*>&) {}
bool hasAddLibrarySearchDirsFunction() { return true; }
#endif
}  // namespace util

}  // namespace inviwo

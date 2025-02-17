// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "DynamicProfileUtils.h"
#include "VsDevShellGenerator.h"
#include "VsSetupConfiguration.h"

using namespace winrt::Microsoft::Terminal::Settings::Model;

void VsDevShellGenerator::GenerateProfiles(const VsSetupConfiguration::VsSetupInstance& instance, bool hidden, std::vector<winrt::com_ptr<implementation::Profile>>& profiles) const
{
    try
    {
        if (!IsInstanceValid(instance))
        {
            return;
        }

        const auto seed = GetProfileGuidSeed(instance);
        const winrt::guid profileGuid{ ::Microsoft::Console::Utils::CreateV5Uuid(TERMINAL_PROFILE_NAMESPACE_GUID, gsl::as_bytes(gsl::make_span(seed))) };
        auto profile = winrt::make_self<implementation::Profile>(profileGuid);
        profile->Name(winrt::hstring{ GetProfileName(instance) });
        profile->Commandline(winrt::hstring{ GetProfileCommandLine(instance) });
        profile->StartingDirectory(winrt::hstring{ instance.GetInstallationPath() });
        profile->Icon(winrt::hstring{ GetProfileIconPath() });
        profile->Hidden(hidden);
        profiles.emplace_back(std::move(profile));
    }
    CATCH_LOG();
}

std::wstring VsDevShellGenerator::GetProfileName(const VsSetupConfiguration::VsSetupInstance& instance) const
{
    std::wstring name{ L"Developer PowerShell for VS " };
    name.append(instance.GetProfileNameSuffix());
    return name;
}

std::wstring VsDevShellGenerator::GetProfileCommandLine(const VsSetupConfiguration::VsSetupInstance& instance) const
{
    // The triple-quotes are a PowerShell path escape sequence that can safely be stored in a JSON object.
    // The "SkipAutomaticLocation" parameter will prevent "Enter-VsDevShell" from automatically setting the shell path
    // so the path in the profile will be used instead.
    std::wstring commandLine{ L"powershell.exe -NoExit -Command \"& {" };
    commandLine.append(L"Import-Module \"\"\"" + GetDevShellModulePath(instance) + L"\"\"\";");
    commandLine.append(L"Enter-VsDevShell " + instance.GetInstanceId() + L" -SkipAutomaticLocation");
    commandLine.append(L"}\"");

    return commandLine;
}

std::wstring VsDevShellGenerator::GetDevShellModulePath(const VsSetupConfiguration::VsSetupInstance& instance) const
{
    // The path of Microsoft.VisualStudio.DevShell.dll changed in 16.3
    if (instance.VersionInRange(L"[16.3,"))
    {
        return instance.ResolvePath(L"Common7\\Tools\\Microsoft.VisualStudio.DevShell.dll");
    }

    return instance.ResolvePath(L"Common7\\Tools\\vsdevshell\\Microsoft.VisualStudio.DevShell.dll");
}

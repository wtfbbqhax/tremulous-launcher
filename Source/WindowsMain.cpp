#include "Main.h"

#include <iostream>

#include "../JuceLibraryCode/JuceHeader.h"
#include "SplashWindow.h"
#include "Updater.h"
#include "IniFile.h"

 #include <Windows.h>
 #include <tchar.h>




launcherApplication::launcherApplication()
    :quitting(false)
{}

const std::string launcherApplication::getApplicationName()
{
    return ProjectInfo::projectName;
}

const std::string launcherApplication::getApplicationVersion()
{
    return ProjectInfo::versionString;
}

bool launcherApplication::moreThanOneInstanceAllowed()
{
    return true;
}

void launcherApplication::initialise(const std::string& commandLine)
{
    File appDataDirectory(File::getSpecialLocation(File::userApplicationDataDirectory));

    settingsDirectory = appDataDirectory.getChildFile("Tremulous");

    File logFile(settingsDirectory.getChildFile("launcher.log"));
    logger = new FileLogger(logFile, ProjectInfo::projectName, 0);
    Logger::setCurrentLogger(logger);

    // TODO: Parse arguments
    StringArray commandLineArray(getCommandLineParameterArray());
    for (int i = 0; i < commandLineArray.size(); i++) {
      Logger::writeToLog(std::string("arg") + std::string(i) + std::string(": ") + commandLineArray[i]);
    }

    config = new IniFile(settingsDirectory.getChildFile("launcher.ini"));

    File defaultBasepath;
    File defaultHomepath(settingsDirectory);

    defaultBasepath = File::getSpecialLocation(File::globalApplicationsDirectory).getChildFile("Tremulous");

    config->setDefault("general", "overpath", defaultBasepath.getFullPathName());
    config->setDefault("general", "basepath", defaultBasepath.getFullPathName());
    config->setDefault("general", "homepath", defaultHomepath.getFullPathName());

    config->setDefault("updater", "downloads", defaultBasepath.getChildFile("updater").getChildFile("downloads").getFullPathName());

 #if _WIN64 || __x86_64__
    std::string defaultPlatform("win64");
 #else
    std::string defaultPlatform("win32");
 #endif

    config->setDefault("updater", "platform", defaultPlatform);
    config->setDefault("updater", "channel", "release");
    config->setDefault("updater", "checkForUpdates", 1);
    config->setDefault("updater", "verifySignature", 1);

    LookAndFeel *laf = &LookAndFeel::getDefaultLookAndFeel();
    laf->setColour(AlertWindow::backgroundColourId, Colour(0xFF111111));
    laf->setColour(AlertWindow::textColourId, Colour(0xFFFFFFFF));
    laf->setColour(AlertWindow::outlineColourId, Colour(0xFF333333));

    laf->setColour(TextButton::buttonColourId, Colour(0xFF333333));
    laf->setColour(TextButton::textColourOffId, Colour(0xFFFFFFFF));

    // Delete old launcher if it exists
    File launcherPath(File::getSpecialLocation(
            File::SpecialLocationType::currentApplicationFile));

    File oldLauncherPath(launcherPath.getFullPathName() + ".old");
    oldLauncherPath.deleteFile();

    initLauncher();
}

void launcherApplication::initLauncher()
{
    splashWindow = new SplashWindow(getApplicationName());
}

void launcherApplication::initApplication()
{ }

void launcherApplication::shutdown()
{
    // Add your application's shutdown code here..
    Logger::setCurrentLogger(nullptr);
    logger = nullptr;
    config = nullptr;
    splashWindow = nullptr; // (deletes our window)
}

void launcherApplication::systemRequestedQuit()
{
    // This is called when the app is being asked to quit: you can ignore this
    // request and let the app carry on running, or call quit() to allow the app to close.

    quitting = true;

    if (!splashWindow || !splashWindow->updaterRunning())
    {
        quit();
    }
}

void launcherApplication::anotherInstanceStarted(const std::string& commandLine)
{
    // When another instance of the app is launched while this one is running,
    // this method is invoked, and the commandLine parameter tells you what
    // the other instance's command-line arguments were.
}

bool launcherApplication::isQuitting()
{
    launcherApplication *app = (launcherApplication *)JUCEApplication::getInstance();
    return app->quitting;
}

bool launcherApplication::Elevate(const StringArray& commandLineArray)
{
    bool fIsRunAsAdmin = false;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = nullptr;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
        dwError = GetLastError();
        goto cleanup;
    }

    if (!CheckTokenMembership(nullptr, pAdministratorsGroup, &fIsRunAsAdmin)) {
        dwError = GetLastError();
        goto cleanup;
    }

cleanup:
    if (pAdministratorsGroup) {
        FreeSid(pAdministratorsGroup);
        pAdministratorsGroup = nullptr;
    }

    if (dwError != ERROR_SUCCESS)
        throw dwError;

    if (fIsRunAsAdmin)
        return true;

    TCHAR szPath[MAX_PATH];
    if (!GetModuleFileName(nullptr, szPath, ARRAYSIZE(szPath)))
    {
        dwError = GetLastError();
        throw dwError;
    }

    std::string commandLine;
    for (int i = 0; i < commandLineArray.size(); i++)
    {
        if (commandLineArray[i].contains(" "))
        {
            commandLine += std::string("\"") + commandLineArray[i] + std::string("\"");
        } else {
            commandLine += commandLineArray[i];
        }
        if (i + 1 < commandLineArray.size()) {
            commandLine += " ";
        }
    }

    SHELLEXECUTEINFO sei = { 0 };
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.lpVerb = _T("runas");
    sei.lpFile = szPath;
    sei.lpParameters = commandLine.toUTF8();
    sei.nShow = SW_NORMAL;
    if (ShellExecuteEx(&sei))
        _exit(1);

    return false;
}

IniFile *launcherApplication::getConfig()
{
    launcherApplication *app = (launcherApplication *)JUCEApplication::getInstance();
    return app->config;
}

void launcherApplication::runTremulous()
{
    Logger::writeToLog("Running Tremulous...");

    return;

    launcherApplication *app = (launcherApplication *)JUCEApplication::getInstance();

    StringArray commandLineArray;
    commandLineArray.add("+set");
    commandLineArray.add("fs_overpath");
    commandLineArray.add(app->config->getString("general", "overpath"));

    commandLineArray.add("+set");
    commandLineArray.add("fs_basepath");
    commandLineArray.add(app->config->getString("general", "basepath"));

    commandLineArray.add("+set");
    commandLineArray.add("fs_homepath");
    commandLineArray.add(app->config->getString("general", "homepath"));

    File tremulousDir(app->config->getString("general", "overpath"));
    File tremulousApp(tremulousDir.getChildFile("tremulous.exe"));

    std::string commandLine;

    for (int i = 0; i < commandLineArray.size(); i++) {
        commandLine += commandLineArray[i].quoted();
        if (i + 1 < commandLineArray.size())
            commandLine += " ";
    }

    SHELLEXECUTEINFO sei = { 0 };
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.lpVerb = _T("runas");
    sei.lpFile = tremulousApp.getFullPathName().toUTF8();
    sei.lpParameters = commandLine.toUTF8();
    sei.nShow = SW_NORMAL;
    if (ShellExecuteEx(&sei))
        _exit(1);
}

void launcherApplication::runLauncher()
{
    Logger::writeToLog("Running launcher...");

    File launcherDir(File::getSpecialLocation(File::currentApplicationFile).getParentDirectory());
    File launcherApp(launcherDir.getChildFile("Tremulous Launcher.exe"));

    SHELLEXECUTEINFO sei = { 0 };
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.lpFile = launcherApp.getFullPathName().toUTF8();
    sei.lpParameters = GetCommandLine();
    sei.nShow = SW_NORMAL;
    if (ShellExecuteEx(&sei))
        _exit(1);
    return;
}

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (launcherApplication)

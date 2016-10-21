/*
  ==============================================================================

    Updater.h
    Created: 10 Jul 2015 7:07:51pm
    Author:  jkent

  ==============================================================================
*/

#ifndef UPDATER_H_INCLUDED
#define UPDATER_H_INCLUDED

#include <iostream>
#include <vector>

//#include "Component.h"
//#include "Semver.h"

enum UpdaterStatus {
    UPDATER_NOTSTARTED = 0,
    UPDATER_RUNNING,
    UPDATER_FINISHED,
    UPDATER_NEEDRESTART,
    UPDATER_FAILED,
};

class Updater
{
public:
    static const char UPDATER_URL_PREFIX[];

    Updater();
    ~Updater();

    bool checkForUpdates();
    void run(); //override;

    // Status information
    enum UpdaterStatus getStatus();
    bool getStatusUpdate(double& overallPercent, double& taskPercent);
    std::string getStatusText();

    static Updater *getUpdater();
    static bool downloadFile(const std::string& url, const FILE& file, bool resume = 0);

private:
    FILE basepath;
    std::vector<Launcher::Component> components;

    bool getLatestComponents();
    bool updateComponent(Launcher::Component& component);

    static Updater *updater;

    // Status information
    enum UpdaterStatus status;
    //SpinLock statusLock;
    std::string statusString;
    bool statusStringUpdated;
    double taskPercent;
    double overallPercent;

    int totalTasks;
    int completedTasks;

    void setStatusText(const std::string& s);
    void startFirstTask();
    void startNextTask();
    void setTaskPercent(const double& percent);
};

#endif  // UPDATER_H_INCLUDED

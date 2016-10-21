/*
==============================================================================

Component.h
Created: 24 Jul 2015 4:00:00pm
Author:  jkent

==============================================================================
*/

#ifndef COMPONENT_H_INCLUDED
#define COMPONENT_H_INCLUDED

#include "Semver.h"

#include <iostream>
#include <cstdint>

namespace Launcher
{
    class Component
    {
    public:
        Component();
        Component(const Component& other);
        Component(const std::string& name);

        void setRemoteVersion(const Semver& version);
        bool canUpdate();
        std::string& getName();

        bool getRemoteInfo();
        bool alreadyInstalled();
        bool download();
        bool verify();
        bool install();

    private:
        std::string name;
        Semver localVersion;
        Semver remoteVersion;
        std::string remoteUrl;
        int64_t remoteSize;
        std::string remoteHash;
        FILE downloadPath;

        bool installPk3();
        bool installLauncher();
        bool installTremulous();
    };
};

#endif  // COMPONENT_H_INCLUDED

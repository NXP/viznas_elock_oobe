#!/bin/bash
CMD=$1

if [ ${CMD}X == X ]; then
    echo "USAGE:server.sh rm|is|start|stop"
    exit 0
fi

if [ ${CMD}X == "is"X ]; then
    adb install app/build/outputs/apk/debug/app-debug.apk
    #adb shell am start -n  facemanager/.activity.SplashActivity
    exit 0
fi

if [ ${CMD}X == "rm"X ]; then
    adb uninstall com.nxp.facemanager
    exit 0
fi

if [ ${CMD}X == "stop"X ]; then
    adb shell pm clear facemanager
    exit 0
fi

if [ ${CMD}X == "start"X ]; then
    adb shell am start -n  facemanager/.activity.SplashActivity
    exit 0
fi

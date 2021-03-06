/*
 *  The ManaPlus Client
 *  Copyright (C) 2011-2014  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "test/testmain.h"

#ifdef USE_OPENGL

#include "settings.h"

#include "utils/delete2.h"
#include "utils/paths.h"
#include "utils/process.h"

#include <iostream>

#include "debug.h"

std::string fileName;
extern char *selfName;

TestMain::TestMain() :
    log(new Logger),
    mConfig()
{
    fileName = getSelfName();
    log->setLogFile(settings.localDataDir + std::string("/manaplustest.log"));
}

TestMain::~TestMain()
{
    delete2(log);
}

void TestMain::initConfig()
{
    mConfig.init(settings.configDir + "/test.xml");
//    mConfig.setDefaultValues(getConfigDefaults());

    mConfig.setValue("hwaccel", false);
    mConfig.setValue("screen", false);
    mConfig.setValue("sound", false);
    mConfig.setValue("guialpha", 0.8F);
//    mConfig.setValue("remember", true);
    mConfig.setValue("sfxVolume", 50);
    mConfig.setValue("musicVolume", 60);
    mConfig.setValue("fpslimit", 0);
    mConfig.setValue("customcursor", true);
    mConfig.setValue("useScreenshotDirectorySuffix", true);
    mConfig.setValue("ChatLogLength", 128);
    mConfig.setValue("screenwidth", 800);
    mConfig.setValue("screenheight", 600);
}

int TestMain::exec(const bool testAudio)
{
    initConfig();
    int softwareTest = invokeSoftwareRenderTest("1");
    int soundTest = -1;
    int rescaleTest[3];
    int softFps = 0;
    int normalOpenGLFps = 0;
    int safeOpenGLFps = 0;
    int textureSize1 = 1024;
    int textureSize3 = 1024;

    RenderType openGLMode = RENDER_SOFTWARE;
    int detectMode = 0;
    rescaleTest[0] = -1;
    rescaleTest[1] = -1;
    rescaleTest[2] = -1;
    std::string info;

    const int videoDetectTest = invokeTest("99");
    if (!videoDetectTest)
        detectMode = readValue2(99);

    int normalOpenGLTest = invokeNormalOpenGLRenderTest("2");
    int safeOpenGLTest = invokeSafeOpenGLRenderTest("3");
    if (testAudio)
        soundTest = invokeTest4();
    else
        soundTest = 1;

    info.append(strprintf("%d.%d,%d,%d.", soundTest, softwareTest,
        normalOpenGLTest, safeOpenGLTest));

    if (!softwareTest)
    {
        int softFpsTest = invokeSoftwareRenderTest("8");
        info.append(strprintf("%d", softFpsTest));
        if (!softFpsTest)
        {
            softFps = readValue2(8);
            info.append(strprintf(",%d", softFps));
            if (!softFps)
            {
                softwareTest = -1;
                softFpsTest = -1;
            }
            else
            {
                rescaleTest[0] = invokeSoftwareRenderTest("5");
                info.append(strprintf(",%d", rescaleTest[0]));
            }
        }
        else
        {
            softwareTest = -1;
        }
    }
    info.append(".");
    if (!normalOpenGLTest)
    {
        int normalOpenGLFpsTest = invokeNormalOpenGLRenderTest("9");
        info.append(strprintf("%d", normalOpenGLFpsTest));
        if (!normalOpenGLFpsTest)
        {
            normalOpenGLFps = readValue2(9);
            info.append(strprintf(",%d", normalOpenGLFps));
            if (!normalOpenGLFps)
            {
                normalOpenGLTest = -1;
                normalOpenGLFpsTest = -1;
            }
            else
            {
                rescaleTest[1] = invokeNormalOpenGLRenderTest("6");
                info.append(strprintf(",%d", rescaleTest[1]));
            }
        }
        else
        {
            normalOpenGLTest = -1;
        }
    }
    info.append(".");
    if (!safeOpenGLTest)
    {
        int safeOpenGLFpsTest = invokeSafeOpenGLRenderTest("10");
        info.append(strprintf("%d", safeOpenGLFpsTest));
        if (!safeOpenGLFpsTest)
        {
            safeOpenGLFps = readValue2(10);
            info.append(strprintf(",%d", safeOpenGLFps));
            if (!safeOpenGLFps)
            {
                safeOpenGLTest = -1;
                safeOpenGLFpsTest = -1;
            }
            else
            {
                rescaleTest[2] = invokeSafeOpenGLRenderTest("7");
                info.append(strprintf(",%d", rescaleTest[2]));
            }
        }
        else
        {
            safeOpenGLTest = -1;
        }
    }
    info.append(".");

    int maxFps = softFps;
    if (maxFps < normalOpenGLFps)
    {
        openGLMode = RENDER_NORMAL_OPENGL;
        maxFps = normalOpenGLFps;
    }
    if (maxFps < safeOpenGLFps)
    {
        openGLMode = RENDER_SAFE_OPENGL;
        maxFps = safeOpenGLFps;
    }

    int batchSize = 256;

    // if OpenGL mode is normal mode we can try detect max batch sizes
    if (openGLMode == RENDER_NORMAL_OPENGL
        || openGLMode == RENDER_GLES_OPENGL
        || openGLMode == RENDER_MODERN_OPENGL)
    {
        if (!invokeNormalOpenBatchTest("11"))
            batchSize = readValue2(11);
        if (batchSize < 256)
            batchSize = 256;

        if (!invokeNormalOpenBatchTest("14"))
            textureSize1 = readValue2(14);
//        if (!invokeMobileOpenBatchTest("15"))
//            textureSize2 = readValue2(15);
        if (!invokeSafeOpenBatchTest("16"))
            textureSize3 = readValue2(16);
        info.append(strprintf(",%d,%d,-", textureSize1, textureSize3));
        textureSize1 = std::min(textureSize1, textureSize3);
        if (textureSize1 < 1024)
            textureSize1 = 1024;
    }
    else if (openGLMode == RENDER_SAFE_OPENGL)
    {
        if (!invokeSafeOpenBatchTest("16"))
            textureSize3 = readValue2(16);
        textureSize1 = textureSize3;
        if (normalOpenGLTest != -1)
        {
            if (!invokeNormalOpenBatchTest("14"))
                textureSize1 = readValue2(14);
        }
        info.append(strprintf(",%d,%d,-", textureSize1, textureSize3));
        textureSize1 = std::min(textureSize1, textureSize3);
        if (textureSize1 < 1024)
            textureSize1 = 1024;
    }

    // if OpenGL implimentation is not good, disable it.
    if (!(detectMode & 15))
        openGLMode = RENDER_SOFTWARE;

    writeConfig(openGLMode, rescaleTest[static_cast<size_t>(openGLMode)],
        soundTest, info, batchSize, textureSize1, detectMode);
    return 0;
}

void TestMain::writeConfig(const RenderType openGLMode,
                           const int rescale,
                           const int sound,
                           const std::string &info,
                           const int batchSize A_UNUSED,
                           const int textureSize,
                           const int detectMode)
{
    mConfig.init(settings.configDir + "/config.xml");

    log->log("set mode to %d", static_cast<int>(openGLMode));

    // searched values
    mConfig.setValue("opengl", static_cast<int>(openGLMode));
    mConfig.setValue("showBackground", !rescale);
    mConfig.setValue("sound", !sound);

    // better performance
    mConfig.setValue("hwaccel", true);
    mConfig.setValue("fpslimit", 60);
    mConfig.setValue("altfpslimit", 2);
    mConfig.setValue("safemode", false);
    mConfig.setValue("enableMapReduce", true);
    mConfig.setValue("textureSize", textureSize);

    // max batch size
//    mConfig.setValue("batchsize", batchSize);

    // additional modes
    mConfig.setValue("useTextureSampler",
        static_cast<bool>(detectMode & 1024));
    mConfig.setValue("compresstextures",
        static_cast<bool>(detectMode & 2048));

    // stats
    mConfig.setValue("testInfo", info);

    mConfig.write();
}

int TestMain::readValue2(const int ver)
{
    const int def = readValue(ver, 0);
    log->log("value for %d = %d", ver, def);
    return def;
}

int TestMain::readValue(const int ver, int def)
{
    std::string tmp;
    int var;
    std::ifstream file;
    file.open((settings.localDataDir + std::string("/test.log")).c_str(),
        std::ios::in);
    if (!getline(file, tmp))
    {
        file.close();
        return def;
    }
    var = atoi(tmp.c_str());
    if (ver != var || !getline(file, tmp))
    {
        file.close();
        return def;
    }
    def = atoi(tmp.c_str());
    file.close();
    return def;
}

int TestMain::invokeTest(const std::string &test)
{
    mConfig.setValue("opengl", static_cast<int>(RENDER_SOFTWARE));

    mConfig.write();
    const int ret = execFileWait(fileName, fileName, "-t", test);
    return ret;
}

int TestMain::invokeTest4()
{
    mConfig.setValue("sound", true);
    const int ret = invokeTest("4");

    log->log("4: %d", ret);
    return ret;
}

int TestMain::invokeSoftwareRenderTest(const std::string &test)
{
    mConfig.setValue("opengl", static_cast<int>(RENDER_SOFTWARE));
    mConfig.write();
    const int ret = execFileWait(fileName, fileName, "-t", test, 30);
    log->log("%s: %d", test.c_str(), ret);
    return ret;
}

int TestMain::invokeNormalOpenGLRenderTest(const std::string &test)
{
    mConfig.setValue("opengl", static_cast<int>(RENDER_NORMAL_OPENGL));
    mConfig.write();
    const int ret = execFileWait(fileName, fileName, "-t", test, 30);
    log->log("%s: %d", test.c_str(), ret);
    return ret;
}

int TestMain::invokeNormalOpenBatchTest(const std::string &test)
{
    mConfig.setValue("opengl", static_cast<int>(RENDER_NORMAL_OPENGL));
    mConfig.write();
    const int ret = execFileWait(fileName, fileName, "-t", test, 30);
//    log->log("%s: %d", test.c_str(), ret);
    return ret;
}

int TestMain::invokeMobileOpenBatchTest(const std::string &test)
{
    mConfig.setValue("opengl", static_cast<int>(RENDER_GLES_OPENGL));
    mConfig.write();
    const int ret = execFileWait(fileName, fileName, "-t", test, 30);
//    log->log("%s: %d", test.c_str(), ret);
    return ret;
}

int TestMain::invokeSafeOpenBatchTest(const std::string &test)
{
    mConfig.setValue("opengl", static_cast<int>(RENDER_SAFE_OPENGL));
    mConfig.write();
    const int ret = execFileWait(fileName, fileName, "-t", test, 30);
//    log->log("%s: %d", test.c_str(), ret);
    return ret;
}

int TestMain::invokeSafeOpenGLRenderTest(const std::string &test)
{
    mConfig.setValue("opengl", static_cast<int>(RENDER_SAFE_OPENGL));
    mConfig.write();
    const int ret = execFileWait(fileName, fileName, "-t", test, 30);
    log->log("%s: %d", test.c_str(), ret);
    return ret;
}
#endif

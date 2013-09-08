/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2013  The ManaPlus Developers
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

#include "render/graphics.h"

#include "main.h"

#include "configuration.h"
#include "graphicsmanager.h"
#include "graphicsvertexes.h"
#include "logger.h"

#include "resources/imagehelper.h"
#include "resources/openglimagehelper.h"

#include <guichan/sdl/sdlpixel.hpp>

#ifdef USE_OPENGL
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif
#endif

#include "debug.h"

#ifdef USE_OPENGL
#ifndef GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX
#define GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
#endif
#endif

Graphics::Graphics() :
    gcn::Graphics(),
    mWidth(0),
    mHeight(0),
    mWindow(nullptr),
#ifdef USE_SDL2
    mRenderer(nullptr),
#ifdef USE_OPENGL
    mGLContext(nullptr),
#endif
#endif
    mBpp(0),
    mAlpha(false),
    mFullscreen(false),
    mHWAccel(false),
    mRedraw(false),
    mDoubleBuffer(false),
    mRect(),
    mSecure(false),
    mOpenGL(RENDER_SOFTWARE),
    mEnableResize(false),
    mNoFrame(false),
    mName("Unknown"),
    mStartFreeMem(0),
    mSync(false),
    mColor(),
    mColor2()
{
    mRect.x = 0;
    mRect.y = 0;
    mRect.w = 0;
    mRect.h = 0;
}

Graphics::~Graphics()
{
    _endDraw();
#ifdef USE_SDL2
    if (mRenderer)
    {
        SDL_DestroyRenderer(mRenderer);
        mRenderer = nullptr;
    }
#ifdef USE_OPENGL
    if (mGLContext)
    {
        SDL_GL_DeleteContext(mGLContext);
        mGLContext = 0;
    }
#endif
#endif
}

void Graphics::setSync(const bool sync)
{
    mSync = sync;
}

void Graphics::setMainFlags(const int w, const int h, const int bpp,
                            const bool fs, const bool hwaccel,
                            const bool resize, const bool noFrame)
{
    logger->log("graphics backend: %s", getName().c_str());
    logger->log("Setting video mode %dx%d %s",
            w, h, fs ? "fullscreen" : "windowed");

    mWidth = w;
    mHeight = h;
    mBpp = bpp;
    mFullscreen = fs;
    mHWAccel = hwaccel;
    mEnableResize = resize;
    mNoFrame = noFrame;
}

int Graphics::getOpenGLFlags() const
{
#ifdef USE_OPENGL

#ifdef USE_SDL2
    int displayFlags = SDL_WINDOW_OPENGL;
    if (mFullscreen)
        displayFlags |= SDL_WINDOW_FULLSCREEN;
#else
    int displayFlags = SDL_ANYFORMAT | SDL_OPENGL;
#endif  // USE_SDL2

    if (mFullscreen)
    {
        displayFlags |= SDL_FULLSCREEN;
    }
    else
    {
        // Resizing currently not supported on Windows, where it would require
        // reuploading all textures.
#if !defined(_WIN32)
        if (mEnableResize)
            displayFlags |= SDL_RESIZABLE;
#endif
    }

    if (mNoFrame)
        displayFlags |= SDL_NOFRAME;

    return displayFlags;
#else
    return 0;
#endif  // USE_OPENGL
}

bool Graphics::setOpenGLMode()
{
#ifdef USE_OPENGL
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (!(mWindow = graphicsManager.createWindow(mWidth, mHeight,
        mBpp, getOpenGLFlags())))
    {
        mRect.w = 0;
        mRect.h = 0;
        return false;
    }

#ifdef USE_SDL2
    int w1 = 0;
    int h1 = 0;
    SDL_GetWindowSize(mWindow, &w1, &h1);
    mRect.w = w1;
    mRect.h = h1;

    mGLContext = SDL_GL_CreateContext(mWindow);

#else  // USE_SDL2

    mRect.w = static_cast<uint16_t>(mWindow->w);
    mRect.h = static_cast<uint16_t>(mWindow->h);
#endif  // USE_SDL2

#ifdef __APPLE__
    if (mSync)
    {
        const GLint VBL = 1;
        CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &VBL);
    }
#endif

    graphicsManager.setGLVersion();
    graphicsManager.logVersion();

    // Setup OpenGL
    glViewport(0, 0, mWidth, mHeight);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    int gotDoubleBuffer = 0;
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &gotDoubleBuffer);
    logger->log("Using OpenGL %s double buffering.",
                (gotDoubleBuffer ? "with" : "without"));

    graphicsManager.initOpenGL();
    initArrays();
    graphicsManager.updateTextureFormat();
    updateMemoryInfo();

    GLint texSize;
    bool rectTex = graphicsManager.supportExtension(
        "GL_ARB_texture_rectangle");
    if (rectTex && OpenGLImageHelper::getInternalTextureType() == 4
        && getOpenGL() != 3 && config.getBoolValue("rectangulartextures")
        && !graphicsManager.isUseTextureSampler())
    {
        logger->log1("using GL_ARB_texture_rectangle");
        OpenGLImageHelper::mTextureType = GL_TEXTURE_RECTANGLE_ARB;
        glEnable(GL_TEXTURE_RECTANGLE_ARB);
        glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB, &texSize);
        OpenGLImageHelper::mTextureSize = texSize;
        logger->log("OpenGL texture size: %d pixels (rectangle textures)",
            OpenGLImageHelper::mTextureSize);
    }
    else
    {
        OpenGLImageHelper::mTextureType = GL_TEXTURE_2D;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
        OpenGLImageHelper::mTextureSize = texSize;
        logger->log("OpenGL texture size: %d pixels",
            OpenGLImageHelper::mTextureSize);
    }
    return videoInfo();
#else  // USE_OPENGL

    return false;
#endif  // USE_OPENGL
}

int Graphics::getSoftwareFlags() const
{
#ifdef USE_SDL2
    int displayFlags = SDL_WINDOW_SHOWN;
#else
    int displayFlags = SDL_ANYFORMAT;

    if (mHWAccel)
        displayFlags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
    else
        displayFlags |= SDL_SWSURFACE;
#endif

    if (mFullscreen)
        displayFlags |= SDL_FULLSCREEN;
    else if (mEnableResize)
        displayFlags |= SDL_RESIZABLE;

    if (mNoFrame)
        displayFlags |= SDL_NOFRAME;
    return displayFlags;
}


void Graphics::updateMemoryInfo()
{
#ifdef USE_OPENGL
    if (mStartFreeMem)
        return;

    if (graphicsManager.supportExtension("GL_NVX_gpu_memory_info"))
    {
        glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX,
            &mStartFreeMem);
        logger->log("free video memory: %d", mStartFreeMem);
    }
#endif
}

int Graphics::getMemoryUsage() const
{
#ifdef USE_OPENGL
    if (!mStartFreeMem)
        return 0;

    if (graphicsManager.supportExtension("GL_NVX_gpu_memory_info"))
    {
        GLint val;
        glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX,
            &val);
        return mStartFreeMem - val;
    }
#endif
    return 0;
}

#ifdef USE_SDL2
void Graphics::dumpRendererInfo(const char *const str,
                                const SDL_RendererInfo &info)
{
    logger->log(str, info.name);
    logger->log("flags:");
    if (info.flags & SDL_RENDERER_SOFTWARE)
        logger->log(" software");
    if (info.flags & SDL_RENDERER_ACCELERATED)
        logger->log(" accelerated");
    if (info.flags & SDL_RENDERER_PRESENTVSYNC)
        logger->log(" vsync");
    if (info.flags & SDL_RENDERER_TARGETTEXTURE)
        logger->log(" texture target");
}
#endif

bool Graphics::videoInfo()
{
    logger->log("SDL video info");
#ifdef USE_SDL2
    logger->log("Using video driver: %s", SDL_GetCurrentVideoDriver());

    if (mRenderer)
    {
        SDL_RendererInfo info;
        SDL_GetRendererInfo(mRenderer, &info);
        dumpRendererInfo("Current SDL renderer name: %s", info);

        const int num = SDL_GetNumRenderDrivers();
        logger->log("Known renderers");
        for (int f = 0; f < num; f ++)
        {
            if (!SDL_GetRenderDriverInfo(f, &info))
                dumpRendererInfo("renderer name: %s", info);
        }
    }
#else
    char videoDriverName[65];
    if (SDL_VideoDriverName(videoDriverName, 64))
        logger->log("Using video driver: %s", videoDriverName);
    else
        logger->log1("Using video driver: unknown");
    mDoubleBuffer = ((mWindow->flags & SDL_DOUBLEBUF) == SDL_DOUBLEBUF);
    logger->log("Double buffer mode: %s", mDoubleBuffer ? "yes" : "no");

    imageHelper->dumpSurfaceFormat(mWindow);

    const SDL_VideoInfo *const vi = SDL_GetVideoInfo();
    if (!vi)
        return false;

    logger->log("Possible to create hardware surfaces: %s",
            ((vi->hw_available) ? "yes" : "no"));
    logger->log("Window manager available: %s",
            ((vi->wm_available) ? "yes" : "no"));
    logger->log("Accelerated hardware to hardware blits: %s",
            ((vi->blit_hw) ? "yes" : "no"));
    logger->log("Accelerated hardware to hardware colorkey blits: %s",
            ((vi->blit_hw_CC) ? "yes" : "no"));
    logger->log("Accelerated hardware to hardware alpha blits: %s",
            ((vi->blit_hw_A) ? "yes" : "no"));
    logger->log("Accelerated software to hardware blits: %s",
            ((vi->blit_sw) ? "yes" : "no"));
    logger->log("Accelerated software to hardware colorkey blits: %s",
            ((vi->blit_sw_CC) ? "yes" : "no"));
    logger->log("Accelerated software to hardware alpha blits: %s",
            ((vi->blit_sw_A) ? "yes" : "no"));
    logger->log("Accelerated color fills: %s",
            ((vi->blit_fill) ? "yes" : "no"));
#endif

    return true;
}

bool Graphics::setFullscreen(const bool fs)
{
    if (mFullscreen == fs)
        return true;

    return setVideoMode(mWidth, mHeight, mBpp, fs, mHWAccel,
        mEnableResize, mNoFrame);
}

bool Graphics::resizeScreen(const int width, const int height)
{
    if (mWidth == width && mHeight == height)
        return true;

#ifdef USE_SDL2
    _endDraw();

    mRect.w = width;
    mRect.h = height;
    mWidth = width;
    mHeight = height;

#ifdef USE_OPENGL
    // +++ probably this way will not work in windows/mac
    // Setup OpenGL
    glViewport(0, 0, mWidth, mHeight);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
#else
    // +++ need impliment resize in soft mode
#endif  // USE_OPENGL

    _beginDraw();
    return true;

#else
    const int prevWidth = mWidth;
    const int prevHeight = mHeight;

    _endDraw();

    const bool success = setVideoMode(width, height, mBpp,
        mFullscreen, mHWAccel, mEnableResize, mNoFrame);

    // If it didn't work, try to restore the previous size. If that didn't
    // work either, bail out (but then we're in deep trouble).
    if (!success)
    {
        if (!setVideoMode(prevWidth, prevHeight, mBpp,
            mFullscreen, mHWAccel, mEnableResize, mNoFrame))
        {
            return false;
        }
    }

    _beginDraw();

    return success;
#endif  // USE_SDL2
}

int Graphics::getWidth() const
{
    return mWidth;
}

int Graphics::getHeight() const
{
    return mHeight;
}

bool Graphics::drawImage(const Image *image, int x, int y)
{
    if (image)
    {
        return drawImage2(image, 0, 0, x, y,
            image->mBounds.w, image->mBounds.h, false);
    }
    else
    {
        return false;
    }
}

void Graphics::drawImageRect(const int x, const int y,
                             const int w, const int h,
                             const Image *const topLeft,
                             const Image *const topRight,
                             const Image *const bottomLeft,
                             const Image *const bottomRight,
                             const Image *const top,
                             const Image *const right,
                             const Image *const bottom,
                             const Image *const left,
                             const Image *const center)
{
    BLOCK_START("Graphics::drawImageRect")
    const bool drawMain = center && topLeft && topRight
        && bottomLeft && bottomRight;

    // Draw the center area
    if (center && drawMain)
    {
        const int tlw = topLeft->getWidth();
        const int tlh = topLeft->getHeight();
        drawImagePattern(center, tlw + x, tlh + y,
            w - tlw - topRight->getWidth(),
            h - tlh - bottomLeft->getHeight());
    }

    // Draw the sides
    if (top && left && bottom && right)
    {
        const int lw = left->getWidth();
        const int rw = right->getWidth();
        const int th = top->getHeight();
        const int bh = bottom->getHeight();
        drawImagePattern(top, x + lw, y, w - lw - rw, th);
        drawImagePattern(bottom, x + lw, h - bh + y, w - lw - rw, bh);
        drawImagePattern(left, x, y + th, lw, h - th - bh);
        drawImagePattern(right, x + w - rw, th + y, rw, h - th - bh);
    }
    // Draw the corners
    if (drawMain)
    {
        drawImage(topLeft, x, y);
        drawImage(topRight, x + w - topRight->getWidth(), y);
        drawImage(bottomLeft, x, h - bottomLeft->getHeight() + y);
        drawImage(bottomRight,
            x + w - bottomRight->getWidth(),
            y + h - bottomRight->getHeight());
    }
    BLOCK_END("Graphics::drawImageRect")
}

void Graphics::drawImageRect(int x, int y, int w, int h,
                             const ImageRect &imgRect)
{
    drawImageRect(x, y, w, h,
            imgRect.grid[0], imgRect.grid[2], imgRect.grid[6], imgRect.grid[8],
            imgRect.grid[1], imgRect.grid[5], imgRect.grid[7], imgRect.grid[3],
            imgRect.grid[4]);
}

bool Graphics::drawNet(const int x1, const int y1, const int x2, const int y2,
                       const int width, const int height)
{
    for (int y = y1; y < y2; y += height)
        drawLine(x1, y, x2, y);

    for (int x = x1; x < x2; x += width)
        drawLine(x, y1, x, y2);

    return true;
}

bool Graphics::calcImageRect(ImageVertexes *const vert,
                             const int x, const int y,
                             const int w, const int h,
                             const Image *const topLeft,
                             const Image *const topRight,
                             const Image *const bottomLeft,
                             const Image *const bottomRight,
                             const Image *const top,
                             const Image *const right,
                             const Image *const bottom,
                             const Image *const left,
                             const Image *const center)
{
    if (!vert)
        return false;

    BLOCK_START("Graphics::calcImageRect")
    const bool drawMain = center && topLeft && topRight
        && bottomLeft && bottomRight;

//    pushClipArea(gcn::Rectangle(x, y, w, h));

    // Draw the center area
    if (center && drawMain)
    {
        const int tlw = topLeft->getWidth();
        const int tlh = topLeft->getHeight();
        calcImagePattern(vert, center, tlw + x, tlh + y,
            w - tlw - topRight->getWidth(),
            h - tlh - bottomLeft->getHeight());
    }
    // Draw the sides
    if (top && left && bottom && right)
    {
        const int lw = left->getWidth();
        const int rw = right->getWidth();
        const int th = top->getHeight();
        const int bh = bottom->getHeight();
        calcImagePattern(vert, top, x + lw, y, w - lw - rw, th);
        calcImagePattern(vert, bottom, x + lw, y + h - bh, w - lw - rw, bh);
        calcImagePattern(vert, left, x, y + th, lw, h - th - bh);
        calcImagePattern(vert, right, x + w - rw, y + th, rw, h - th - bh);
    }

    calcTile(vert, topLeft, x, y);
    if (topRight)
        calcTile(vert, topRight, x + w - topRight->getWidth(), y);
    if (bottomLeft)
        calcTile(vert, bottomLeft, x, y + h - bottomLeft->getHeight());
    if (bottomRight)
    {
        calcTile(vert, bottomRight, x + w - bottomRight->getWidth(),
            y + h - bottomRight->getHeight());
    }

//    popClipArea();
    BLOCK_END("Graphics::calcImageRect")
    return 0;
}

void Graphics::setWindowSize(const int width, const int height)
{
#ifdef USE_SDL2
    SDL_SetWindowSize(mWindow, width, height);
#endif
}
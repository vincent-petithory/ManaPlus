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

#include "main.h"

#if defined USE_OPENGL && !defined ANDROID
#include "render/safeopenglgraphics.h"

#include "configuration.h"
#include "graphicsmanager.h"
#include "graphicsvertexes.h"
#include "logger.h"

#include "resources/image.h"
#include "resources/openglimagehelper.h"

#include "utils/sdlcheckutils.h"

#include <SDL.h>

#include "debug.h"

GLuint SafeOpenGLGraphics::mLastImage = 0;

SafeOpenGLGraphics::SafeOpenGLGraphics():
    mTexture(false),
    mIsByteColor(false),
    mByteColor(),
    mFloatColor(1.0f),
    mColorAlpha(false),
    mFbo()
{
    mOpenGL = RENDER_SAFE_OPENGL;
    mName = "safe OpenGL";
}

SafeOpenGLGraphics::~SafeOpenGLGraphics()
{
}

bool SafeOpenGLGraphics::setVideoMode(const int w, const int h, const int bpp,
                                      const bool fs, const bool hwaccel,
                                      const bool resize, const bool noFrame)
{
    setMainFlags(w, h, bpp, fs, hwaccel, resize, noFrame);

    return setOpenGLMode();
}

static inline void drawQuad(const Image *image,
                            const int srcX, const int srcY,
                            const int dstX, const int dstY,
                            const int width, const int height)
{
    if (OpenGLImageHelper::mTextureType == GL_TEXTURE_2D)
    {
        const float tw = static_cast<float>(image->mTexWidth);
        const float th = static_cast<float>(image->mTexHeight);
        // Find OpenGL normalized texture coordinates.
        const float texX1 = static_cast<float>(srcX) / tw;
        const float texY1 = static_cast<float>(srcY) / th;
        const float texX2 = static_cast<float>(srcX + width) / tw;
        const float texY2 = static_cast<float>(srcY + height) / th;

        glTexCoord2f(texX1, texY1);
        glVertex2i(dstX, dstY);
        glTexCoord2f(texX2, texY1);
        glVertex2i(dstX + width, dstY);
        glTexCoord2f(texX2, texY2);
        glVertex2i(dstX + width, dstY + height);
        glTexCoord2f(texX1, texY2);
        glVertex2i(dstX, dstY + height);
    }
    else
    {
        glTexCoord2i(srcX, srcY);
        glVertex2i(dstX, dstY);
        glTexCoord2i(srcX + width, srcY);
        glVertex2i(dstX + width, dstY);
        glTexCoord2i(srcX + width, srcY + height);
        glVertex2i(dstX + width, dstY + height);
        glTexCoord2i(srcX, srcY + height);
        glVertex2i(dstX, dstY + height);
    }
}

static inline void drawRescaledQuad(const Image *const image,
                                    const int srcX, const int srcY,
                                    const int dstX, const int dstY,
                                    const int width, const int height,
                                    const int desiredWidth,
                                    const int desiredHeight)
{
    if (OpenGLImageHelper::mTextureType == GL_TEXTURE_2D)
    {
        const float tw = static_cast<float>(image->mTexWidth);
        const float th = static_cast<float>(image->mTexHeight);
        // Find OpenGL normalized texture coordinates.
        const float texX1 = static_cast<float>(srcX) / tw;
        const float texY1 = static_cast<float>(srcY) / th;
        const float texX2 = static_cast<float>(srcX + width) / tw;
        const float texY2 = static_cast<float>(srcY + height) / th;

        glTexCoord2f(texX1, texY1);
        glVertex2i(dstX, dstY);
        glTexCoord2f(texX2, texY1);
        glVertex2i(dstX + desiredWidth, dstY);
        glTexCoord2f(texX2, texY2);
        glVertex2i(dstX + desiredWidth, dstY + desiredHeight);
        glTexCoord2f(texX1, texY2);
        glVertex2i(dstX, dstY + desiredHeight);
    }
    else
    {
        glTexCoord2i(srcX, srcY);
        glVertex2i(dstX, dstY);
        glTexCoord2i(srcX + width, srcY);
        glVertex2i(dstX + desiredWidth, dstY);
        glTexCoord2i(srcX + width, srcY + height);
        glVertex2i(dstX + desiredWidth, dstY + desiredHeight);
        glTexCoord2i(srcX, srcY + height);
        glVertex2i(dstX, dstY + desiredHeight);
    }
}


bool SafeOpenGLGraphics::drawImage2(const Image *const image,
                                    int srcX, int srcY,
                                    int dstX, int dstY,
                                    const int width, const int height,
                                    const bool useColor)
{
    FUNC_BLOCK("Graphics::drawImage2", 1)
    if (!image)
        return false;

    if (!useColor)
        setColorAlpha(image->mAlpha);

    bindTexture(OpenGLImageHelper::mTextureType, image->mGLImage);

    setTexturingAndBlending(true);

    // Draw a textured quad.
    glBegin(GL_QUADS);
    drawQuad(image, srcX + image->mBounds.x, srcY + image->mBounds.y,
        dstX, dstY, width, height);
    glEnd();

    return true;
}

bool SafeOpenGLGraphics::drawRescaledImage(const Image *const image, int srcX,
                                           int srcY, int dstX, int dstY,
                                           const int width, const int height,
                                           const int desiredWidth,
                                           const int desiredHeight,
                                           const bool useColor)
{
    return drawRescaledImage(image, srcX, srcY,
                             dstX, dstY,
                             width, height,
                             desiredWidth, desiredHeight,
                             useColor, true);
}

bool SafeOpenGLGraphics::drawRescaledImage(const Image *const image, int srcX,
                                           int srcY, int dstX, int dstY,
                                           const int width, const int height,
                                           const int desiredWidth,
                                           const int desiredHeight,
                                           const bool useColor,
                                           bool smooth)
{
    FUNC_BLOCK("Graphics::drawRescaledImage", 1)
    if (!image)
        return false;

    // Just draw the image normally when no resizing is necessary,
    if (width == desiredWidth && height == desiredHeight)
    {
        return drawImage2(image, srcX, srcY, dstX, dstY,
                          width, height, useColor);
    }

    // When the desired image is smaller than the current one,
    // disable smooth effect.
    if (width > desiredWidth && height > desiredHeight)
        smooth = false;

    srcX += image->mBounds.x;
    srcY += image->mBounds.y;

    if (!useColor)
        setColorAlpha(image->mAlpha);

    bindTexture(OpenGLImageHelper::mTextureType, image->mGLImage);

    setTexturingAndBlending(true);

    // Draw a textured quad.
    glBegin(GL_QUADS);
    drawRescaledQuad(image, srcX, srcY, dstX, dstY, width, height,
                     desiredWidth, desiredHeight);

    if (smooth)  // A basic smooth effect...
    {
        setColorAlpha(0.2f);
        drawRescaledQuad(image, srcX, srcY, dstX - 1, dstY - 1, width, height,
                        desiredWidth + 1, desiredHeight + 1);
        drawRescaledQuad(image, srcX, srcY, dstX + 1, dstY + 1, width, height,
                        desiredWidth - 1, desiredHeight - 1);

        drawRescaledQuad(image, srcX, srcY, dstX + 1, dstY, width, height,
                        desiredWidth - 1, desiredHeight);
        drawRescaledQuad(image, srcX, srcY, dstX, dstY + 1, width, height,
                        desiredWidth, desiredHeight - 1);
    }

    glEnd();

    return true;
}

/* Optimising the functions that Graphics::drawImagePattern would call,
 * so that glBegin...glEnd are outside the main loop. */
void SafeOpenGLGraphics::drawImagePattern(const Image *const image,
                                          const int x, const int y,
                                          const int w, const int h)
{
    FUNC_BLOCK("Graphics::drawImagePattern", 1)
    if (!image)
        return;

    const SDL_Rect &imageRect = image->mBounds;
    const int iw = imageRect.w;
    const int ih = imageRect.h;
    if (iw == 0 || ih == 0)
        return;

    const int srcX = imageRect.x;
    const int srcY = imageRect.y;

    setColorAlpha(image->mAlpha);
    bindTexture(OpenGLImageHelper::mTextureType, image->mGLImage);
    setTexturingAndBlending(true);

    // Draw a set of textured rectangles
    glBegin(GL_QUADS);

    for (int py = 0; py < h; py += ih)
    {
        const int height = (py + ih >= h) ? h - py : ih;
        const int dstY = y + py;
        for (int px = 0; px < w; px += iw)
        {
            int width = (px + iw >= w) ? w - px : iw;
            int dstX = x + px;
            drawQuad(image, srcX, srcY, dstX, dstY, width, height);
        }
    }

    glEnd();
}

void SafeOpenGLGraphics::drawRescaledImagePattern(const Image *const image,
                                                  const int x, const int y,
                                                  const int w, const int h,
                                                  const int scaledWidth,
                                                  const int scaledHeight)
{
    if (!image)
        return;

    const int iw = scaledWidth;
    const int ih = scaledHeight;
    if (iw == 0 || ih == 0)
        return;

    const SDL_Rect &imageRect = image->mBounds;
    const int srcX = imageRect.x;
    const int srcY = imageRect.y;

    setColorAlpha(image->mAlpha);
    bindTexture(OpenGLImageHelper::mTextureType, image->mGLImage);
    setTexturingAndBlending(true);

    // Draw a set of textured rectangles
    glBegin(GL_QUADS);

    const float scaleFactorW = static_cast<float>(scaledWidth)
        / image->getWidth();
    const float scaleFactorH = static_cast<float>(scaledHeight)
        / image->getHeight();

    for (int py = 0; py < h; py += ih)
    {
        const int height = (py + ih >= h) ? h - py : ih;
        const int dstY = y + py;
        for (int px = 0; px < w; px += iw)
        {
            int width = (px + iw >= w) ? w - px : iw;
            int dstX = x + px;

            drawRescaledQuad(image, srcX, srcY, dstX, dstY,
                width / scaleFactorW, height / scaleFactorH,
                scaledWidth, scaledHeight);
        }
    }

    glEnd();
}

void SafeOpenGLGraphics::calcTile(ImageCollection *const vertCol A_UNUSED,
                                  const Image *const image A_UNUSED,
                                  int x A_UNUSED, int y A_UNUSED)
{
}

void SafeOpenGLGraphics::calcTile(ImageVertexes *const vert A_UNUSED,
                                  const Image *const image A_UNUSED,
                                  int x A_UNUSED, int y A_UNUSED) const
{
}

void SafeOpenGLGraphics::calcImagePattern(ImageVertexes *const vert A_UNUSED,
                                          const Image *const image A_UNUSED,
                                          const int x A_UNUSED,
                                          const int y A_UNUSED,
                                          const int w A_UNUSED,
                                          const int h A_UNUSED) const
{
}

void SafeOpenGLGraphics::calcImagePattern(ImageCollection *const vert A_UNUSED,
                                          const Image *const image A_UNUSED,
                                          const int x A_UNUSED,
                                          const int y A_UNUSED,
                                          const int w A_UNUSED,
                                          const int h A_UNUSED) const
{
}

void SafeOpenGLGraphics::drawTile(const ImageVertexes *const vert A_UNUSED)
{
}

void SafeOpenGLGraphics::drawTile(const ImageCollection *const
                                  vertCol A_UNUSED)
{
}

void SafeOpenGLGraphics::updateScreen()
{
    BLOCK_START("Graphics::updateScreen")
    glFlush();
    glFinish();
#ifdef USE_SDL2
    SDL_GL_SwapWindow(mWindow);
#else
    SDL_GL_SwapBuffers();
#endif
    BLOCK_END("Graphics::updateScreen")
}

bool SafeOpenGLGraphics::calcWindow(ImageCollection *const vertCol A_UNUSED,
                                    const int x A_UNUSED, const int y A_UNUSED,
                                    const int w A_UNUSED, const int h A_UNUSED,
                                    const ImageRect &imgRect A_UNUSED)
{
    return false;
}

void SafeOpenGLGraphics::_beginDraw()
{
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0.0, static_cast<double>(mRect.w),
        static_cast<double>(mRect.h), 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_DITHER);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glDisable(GL_COLOR_LOGIC_OP);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_STENCIL_TEST);

    glShadeModel(GL_FLAT);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    pushClipArea(gcn::Rectangle(0, 0, mRect.w, mRect.h));
}

void SafeOpenGLGraphics::_endDraw()
{
    popClipArea();
}

void SafeOpenGLGraphics::prepareScreenshot()
{
    if (config.getBoolValue("usefbo"))
        graphicsManager.createFBO(mRect.w, mRect.h, &mFbo);
}

SDL_Surface* SafeOpenGLGraphics::getScreenshot()
{
    const int h = mRect.h;
    const int w = mRect.w - (mRect.w % 4);
    GLint pack = 1;

    SDL_Surface *const screenshot = MSDL_CreateRGBSurface(
        SDL_SWSURFACE, w, h, 24, 0xff0000, 0x00ff00, 0x0000ff, 0x000000);

    if (!screenshot)
        return nullptr;

    if (SDL_MUSTLOCK(screenshot))
        SDL_LockSurface(screenshot);

    // Grap the pixel buffer and write it to the SDL surface
    glGetIntegerv(GL_PACK_ALIGNMENT, &pack);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, screenshot->pixels);

    // Flip the screenshot, as OpenGL has 0,0 in bottom left
    unsigned int lineSize = 3 * w;
    GLubyte* buf = static_cast<GLubyte*>(malloc(lineSize));

    const int h2 = h / 2;
    for (int i = 0; i < h2; i++)
    {
        GLubyte *const top = static_cast<GLubyte*>(
            screenshot->pixels) + lineSize * i;
        GLubyte *const bot = static_cast<GLubyte*>(
            screenshot->pixels) + lineSize * (h - 1 - i);

        memcpy(buf, top, lineSize);
        memcpy(top, bot, lineSize);
        memcpy(bot, buf, lineSize);
    }

    free(buf);

    if (config.getBoolValue("usefbo"))
        graphicsManager.deleteFBO(&mFbo);

    glPixelStorei(GL_PACK_ALIGNMENT, pack);

    if (SDL_MUSTLOCK(screenshot))
        SDL_UnlockSurface(screenshot);

    return screenshot;
}

bool SafeOpenGLGraphics::pushClipArea(gcn::Rectangle area)
{
    int transX = 0;
    int transY = 0;

    if (!mClipStack.empty())
    {
        const gcn::ClipRectangle &clipArea = mClipStack.top();
        transX = -clipArea.xOffset;
        transY = -clipArea.yOffset;
    }

    const bool result = gcn::Graphics::pushClipArea(area);

    const gcn::ClipRectangle &clipArea = mClipStack.top();

    glPushMatrix();
    glTranslatef(static_cast<GLfloat>(transX + clipArea.xOffset),
                 static_cast<GLfloat>(transY + clipArea.yOffset), 0);
    glScissor(clipArea.x, mRect.h - clipArea.y - clipArea.height,
        clipArea.width, clipArea.height);

    return result;
}

void SafeOpenGLGraphics::popClipArea()
{
    gcn::Graphics::popClipArea();

    if (mClipStack.empty())
        return;

    glPopMatrix();
    const gcn::ClipRectangle &clipArea = mClipStack.top();
    glScissor(clipArea.x, mRect.h - clipArea.y - clipArea.height,
        clipArea.width, clipArea.height);
}

void SafeOpenGLGraphics::drawPoint(int x, int y)
{
    setTexturingAndBlending(false);
    restoreColor();

    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

void SafeOpenGLGraphics::drawLine(int x1, int y1, int x2, int y2)
{
    setTexturingAndBlending(false);
    restoreColor();

    glBegin(GL_LINES);
    glVertex2f(static_cast<float>(x1) + 0.5f, static_cast<float>(y1) + 0.5f);
    glVertex2f(static_cast<float>(x2) + 0.5f, static_cast<float>(y2) + 0.5f);
    glEnd();

    glBegin(GL_POINTS);
    glVertex2f(static_cast<float>(x2) + 0.5f, static_cast<float>(y2) + 0.5f);
    glEnd();
}

void SafeOpenGLGraphics::drawRectangle(const gcn::Rectangle& rect)
{
    drawRectangle(rect, false);
}

void SafeOpenGLGraphics::fillRectangle(const gcn::Rectangle& rect)
{
    drawRectangle(rect, true);
}

void SafeOpenGLGraphics::setTargetPlane(int width A_UNUSED,
                                        int height A_UNUSED)
{
}

void SafeOpenGLGraphics::setTexturingAndBlending(const bool enable)
{
    if (enable)
    {
        if (!mTexture)
        {
            glEnable(OpenGLImageHelper::mTextureType);
            mTexture = true;
        }

        if (!mAlpha)
        {
            glEnable(GL_BLEND);
            mAlpha = true;
        }
    }
    else
    {
        mLastImage = 0;
        if (mAlpha && !mColorAlpha)
        {
            glDisable(GL_BLEND);
            mAlpha = false;
        }
        else if (!mAlpha && mColorAlpha)
        {
            glEnable(GL_BLEND);
            mAlpha = true;
        }

        if (mTexture)
        {
            glDisable(OpenGLImageHelper::mTextureType);
            mTexture = false;
        }
    }
}

void SafeOpenGLGraphics::drawRectangle(const gcn::Rectangle& rect,
                                       const bool filled)
{
    BLOCK_START("Graphics::drawRectangle")
    const float offset = filled ? 0 : 0.5f;

    setTexturingAndBlending(false);
    restoreColor();

    glBegin(filled ? GL_QUADS : GL_LINE_LOOP);
    glVertex2f(static_cast<float>(rect.x) + offset,
        static_cast<float>(rect.y) + offset);
    glVertex2f(static_cast<float>(rect.x + rect.width) - offset,
        static_cast<float>(rect.y) + offset);
    glVertex2f(static_cast<float>(rect.x + rect.width) - offset,
        static_cast<float>(rect.y + rect.height) - offset);
    glVertex2f(static_cast<float>(rect.x) + offset,
        static_cast<float>(rect.y + rect.height) - offset);
    glEnd();
    BLOCK_END("Graphics::drawRectangle")
}

void SafeOpenGLGraphics::bindTexture(const GLenum target, const GLuint texture)
{
    if (mLastImage != texture)
    {
        mLastImage = texture;
        glBindTexture(target, texture);
    }
}

void SafeOpenGLGraphics::setColorAlpha(const float alpha)
{
    if (!mIsByteColor && mFloatColor == alpha)
        return;

    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    mIsByteColor = false;
    mFloatColor = alpha;
}

void SafeOpenGLGraphics::restoreColor()
{
    if (mIsByteColor && mByteColor == mColor)
        return;

    glColor4ub(static_cast<GLubyte>(mColor.r),
               static_cast<GLubyte>(mColor.g),
               static_cast<GLubyte>(mColor.b),
               static_cast<GLubyte>(mColor.a));
    mIsByteColor = true;
    mByteColor = mColor;
}

#endif  // USE_OPENGL
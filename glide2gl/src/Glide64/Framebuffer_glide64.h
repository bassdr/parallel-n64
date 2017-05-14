/*
* Glide64 - Glide video plugin for Nintendo 64 emulators.
* Copyright (c) 2002  Dave2001
* Copyright (c) 2003-2009  Sergey 'Gonetz' Lipski
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

//****************************************************************
//
// Glide64 - Glide Plugin for Nintendo 64 emulators
// Project started on December 29th, 2001
//
// Authors:
// Dave2001, original author, founded the project in 2001, left it in 2002
// Gugaman, joined the project in 2002, left it in 2002
// Sergey 'Gonetz' Lipski, joined the project in 2002, main author since fall of 2002
// Hiroshi 'KoolSmoky' Morii, joined the project in 2007
//
//****************************************************************
//
// To modify Glide64:
// * Write your name and (optional)email, commented by your work, so I know who did it, and so that you can find which parts you modified when it comes time to send it to me.
// * Do NOT send me the whole project or file that you modified.  Take out your modified code sections, and tell me where to put them.  If people sent the whole thing, I would have many different versions, but no idea how to combine them all.
//
//****************************************************************
//
// Render N64 frame buffer to screen
// Created by Gonetz, 2007
//
//****************************************************************
#ifndef FRAMEBUFFER_GLIDE64_H
#define FRAMEBUFFER_GLIDE64_H

#include "Gfx_1.3.h"

#include "../Glitch64/glide.h"

struct vertexi
{
   int x,y;       // Screen position in 16:16 bit fixed point
   int z;         // z value in 16:16 bit fixed point
};

typedef struct 
{
  uint32_t addr;   //color image address
  uint32_t size;   
  uint32_t width;
  uint32_t height;
  uint32_t ul_x;
  uint32_t ul_y;
  uint32_t lr_x;
  uint32_t lr_y;
  uint32_t opaque;
} FB_TO_SCREEN_INFO;

typedef struct
{
   uint32_t d_ul_x;
   uint32_t d_ul_y;
   uint32_t d_lr_x;
   uint32_t d_lr_y;
} part_framebuffer;

extern part_framebuffer part_framebuf;

void ZLUT_init(void);
void ZLUT_release(void);

void DrawDepthBufferFog(void);
void DrawDepthBuffer(VERTEX * vtx, int n);

void copyWhiteToRDRAM(void);

void DrawPartFrameBufferToScreen(void);
void DrawWholeFrameBufferToScreen(void);
void CopyFrameBuffer (int32_t buffer);

void drawViRegBG(void);

namespace Glide64
{
extern uint16_t *frameBuffer;
}

#endif  // #ifndef FBtoSCREEN_H

/*****************************************************************************
 * sdlyuvaddon.h : RGB to YUV conversion functions
 * 
 *****************************************************************************
 * Copyright (C) 2001 Xavier Servettaz <xavier@zoy.org>
 *
 * Authors: Xavier Servettaz <xavier@zoy.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

/** @author Xavier Servettaz
   RGB to YUV conversion functions
   */

#ifndef SDLYUVADDON_H
#define SDLYUVADDON_H

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_endian.h>

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
 * Type definitions
 * *******************************************************************/
	/**
	 YUV Surface type
	 */
typedef struct SDL_YUVSurface{
	int w, h;		/* Read-only */
	Uint16 pitch;		/* Read-only */
	Uint8 * y_data;		/* Read-write */
	Uint8 * u_data;		/* Read-write */
	Uint8 * v_data;		/* Read-write */
} SDL_YUVSurface;



/*****************************************************************************
 * Prototypes
 *****************************************************************************/

/** Create an empty YUV Surface
 
  @param width surface width in pixel
  @param height surface height in pixel
  @return pointer to newly allocated SDL_YUVSurface
  @author Xavier Servettaz
  @version 0.1
  @see SDL_CreateSurface from http://www.libsdl.org
 */
SDL_YUVSurface *SDL_CreateYUVSurface 	(int width, int height);

/** Free yuv surface
  @param surface to be freed
  @return nothing
  @author Xavier Servettaz
  @version 0.1
  @see SDL_FreeSurface from http://www.libsdl.org

 */
void 		SDL_FreeYUVSurface 	(SDL_YUVSurface *surface);

/** Blit yuv surface to an overlay
  @param src pointer to the YUV surface to copy
  @param srcrect pointer to the source rect
  @param dst pointer to destination overlay
  @param dstrect pointer to destination rect
  @return int (0 if ok -1 if error)
  @author Xavier Servettaz
  @version 0.1

 */
int 		SDL_BlitOverlay		(SDL_YUVSurface *src, SDL_Rect *srcrect, SDL_Overlay *dst, SDL_Rect *dstrect);

/** Get YUV components from an RGB Surface
  @param image pointer to the RGB source surface
  @param yuv_surface pointer to the destination yuv surface
  @return int (0 if ok -1 if error)
  @author Xavier Servettaz
  @version 0.1

 */
int 		Get_YUV_From_Surface	(SDL_Surface * image,SDL_YUVSurface * yuv_surface);

/** May be broken convert an RGB surface to an YUV surface
  @param image pointer to the RGB source surface
  @param yuv_surface pointer to the destination yuv surface
  @return int (0 if ok -1 if error)
  @author Xavier Servettaz
  @version 0.1

 */
int ConvertRGBtoYUV ( SDL_Surface *image, SDL_YUVSurface * yuv_surface);

#ifdef __cplusplus
}
#endif


#endif

/*****************************************************************************
 * sdlyuvaddon.c : RGB to YUV conversion functions
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "yuv.h"



/*
 * Create an empty YUV surface of the appropriate depth
 */
SDL_YUVSurface * SDL_CreateYUVSurface (int width, int height)
{
	/*****************************************************
	 * FIXME only YUV 4.2.0 surfs
	 * ***************************************************/
	SDL_YUVSurface *surface;
	Uint32 size;
	Uint32 uv_size;

	

	/* Allocate the surface */
	surface = (SDL_YUVSurface *)malloc(sizeof(*surface));
	if ( surface == NULL ) {
		SDL_OutOfMemory();
		return(NULL);
	}
	
	
	surface->w = width;
	surface->h = height;
	surface->pitch = width; //SDL_CalculatePitch(surface);
	surface->y_data = NULL;
	surface->u_data = NULL;
	surface->v_data = NULL;

	size = surface->h*surface->pitch;
	uv_size = (surface->w /2 +1)*(surface->h/2 +1);//FIXME only 4.2.0
	//fprintf(stderr," Ysize = %d UVsize =  %d\n",size,uv_size);

	/* Get the yuv datas */
	if ( surface->w && surface->h ) {
		/* Y data */
		//fprintf(stderr, "ALLOCATING Y: size = %d\n",size);

		surface->y_data = (Uint8 *) malloc(size);
		if ( surface->y_data == NULL ) {
			SDL_FreeYUVSurface(surface);
			SDL_OutOfMemory();
			return(NULL);
		}
		/* U data */
		//fprintf(stderr, "ALLOCATING U: size = %d\n",uv_size);
		surface->u_data = (Uint8 *)malloc(uv_size); 
		if ( surface->u_data == NULL ) {
			SDL_FreeYUVSurface(surface);
			SDL_OutOfMemory();
			return(NULL);
		}
		/* V data */
		//fprintf(stderr, "ALLOCATING V: size = %d\n",uv_size);
		surface->v_data = (Uint8 *)malloc(uv_size);

		if ( surface->v_data == NULL ) {
			SDL_FreeYUVSurface(surface);
			SDL_OutOfMemory();
			return(NULL);
		}

		/* This is important for bitmaps */
		memset(surface->y_data, 0, size);
		memset(surface->u_data, 0, uv_size);
		memset(surface->v_data, 0, uv_size);
	}

	//fprintf(stderr, "YUV SURFACE ALLOCATED\n");

	//fprintf(stderr," y_data = %x\n",surface->y_data);
	//fprintf(stderr," u_data = %x\n",surface->u_data);
	//fprintf(stderr," v_data = %x\n",surface->v_data);

	return(surface);
}


/*
 * Free a surface created by the above function.
 */
void SDL_FreeYUVSurface (SDL_YUVSurface *surface)
{
	/* Free anything that's not NULL */
	if (surface == NULL)  {
		return;
	}

	free(surface->y_data);
	free(surface->u_data);
	free(surface->v_data);

	free(surface);

}

/***************************************************************************/

int Get_YUV_From_Surface(SDL_Surface * image,SDL_YUVSurface * yuv_surface){
  
  Uint32 x;
  Uint32 y;
  Uint32 offset;
  Uint32 offset_uv;
  Uint32 convert_start;
  Uint32 convert_end;

  Uint8* p_y;
  Uint8* p_u;
  Uint8* p_v;

  Uint32 height;
  Uint32 width;
	 
  height = image->h;
  width = image->w;	  
  
  offset = 0;
  offset_uv = 0;
  convert_start = 0;
  convert_end = 0;

  p_y = yuv_surface->y_data;
  p_u = yuv_surface->u_data;
  p_v = yuv_surface->v_data;

  //fprintf(stderr," p_y = %x\n",p_y);
  //fprintf(stderr," p_u = %x\n",p_u);
  //fprintf(stderr," p_v = %x\n",p_v);
  
	  
  
  if (p_y == NULL || p_u == NULL || p_v == NULL){
    //fprintf(stderr,"YUV : Out of memory\n");
    return (-1);
  } 

  
/*mesure speed*/
  convert_start =  SDL_GetTicks();
 
  for (y = 0 ; y < height; y ++) {
    for (x = 0 ; x < width; x ++) {
	    /*TODO improve speed: precalculate Y,U and V from pixel value (without extracting RGB) and store it into a table)*/

	  
	  /* Get RGB components for current pixel */
	  Uint8  r;
	  Uint8  g;
	  Uint8  b;
	  Uint8 new_y;
	  Uint8 new_u;
	  Uint8 new_v;
	  Uint32 current_pixel;
	  Uint8 BitsPerPixel;	  	  

          /*FIXME using 16 BPP surface only */

	  BitsPerPixel= image->format->BitsPerPixel;
	  //fprintf(stderr," BPP = %d\n",BitsPerPixel );
          switch(BitsPerPixel)
          {
           case 8:
             //Uint8 current_pixel;
             current_pixel = *((Uint8 *)image->pixels + y*image->pitch + x);
             break;
           case 16:
             //Uint16 current_pixel;
	     current_pixel = *((Uint16 *)image->pixels + y*image->pitch/2 + x);
             break;
           case 24:
	     //Test
	     current_pixel = *((Uint8 *)image->pixels + y*image->pitch + x *3);

	     //fprintf(stderr, "ERROR : 24 BPP Surfaces NOT supported\n");
             break;
           case 32:
	     //Uint32 current_pixel;
	     current_pixel = *((Uint32 *)image->pixels + y*image->pitch/4 + x);
             break;
          default:
             //fprintf(stderr, "ERROR : Surface has invalide BPP\n");
             break;

          }

          //Get RGB components from pixel
          SDL_GetRGB(current_pixel, image->format , &r,&g, &b);

          /*Calculate Y*/
          new_y = (Uint8)(0.299*r+0.587*g+0.098*b);

          /*copy pixel data to p_y:*/
          memcpy(&p_y[offset] ,&new_y,sizeof(new_y));
	  

	  /*YUV 4.2.0 conversion U and V have 1/2 Hor and 1/2 Vert frequency*/
	  if ((x%2 == 0) && (y%2 == 0)) 
	  { //copy 1/4 
	    /*Formula is hard to find so, don't blast it (have a look at yuv fourcc samples and take a look at the book (video-demystified.com))*/
	  
            new_u = (Uint8)(0.558*(b - new_y) +127);
            new_v = (Uint8)(0.709*(r - new_y) +127);
	  
	    /*Copy U and V into p_u and p_v*/
            memcpy(&p_u[offset_uv] ,&new_u,sizeof(new_u));
            memcpy(&p_v[offset_uv] ,&new_v,sizeof(new_v));
	    
	    
	    //fprintf(stdout, "Y = %d X = %d / last offset_uv = %d\n",y,x,offset_uv);	 
		 offset_uv ++;
		
          }
	  offset ++;
         	  
    }//for x
    

  }//for y

  convert_end =  SDL_GetTicks();

  //fprintf(stderr, "conversion time (millisec) = %d\n", convert_end - convert_start);
}


















/************************************
 *
 * ConvertRGBtoYUV
 *
 * **********************************/
int ConvertRGBtoYUV ( SDL_Surface *image, SDL_YUVSurface * yuv_surface){

  //SDL_YUVSurface * yuv_surface;
  
  //Get YUV from surface
  if (-1 == Get_YUV_From_Surface(image, yuv_surface))
	  return(-1);

   //fprintf(stderr, "Fin RGBtoYUV \n");
  return(1);//yuv_surface);

}
/*********************************
 *
 *  copy source yuv surface to dest overlay
 *  Only the position is used in the dstrect (the width and height are ignored).
 *  
 * 
 * **********************************/

int SDL_BlitOverlay(SDL_YUVSurface *src, SDL_Rect *srcrect, SDL_Overlay *dst, SDL_Rect *dstrect){
  /*declarations*/
  Uint8 * p_y_dst;
  Uint8 * p_u_dst;
  Uint8 * p_v_dst;
  Uint8 line;
  Uint32 start, end;
  Sint32 min_size;
  Sint32 min_h;
  Uint32 copy_offset;

  copy_offset = 0;

  start = SDL_GetTicks();

   //Calculate min pitch and height for copy size
   min_size = (src->pitch <= dst->pitches[0] - dstrect->x) ?
	       src->pitch : dst->pitches[0]- dstrect->x;

   min_h = (src->h <= dst->h - dstrect->y) ? src->h : dst->h - dstrect->y;

   if ((min_size < 0)||(min_h <0)) //No need to continue
	   return(0);

   //fprintf(stderr,"min h =%d\n",min_h);
  
  /*FIXME only blit YUV 4.2.0 overlays*/

  /*Calculate dest pointers*/
  for (line = 0 ; line < min_h; line ++) {
    /*copy ligne a ligne*/
    
    p_y_dst = dst->pixels[0] + (dstrect->y + line)* dst->pitches[0] + dstrect->x;

    memcpy(p_y_dst,  //to
           src->y_data + line * src->pitch,          //from
           min_size); //size

    if (line%2 == 0){
	    Sint32 uv_size;
	    uv_size = (min_size -1)/2 +1; //FIXME 
	    //image->w - 1 is the last pixel of the line
	    // / 2 : U and V have 1/2 freq FIXME 420 only
	    // +1 : size of last U (V)

      p_v_dst = dst->pixels[1] +  dst->pitches[1]* (copy_offset + dstrect->y /2) + (dstrect->x -1)/2 +1;
      p_u_dst = dst->pixels[2] +  dst->pitches[2]* (copy_offset + dstrect->y /2) + (dstrect->x -1)/2 +1;
  
      // V
      memcpy(p_v_dst,  //to
             src->v_data + copy_offset * ((src->pitch - 1)/2+1),//uv_size, //from
             uv_size); //size
      
      // U
      memcpy(p_u_dst,  //to
             src->u_data + copy_offset * ((src->pitch - 1)/2+1),//uv_size, //from
             uv_size); //size

      copy_offset++;
   }


}
  //fprintf(stderr,"Blit: last copy_offset  = %d\n", copy_offset);
  end = SDL_GetTicks();
  //fprintf(stderr,"Overlay Blit time (millisec) = %d\n", end - start);
  return(0);



}

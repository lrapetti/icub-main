/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author:  Valentina Gaggero
 * email:   valentina.gaggero@iit.it
 * website: www.robotcub.org
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

/* @file       eOcfg_nvsEP_sk_usrcbk_ebx.c
    @brief      This file keeps the user-defined functions used in every ems board ebx for endpoint mc
    @author     valentina.gaggero@iit.it
    @date       05/04/2012
 **/


// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "stdlib.h" 
#include "string.h"
#include "stdio.h"
#include <bits/time.h>

#include  "errno.h"

#ifdef _ICUB_CALLBACK_
//#include "IRobotInterface.h"
#include "FeatureInterface.h"
#endif

#define true 1
#define false 0

#include "EoCommon.h"
#include "EOarray.h"
#include "EOnv_hid.h"

#include "EoSkin.h"
#include "eOcfg_nvsEP_sk.h"
#include "eOcfg_nvsEP_sk_hid.h"

#include "eOcfg_nvsEP_sk_overridden.h"
#include "eOcfg_nvsEP_sk_emsboard_usr_hid.h"

// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------



// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------

//#include "eOcfg_nvsEP_mngmnt_usr_hid.h"

// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------
#define SKIN_BOARD_MAXNUM					7  //each skin endpoint is composed by 7 skin boards (mtb3)
#define SKIN_TRIANGLE4BOARD_MAXNUM			16 //each mtb3 manages 16 skin-triangle
#define SKIN_POINTS4TRIANGLE_MAXNUM  		12 //each triangle sends 12 point (value)
#define SKIN_RECCANFRAME4TRIANGLE_MAXNUM 	2 //each triangle sends 12 points in two can frame
#define MAX_ACQUISITION 					10000

// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------
void s_eo_cfg_nvsEP_sk_hid_print_arrayof10canframe(EOarray_of_10canframes *sk_array);
void s_eo_cfg__nvsEP_sk_hid_Histogram_Print(void);
void s_eo_cfg_nvsEP_sk_hid_Histogram_Reset(void);
void s_eo_cfg__nvsEP_sk_hid_ParseCanFrame(EOarray_of_10canframes *sk_array);
void s_eo_cfg_nvsEP_sk_hid_Dump_Data( const EOnv* nv);
void s_eo_cfg__nvsEP_sk_hid_Histogram_TV_Print(void);

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------
//static uint32_t s_skin_matrix_histogram[SKIN_BOARD_MAXNUM][SKIN_TRIANGLE4BOARD_MAXNUM][SKIN_POINTS4TRIANGLE_MAXNUM];
uint32_t s_skin_matrix_histogramRecCanFrame[SKIN_BOARD_MAXNUM][SKIN_TRIANGLE4BOARD_MAXNUM][SKIN_RECCANFRAME4TRIANGLE_MAXNUM];
struct timeval s_skin_matrix_histogramTV[MAX_ACQUISITION][SKIN_BOARD_MAXNUM][SKIN_TRIANGLE4BOARD_MAXNUM][SKIN_RECCANFRAME4TRIANGLE_MAXNUM];
struct timeval s_skin_matrix_histogramTV_zeros[SKIN_BOARD_MAXNUM][SKIN_TRIANGLE4BOARD_MAXNUM][SKIN_RECCANFRAME4TRIANGLE_MAXNUM];
uint32_t count = 0;
uint8_t keepGoingOn = 1;
uint8_t printed = 0;
FILE * outFile1, *outFile2;

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables
// --------------------------------------------------------------------------------------------------------------------
// empty-section



// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------

void s_eo_cfg_nvsEP_sk_hid_Dump_Data( const EOnv* nv)
{
	static uint8_t dump[1024] = {0};
	static struct timeval tv[1024] = {0};
	static struct timeval zero = {0};
	static uint32_t t = 0;

	EOarray_of_10canframes sk_array;
	memcpy( &sk_array, nv->rem, sizeof(EOarray_of_10canframes));

	int i, cardId, valid, mtbId, triangle, msgtype;

	for(i=0; i<sk_array.head.size; i++)
	{
		eOutil_canframe_t *canframe;
		int  j, p = 0;

		canframe = (eOutil_canframe_t*) &sk_array.data[i*sizeof(eOutil_canframe_t)];
		valid = (((canframe->id & 0x0F00) >> 8) == 3) ? 1 : 0;

		if(valid)
		{
			cardId = (canframe->id & 0x00f0) >> 4;
			switch (cardId)
			{
			case 14:
				mtbId = 0;
				break;
			case 13:
				mtbId = 1;
				break;
			case 12:
				mtbId = 2;
				break;
			case 11:
				mtbId = 3;
				break;
			case 10:
				mtbId = 4;
				break;
			case 9:
				mtbId = 5;
				break;
			case 8:
				mtbId = 6;
				break;
			}
			triangle = (canframe->id & 0x000f);
			msgtype= ((canframe->data[0])& 0x80);
			//			printf("\n mtb %d triangle %d\n", mtbId, triangle);

			if(t < MAX_ACQUISITION)
			{
				if( (0x03E0 == canframe->id) && (0x40 == (canframe->data[0])) )
				{
					gettimeofday(&(tv[t]),NULL);
					dump[t] = canframe->data[1];
					t++;
				}
			}
			else if(t==MAX_ACQUISITION)
			{
				for (p=0; p<MAX_ACQUISITION; p++)
				{
					fprintf(stdout, "%d\t\t%d%06d\n", dump[p], tv[p].tv_sec, tv[p].tv_usec);
				}

				FILE *log = fopen("/tmp/iii/log.txt", "w");
				if(NULL == log)
					printf(" Te piacerebbe!\n");
				else
				{
					fprintf(log, "valore  -  timestamp\n");
					for (p=0; p<MAX_ACQUISITION; p++)
					{
						fprintf(log, "%d\t\t%d%d\n", dump[p], tv[p].tv_sec, tv[p].tv_usec);
						//fprintf(stdout, " 0x%0x\t\t%d:%d\n", dump[p], tv[p].tv_sec, tv[p].tv_usec);
					}
					fclose(log);
					printf("Fine!!\n");
				}

				// DIE HERE!!!
				//while(1);
				t++;

			}
			//else
			//{
			//	printf("Array full!!\n");
			//}

		}
	}
}

//sk-update
//#ifdef OVERRIDE_eo_cfg_nvsEP_sk_hid_UPDT_sstatus__arrayof10canframe
extern void eo_cfg_nvsEP_sk_hid_UPDT_sstatus__arrayof10canframe(uint16_t n, const EOnv* nv, const eOabstime_t time, const uint32_t sign)
{
	//printf("new callback\n");
	// s_eo_cfg_nvsEP_sk_hid_Dump_Data(nv);
	EOarray_of_10canframes *sk_array = (EOarray_of_10canframes *)nv->rem;
	int i;
/*	Vale stats
  	if(nv->ep == endpoint_sk_emsboard_rightlowerarm)
	{
		if(keepGoingOn)
		{
			s_eo_cfg__nvsEP_sk_hid_ParseCanFrame(sk_array);

		}
		else if(!printed )
		{
			outFile1 = fopen("/usr/local/src/robot/pc104-logs/logHistogram.txt", "w+");
			printf("fopen: %s\n",strerror(errno));
			if(NULL == outFile1)
			{
				printf(" Te piacerebbe!\n");
				outFile1 = stdout;
			}
			outFile2 = fopen("/usr/local/src/robot/pc104-logs/logTimestamp.txt", "w+");
			printf("fopen: %s\n",strerror(errno));
			if(NULL == outFile2)
			{
				printf(" Te piacerebbe!\n");
				outFile2 = stdout;
			}
			s_eo_cfg__nvsEP_sk_hid_Histogram_Print();
			s_eo_cfg__nvsEP_sk_hid_Histogram_TV_Print();
			printf("fprintf: %s\n",strerror(errno));
			fclose(outFile1);
			fclose(outFile2);
			printf("fclose: %s\n",strerror(errno));
			printed = true;
			printf("Catched 'sk_array->head.size' bigger or equal to 9 %d times\n", count);
		}
	}
*/

#ifdef _ICUB_CALLBACK_
	FEAT_ID id;
	id.type = Skin;
	id.ep = nv->ep;

	void *featList;
	//	printf("iCub Callback, looking for %s\n", id.name);

	findAndFill(&id, (char *)sk_array);
#endif
}
//#endif

// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions
// --------------------------------------------------------------------------------------------------------------------

static int timeval_subtract(struct timeval *_result, struct timeval *_x, struct timeval *_y)
{
	/* Perform the carry for the later subtraction by updating y. */

	if(_x->tv_usec < _y->tv_usec)
	{
		int nsec    = (_y->tv_usec - _x->tv_usec) / 1000000 + 1;

		_y->tv_usec -= 1000000 * nsec;
		_y->tv_sec  += nsec;
	}

	if(_x->tv_usec - _y->tv_usec > 1000000)
	{
		int nsec    = (_x->tv_usec - _y->tv_usec) / 1000000;

		_y->tv_usec += 1000000 * nsec;
		_y->tv_sec  -= nsec;
	}

	/* Compute the time remaining to wait. tv_usec is certainly positive. */

	_result->tv_sec  = _x->tv_sec  - _y->tv_sec;
	_result->tv_usec = _x->tv_usec - _y->tv_usec;

	/* Return 1 if result is negative. */

	return _x->tv_sec < _y->tv_sec;
}



// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions
// --------------------------------------------------------------------------------------------------------------------
void s_eo_cfg_nvsEP_sk_hid_print_arrayof10canframe(EOarray_of_10canframes *sk_array)
{
	eOutil_canframe_t *canframe;
	int i, j;

	//	printf("--- ARRAY SIZE = %d  ---- \n", sk_array->head.size);
	//	for(i=0; i<sk_array->head.size; i++)
	//	{
	//		canframe = (eOutil_canframe_t*) &sk_array->data[i*sizeof(eOutil_canframe_t)];
	//		printf("SID = 0x%0x  ", canframe->id);
	//		printf("DATA: 0x");
	//		for(j=0; j<canframe->size;j++)
	//		{
	//			printf("%0x", canframe->data[j]);
	//		}
	//		printf("   tri = %0x,  %0x", (canframe->id & 0x000f), (((canframe->data[0])& 0x80) ? 0xC0 :0x40) );
	//		printf("\n\n");
	//
	//	}

}


void s_eo_cfg_nvsEP_sk_hid_Histogram_Reset(void)
{
	//	memset(s_skin_matrix_histogram, 0, sizeof(s_skin_matrix_histogram));
	memset(s_skin_matrix_histogramRecCanFrame, 0, sizeof(s_skin_matrix_histogramRecCanFrame));
}

void s_eo_cfg__nvsEP_sk_hid_ParseCanFrame(EOarray_of_10canframes *sk_array)
{
	eOutil_canframe_t *canframe;
	int i, j;

	uint8_t boardId = 0;
	uint8_t boardAddr = 0;
	uint8_t triangle = 0;
	uint8_t point = 0 ;
	uint8_t point_offset;
	uint8_t msgtype = 0;
	uint8_t row = 0;
	uint8_t offset;

	struct timeval tmp;
	if(sk_array->head.size >= 9)
	{
		count++;
	}

	for(i=0; i<sk_array->head.size; i++)
	{
		canframe = (eOutil_canframe_t*) &sk_array->data[i*sizeof(eOutil_canframe_t)];

		boardAddr = (canframe->id & 0x00f0)>>4;
		boardId = boardAddr - 8;
		if(boardId>6) //gli indirizzi sono 8....14
		{
			continue;
		}
		triangle = (canframe->id & 0x000f);
		msgtype= ((canframe->data[0])& 0x80);

		if(msgtype)
		{ //0xC0
			row = 1;
			offset = 7;
		}
		else
		{ //0x40
			row = 0;
			offset = 0;
		}

		s_skin_matrix_histogramRecCanFrame[boardId][triangle][row]++;
		uint32_t idx = s_skin_matrix_histogramRecCanFrame[boardId][triangle][row];
		if (idx == 1)
		{
			//printf("s_eo_cfg__nvsEP_sk_hid_ParseCanFrame\n");
			gettimeofday(&s_skin_matrix_histogramTV_zeros[boardId][triangle][row], NULL);
		}
		else
		{
			if(idx < MAX_ACQUISITION)
			{
				gettimeofday(&tmp, NULL);
				timeval_subtract(&s_skin_matrix_histogramTV[idx][boardId][triangle][row], &tmp, &s_skin_matrix_histogramTV_zeros[boardId][triangle][row]);
				s_skin_matrix_histogramTV_zeros[boardId][triangle][row].tv_sec = tmp.tv_sec;
				s_skin_matrix_histogramTV_zeros[boardId][triangle][row].tv_usec = tmp.tv_usec;
				//			if( (s_skin_matrix_histogramTV[idx][boardId][triangle][row].tv_sec > 0) || (s_skin_matrix_histogramTV[idx][boardId][triangle][row].tv_usec > 70*1000) )
				//				printf("Vai a vendere frittelle!!\n");
			}
			else
			{
				keepGoingOn = false;
			}
		}
		/*
		 * 		questo può essere usato per creare la matrice di valori
		for(i = 0; i<canframe->size; i++)
		{
			point = offset+i;
			s_skin_matrix_histogram[boardId][triangle][point] = canframe->data[i+1];
		}
		 */

	}


}


void s_eo_cfg__nvsEP_sk_hid_Histogram_Print(void)
{
	uint32_t i,j,k;

	for(i=0; i<SKIN_BOARD_MAXNUM; i++)
	{
		fprintf(outFile1, "board addr: %0x\n", i+8);
		for(j=0; j<SKIN_TRIANGLE4BOARD_MAXNUM; j++)
		{
			fprintf(outFile1, "t:%0d - %0d, %0d |\t", j, s_skin_matrix_histogramRecCanFrame[i][j][0], s_skin_matrix_histogramRecCanFrame[i][j][1]);
		}
		fprintf(outFile1,"\n");
	}
}

void s_eo_cfg__nvsEP_sk_hid_Histogram_TV_Print(void)
{
	uint32_t i,j,k, l;

	for(i=0; i<SKIN_BOARD_MAXNUM; i++)
	{
		for(j=0; j<SKIN_TRIANGLE4BOARD_MAXNUM; j++)
		{
			for(k=0; k<2; k++)
			{
				for(l=0; l<MAX_ACQUISITION; l++)
				{
					fprintf(outFile2,"%d:%d:%d:%d %06d.%06d\n", i,j,k,l, s_skin_matrix_histogramTV[l][i][j][k]);
				}
			}
			fprintf(outFile2,"\n");
		}
		fprintf(outFile2,"\n\n");
	}
}

// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------



/*

    Copyright (C) 2005,2006,2007,2012,2013 Alois Schloegl <alois.schloegl@gmail.com>

    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 


    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.


 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../biosig-dev.h"

EXTERN_C int sopen_SCP_write(HDRTYPE* hdr) {	
/*
	this function is a stub or placeholder and need to be defined in order to be useful. 
	It will be called by the function SOPEN in "biosig.c"

	Input: 
		char* Header	// contains the file content
		
	Output: 
		HDRTYPE *hdr	// defines the HDR structure accoring to "biosig.h"
*/	
	uint8_t*	ptr; 	// pointer to memory mapping of the file layout
	uint8_t*	PtrCurSect;	// point to current section 
	int		curSect; 
	uint32_t 	len; 
	uint16_t 	crc; 
	uint32_t	i; 
	uint32_t 	sectionStart; 
	struct tm* 	T0_tm;
	double 		AVM, avm; 
	uint16_t	avm16; 
	struct aecg*	aECG;		

	if (VERBOSE_LEVEL>7) fprintf(stdout,"SOPEN_SCP_WRITE 101\n");

	if ((fabs(hdr->VERSION - 1.3)<0.01) && (fabs(hdr->VERSION-2.0)<0.01))  
		fprintf(stderr,"Warning SOPEN (SCP-WRITE): Version %f not supported\n",hdr->VERSION);
		

	uint8_t VERSION = 20; // (uint8_t)round(hdr->VERSION*10); // implemented version number 
	if (hdr->aECG==NULL) {
		fprintf(stderr,"Warning SOPEN_SCP_WRITE: No aECG info defined\n");
		hdr->aECG = malloc(sizeof(struct aecg));
		aECG = (struct aecg*)hdr->aECG;
		aECG->diastolicBloodPressure=0.0;				 
		aECG->systolicBloodPressure=0.0;
		aECG->MedicationDrugs="/0";
		aECG->ReferringPhysician="/0";
		aECG->LatestConfirmingPhysician="/0";
		aECG->Diagnosis="/0";
		aECG->EmergencyLevel=0;
#if (BIOSIG_VERSION < 10500)
		aECG->Section8.NumberOfStatements = 0; 
		aECG->Section8.Statements = NULL; 
		aECG->Section11.NumberOfStatements = 0; 
		aECG->Section11.Statements = NULL; 
#endif
	}
	else 
		aECG = (struct aecg*)hdr->aECG;


//fprintf(stdout,"SCP-Write: IIb %s\n",hdr->aECG->ReferringPhysician);
	/* predefined values */
	aECG->Section1.Tag14.INST_NUMBER 	= 0;		// tag 14, byte 1-2 
	aECG->Section1.Tag14.DEPT_NUMBER 	= 0;		// tag 14, byte 3-4 
	aECG->Section1.Tag14.DEVICE_ID 	= 0;		// tag 14, byte 5-6 
	aECG->Section1.Tag14.DeviceType 	= 0;		// tag 14, byte 7: 0: Cart, 1: System (or Host) 
	aECG->Section1.Tag14.MANUF_CODE 	= 255;		// tag 14, byte 8 (MANUF_CODE has to be 255)
	aECG->Section1.Tag14.MOD_DESC  	= "Cart1";	// tag 14, byte 9 (MOD_DESC has to be "Cart1")
	aECG->Section1.Tag14.VERSION	= VERSION;	// tag 14, byte 15 (VERSION * 10)
	aECG->Section1.Tag14.PROT_COMP_LEVEL = 0xA0;	// tag 14, byte 16 (PROT_COMP_LEVEL has to be 0xA0 => level II)
	aECG->Section1.Tag14.LANG_SUPP_CODE  = 0x00;	// tag 14, byte 17 (LANG_SUPP_CODE has to be 0x00 => Ascii only, latin and 1-byte code)
	aECG->Section1.Tag14.ECG_CAP_DEV 	= 0xD0;		// tag 14, byte 18 (ECG_CAP_DEV has to be 0xD0 => Acquire, (No Analysis), Print and Store)
	aECG->Section1.Tag14.MAINS_FREQ  	= 0;		// tag 14, byte 19 (MAINS_FREQ has to be 0: unspecified, 1: 50 Hz, 2: 60Hz)
	aECG->Section1.Tag14.ANAL_PROG_REV_NUM 	= "";
	aECG->Section1.Tag14.SERIAL_NUMBER_ACQ_DEV = "";
	aECG->Section1.Tag14.ACQ_DEV_SYS_SW_ID 	= "";
	aECG->Section1.Tag14.ACQ_DEV_SCP_SW	= "OpenECG XML-SCP 1.00"; // tag 14, byte 38 (SCP_IMPL_SW has to be "OpenECG XML-SCP 1.00")
	aECG->Section1.Tag14.ACQ_DEV_MANUF 	= "Manufacturer";	// tag 14, byte 38 (ACQ_DEV_MANUF has to be "Manufacturer")

	aECG->Section5.Length = 0; 
	aECG->Section6.Length = 0; 

	/*  */
	aECG->FLAG.HUFFMAN = 0;
	aECG->FLAG.REF_BEAT= 0;
	aECG->FLAG.DIFF 	= 0;
	aECG->FLAG.BIMODAL = 0;


	if (VERBOSE_LEVEL>7) fprintf(stdout,"SOPEN_SCP_WRITE 111\n");

	ptr = (uint8_t*)hdr->AS.Header;

	int NSections = 12; 
	// initialize section 0
	sectionStart  = 6+16+NSections*10;
	ptr = (uint8_t*)realloc(ptr,sectionStart); 
	memset(ptr,0,sectionStart);
	
	uint32_t curSectLen = 0; // current section length
	for (curSect=NSections-1; curSect>=0; curSect--) {

		curSectLen = 0; // current section length
		//ptr = (uint8_t*)realloc(ptr,sectionStart+curSectLen); 

		if (VERBOSE_LEVEL>7) fprintf(stdout,"Section %i %p\n",curSect,ptr);

		if (curSect==0)  // SECTION 0 
		{
			hdr->HeadLen = sectionStart; // length of all other blocks together
			ptr = (uint8_t*)realloc(ptr,hdr->HeadLen); // total file length

			curSectLen = 16; // current section length
			sectionStart = 6; 
		
			memcpy(ptr+16,"SCPECG",6); // reserved
			curSectLen += NSections*10;
		}
		else if (curSect==1)  // SECTION 1 
		{
			ptr = (uint8_t*)realloc(ptr,sectionStart+10000); 
			PtrCurSect = ptr+sectionStart; 
			curSectLen = 16; // current section length

			if (VERBOSE_LEVEL>7) fprintf(stdout,"Section 1 Tag 0 \n");

			// Tag 0 (max len = 64)
			if (!hdr->FLAG.ANONYMOUS && (hdr->Patient.Name != NULL)) 
			{
				*(ptr+sectionStart+curSectLen) = 0;	// tag
				len = strlen(hdr->Patient.Name) + 1;
				leu16a(len, ptr+sectionStart+curSectLen+1);	// length
				strncpy((char*)ptr+sectionStart+curSectLen+3,hdr->Patient.Name,len);	// field
				curSectLen += len+3; 
			}

			if (VERBOSE_LEVEL>7) fprintf(stdout,"Section 1 Tag 1 \n");

			// Tag 1 (max len = 64) Firstname 
/*
			*(ptr+sectionStart+curSectLen) = 1;	// tag
			len = strlen(hdr->Patient.Name) + 1;
			leu16a(len, ptr+sectionStart+curSectLen+1);	// length
			strncpy((char*)ptr+sectionStart+curSectLen+3,hdr->Patient.Name,len);	// field
			curSectLen += len+3; 
*/	
		
			// Tag 2 (max len = 64) Patient ID 
			if (VERBOSE_LEVEL>7) fprintf(stdout,"Section 1 Tag 2 \n");

//			if (hdr->Patient.Id != NULL) {
			if (strlen(hdr->Patient.Id)>0) {
				*(ptr+sectionStart+curSectLen) = 2;	// tag
				len = strlen(hdr->Patient.Id) + 1;
				leu16a(len, ptr+sectionStart+curSectLen+1);	// length
				strncpy((char*)ptr+sectionStart+curSectLen+3,hdr->Patient.Id,len);	// field
				curSectLen += len+3;
			}	 

// fprintf(stdout,"Section %i Len %i %x\n",curSect,curSectLen,sectionStart);

			// Tag 3 (max len = 64) Second Last Name 
/*
			*(ptr+sectionStart+curSectLen) = 3;	// tag
			len = strlen(hdr->Patient.Name) + 1;
			leu16a(len, ptr+sectionStart+curSectLen+1);	// length
			strncpy(ptr+sectionStart+curSectLen+3,hdr->Patient.Name,len);	// field
			curSectLen += len+3; 
*/

			// Tag 5 (len = 4) 
			if ((hdr->Patient.Birthday) > 0) {
				T0_tm = gdf_time2tm_time(hdr->Patient.Birthday);
				
				*(ptr+sectionStart+curSectLen) = 5;	// tag
				leu16a(4, ptr+sectionStart+curSectLen+1);	// length
				leu16a(T0_tm->tm_year+1900, ptr+sectionStart+curSectLen+3);// year
				*(ptr+sectionStart+curSectLen+5) = (uint8_t)(T0_tm->tm_mon + 1);	// month
				*(ptr+sectionStart+curSectLen+6) = (uint8_t)(T0_tm->tm_mday); 	// day
				curSectLen += 7;
			} 

			// Tag 6 (len = 3)   Height
			if (hdr->Patient.Height>0.0) {
				*(ptr+sectionStart+curSectLen) = 6;	// tag
				leu16a(3, ptr+sectionStart+curSectLen+1);	// length
				leu16a(hdr->Patient.Height, ptr+sectionStart+curSectLen+3);	// value
				*(ptr+sectionStart+curSectLen+5) = 1;	// cm
				curSectLen += 6;
			}	 

			// Tag 7 (len = 3)	Weight
			if (hdr->Patient.Weight>0.0) {
				*(ptr+sectionStart+curSectLen) = 7;	// tag
				leu16a(3, ptr+sectionStart+curSectLen+1);	// length
				leu16a(hdr->Patient.Weight, ptr+sectionStart+curSectLen+3);	// value
				*(ptr+sectionStart+curSectLen+5) = 1;	// kg
				curSectLen += 6;
			}	 

			// Tag 8 (len = 1)
			if (hdr->Patient.Sex != 0) {
				*(ptr+sectionStart+curSectLen) = 8;	// tag
				leu16a(1, ptr+sectionStart+curSectLen+1);	// length
				*(ptr+sectionStart+curSectLen+3) = hdr->Patient.Sex;	// value
				curSectLen += 4;
			}	 

			// Tag 11 (len = 2)
			if (aECG->systolicBloodPressure>0.0) {
				*(ptr+sectionStart+curSectLen) = 11;	// tag
				leu16a(2, ptr+sectionStart+curSectLen+1);	// length
				leu16a((uint16_t)aECG->systolicBloodPressure, ptr+sectionStart+curSectLen+3);	// value
				curSectLen += 5;
			};	 

			// Tag 12 (len = 2)
			if (aECG->diastolicBloodPressure>0.0) {
				*(ptr+sectionStart+curSectLen) = 12;	// tag
				leu16a(2, ptr+sectionStart+curSectLen+1);	// length
				leu16a((uint16_t)aECG->diastolicBloodPressure, ptr+sectionStart+curSectLen+3);	// value
				curSectLen += 5;
			};
			// Tag 13 (max len = 80)
			aECG->Diagnosis="";
			len = strlen(aECG->Diagnosis); 
			if (len>0) {
				*(ptr+sectionStart+curSectLen) = 13;	// tag
				len = min(64,len+1);
				leu16a(len, ptr+sectionStart+curSectLen+1);	// length
				strncpy((char*)(ptr+sectionStart+curSectLen+3),aECG->Diagnosis,len);
				curSectLen += 3+len;
			};	 

			// Tag 14 (max len = 2 + 2 + 2 + 1 + 1 + 6 + 1 + 1 + 1 + 1 + 1 + 16 + 1 + 25 + 25 + 25 + 25 + 25)
			if (VERBOSE_LEVEL>7) fprintf(stdout,"Section 1 Tag 14 \n");

			// Total = 161 (max value)
			*(ptr+sectionStart+curSectLen) = 14;	// tag
			//len = 41; 	// minimum length
			// leu16a(len, ptr+sectionStart+curSectLen+1);	// length	
			memset(ptr+sectionStart+curSectLen+3,0,41);  // dummy value 
			
			curSectLen += 3; 
			leu16a(aECG->Section1.Tag14.INST_NUMBER, ptr+sectionStart+curSectLen);
			leu16a(aECG->Section1.Tag14.DEPT_NUMBER, ptr+sectionStart+curSectLen+2);
			leu16a(aECG->Section1.Tag14.DEVICE_ID,   ptr+sectionStart+curSectLen+4);
			*(ptr+sectionStart+curSectLen+ 6) = aECG->Section1.Tag14.DeviceType;
			*(ptr+sectionStart+curSectLen+ 7) = aECG->Section1.Tag14.MANUF_CODE;	// tag 14, byte 7 (MANUF_CODE has to be 255)
			strncpy((char*)(ptr+sectionStart+curSectLen+8), aECG->Section1.Tag14.MOD_DESC, 6);	// tag 14, byte 7 (MOD_DESC has to be "Cart1")
			*(ptr+sectionStart+curSectLen+14) = VERSION;		// tag 14, byte 14 (VERSION has to be 20)
			*(ptr+sectionStart+curSectLen+14) = aECG->Section1.Tag14.VERSION;
			*(ptr+sectionStart+curSectLen+15) = aECG->Section1.Tag14.PROT_COMP_LEVEL; 		// tag 14, byte 15 (PROT_COMP_LEVEL has to be 0xA0 => level II)
			*(ptr+sectionStart+curSectLen+16) = aECG->Section1.Tag14.LANG_SUPP_CODE;		// tag 14, byte 16 (LANG_SUPP_CODE has to be 0x00 => Ascii only, latin and 1-byte code)
			*(ptr+sectionStart+curSectLen+17) = aECG->Section1.Tag14.ECG_CAP_DEV;		// tag 14, byte 17 (ECG_CAP_DEV has to be 0xD0 => Acquire, (No Analysis), Print and Store)
			*(ptr+sectionStart+curSectLen+18) = aECG->Section1.Tag14.MAINS_FREQ;		// tag 14, byte 18 (MAINS_FREQ has to be 0: unspecified, 1: 50 Hz, 2: 60Hz)
			*(ptr+sectionStart+curSectLen+35) = strlen(aECG->Section1.Tag14.ANAL_PROG_REV_NUM)+1;		// tag 14, byte 34 => length of ANAL_PROG_REV_NUM + 1 = 1
			uint16_t len1 = 36;

			char* tmp; 
			tmp = aECG->Section1.Tag14.ANAL_PROG_REV_NUM;	
			len = min(25, strlen(tmp) + 1);
			strncpy((char*)(ptr+sectionStart+curSectLen+len1), tmp, len);
			len1 += len;

			tmp = aECG->Section1.Tag14.SERIAL_NUMBER_ACQ_DEV;	
			len = min(25, strlen(tmp) + 1);
			strncpy((char*)(ptr+sectionStart+curSectLen+len1), tmp, len);
			len1 += len;

			tmp = aECG->Section1.Tag14.ACQ_DEV_SYS_SW_ID;	
			len = min(25, strlen(tmp) + 1);
			strncpy((char*)(ptr+sectionStart+curSectLen+len1), tmp, len);
			len1 += len;

			tmp = aECG->Section1.Tag14.ACQ_DEV_SCP_SW;	
			len = min(25, strlen(tmp) + 1);
			strncpy((char*)(ptr+sectionStart+curSectLen+len1), tmp, len);
			len1 += len;

			tmp = aECG->Section1.Tag14.ACQ_DEV_MANUF;	
			len = min(25, strlen(tmp) + 1);
			strncpy((char*)(ptr+sectionStart+curSectLen+len1), tmp, len);
			len1 += len;

			leu16a(len1, ptr+sectionStart+curSectLen+1-3);	// length
			curSectLen += len1; 

			// Tag 16 (max len = 80)
			if (VERBOSE_LEVEL>7) fprintf(stdout,"Section 1 Tag 16 \n");
			len = hdr->ID.Hospital ? strlen(hdr->ID.Hospital) : 0; 
			if (len > 0) {
				*(ptr+sectionStart+curSectLen) = 16;	// tag
				len = min(64,len+1);
				leu16a(len, ptr+sectionStart+curSectLen+1);	// length
				strncpy((char*)(ptr+sectionStart+curSectLen+3),hdr->ID.Hospital,len);
				curSectLen += 3+len;
			}

			// Tag 20 (max len = 64 ) 
			if (VERBOSE_LEVEL>7) fprintf(stdout,"Section 1 Tag 20 \n");
			len = aECG->ReferringPhysician ? strlen(aECG->ReferringPhysician) : 0;
			if (len > 0) {
				*(ptr+sectionStart+curSectLen) = 20;	// tag
				len = min(64,len+1);
				leu16a(len, ptr+sectionStart+curSectLen+1);	// length
				strncpy((char*)(ptr+sectionStart+curSectLen+3),aECG->ReferringPhysician,len);
				curSectLen += 3+len;
			};	 

			// Tag 21 (max len = 64 )
			if (VERBOSE_LEVEL>7) fprintf(stdout,"Section 1 Tag 21 \n");
			len = aECG->MedicationDrugs ? strlen(aECG->MedicationDrugs) : 0; 
			if (len>0) {
				*(ptr+sectionStart+curSectLen) = 21;	// tag
				len = min(64,len+1);
				leu16a(len, ptr+sectionStart+curSectLen+1);	// length
				strncpy((char*)(ptr+sectionStart+curSectLen+3),aECG->MedicationDrugs,len);
				curSectLen += 3+len;
			};	 

			// Tag 22 (max len = 40 )
			if (VERBOSE_LEVEL>7) fprintf(stdout,"Section 1 Tag 22 \n");
			len = hdr->ID.Technician ? strlen(hdr->ID.Technician) : 0; 
			if (len > 0) {
				*(ptr+sectionStart+curSectLen) = 22;	// tag
				len = min(64,len+1);
				leu16a(len, ptr+sectionStart+curSectLen+1);	// length
				strncpy((char*)(ptr+sectionStart+curSectLen+3),hdr->ID.Technician,len);
				curSectLen += 3+len;
			}

			// Tag 24 ( len = 1 ) 
			if (VERBOSE_LEVEL>7) fprintf(stdout,"Section 1 Tag 24 \n");
			*(ptr+sectionStart+curSectLen) = 24;	// tag
			leu16a(1, ptr+sectionStart+curSectLen+1);	// length
			*(ptr+sectionStart+curSectLen+3) = aECG->EmergencyLevel;
			curSectLen += 4;

			// Tag 25 (len = 4)
			if (VERBOSE_LEVEL>7) fprintf(stdout,"Section 1 Tag 25 \n");

			gdf_time T1 = hdr->T0;
#ifndef __APPLE__
			T1 += (int32_t)ldexp(timezone/86400.0,32);			
#endif 
			T0_tm = gdf_time2tm_time(T1);

			*(ptr+sectionStart+curSectLen) = 25;	// tag
			leu16a(4, ptr+sectionStart+curSectLen+1);	// length
			leu16a(T0_tm->tm_year+1900, ptr+sectionStart+curSectLen+3);// year
			*(ptr+sectionStart+curSectLen+5) = (uint8_t)(T0_tm->tm_mon + 1);// month
			*(ptr+sectionStart+curSectLen+6) = (uint8_t)T0_tm->tm_mday; 	// day
			curSectLen += 7; 

			// Tag 26 (len = 3)
			*(ptr+sectionStart+curSectLen) = 26;	// tag
			leu16a(3, ptr+sectionStart+curSectLen+1);	// length
			*(ptr+sectionStart+curSectLen+3) = (uint8_t)T0_tm->tm_hour;	// hour
			*(ptr+sectionStart+curSectLen+4) = (uint8_t)T0_tm->tm_min;	// minute
			*(ptr+sectionStart+curSectLen+5) = (uint8_t)T0_tm->tm_sec; 	// second
			curSectLen += 6; 

			if (hdr->NS>0)  {
			// Tag 27 (len = 3) highpass filter 
			*(ptr+sectionStart+curSectLen) = 27;	// tag
			leu16a(2, ptr+sectionStart+curSectLen+1);	// length
			leu16a((uint16_t)hdr->CHANNEL[1].HighPass, ptr+sectionStart+curSectLen+3);	// hour
			curSectLen += 5; 

			// Tag 28 (len = 3)  lowpass filter
			*(ptr+sectionStart+curSectLen) = 28;	// tag
			leu16a(2, ptr+sectionStart+curSectLen+1);	// length
			leu16a((uint16_t)hdr->CHANNEL[1].LowPass, ptr+sectionStart+curSectLen+3);	// hour
			curSectLen += 5; 

			// Tag 29 (len = 1) filter bitmap
			uint8_t bitmap = 0; 
        		if (fabs(hdr->CHANNEL[1].LowPass-60.0)<0.01) 
				bitmap = 1; 
        		else if (fabs(hdr->CHANNEL[1].LowPass-50.0)<0.01) 
				bitmap = 2;
			else 
				bitmap = 0; 		 
			*(ptr+sectionStart+curSectLen) = 29;	// tag
			leu16a(1, ptr+sectionStart+curSectLen+1);	// length			
			*(ptr+sectionStart+curSectLen+3) = bitmap; 
			curSectLen += 4; 

			}
			
			// Tag 32 (len = 5)
			if (VERBOSE_LEVEL>7) fprintf(stdout,"Section 1 Tag 32 \n");

			*(ptr+sectionStart+curSectLen) = 32;	// tag
			leu16a(2, ptr+sectionStart+curSectLen+1);	// length
			if (hdr->Patient.Impairment.Heart==1) {
				*(ptr+sectionStart+curSectLen+3) = 0; 
				*(ptr+sectionStart+curSectLen+4) = 1; 	// Apparently healthy
				curSectLen += 5; 
			}			
			else if (hdr->Patient.Impairment.Heart==3) {
				*(ptr+sectionStart+curSectLen+3) = 0; 
				*(ptr+sectionStart+curSectLen+4) = 42; 	// Implanted cardiac pacemaker
				curSectLen += 5; 
			}			

			// Tag 34 (len = 5)
			*(ptr+sectionStart+curSectLen) = 34;	// tag
			leu16a(5, ptr+sectionStart+curSectLen+1);	// length
			lei16a(hdr->tzmin, ptr+sectionStart+curSectLen+3);
			lei16a(0, ptr+sectionStart+curSectLen+5); 
			curSectLen += 8; 

			// Tag 255 (len = 0)
			*(ptr+sectionStart+curSectLen) = 255;	// tag
			leu16a(0, ptr+sectionStart+curSectLen+1);	// length
			curSectLen += 3; 

			// Evaluate the size and correct it if odd
			if (curSectLen & 1) {
				*(ptr+sectionStart+curSectLen++) = 0; 
			}

			if (VERBOSE_LEVEL>7) fprintf(stdout,"End-of-Section %i %p\n",curSect,ptr);

		}
		else if (curSect==2)  // SECTION 2 
		{
		}
    		else if (curSect==3)  // SECTION 3 
		{
			ptr = (uint8_t*)realloc(ptr,sectionStart+16+2+9*hdr->NS+1); 
			PtrCurSect = ptr+sectionStart; 
			curSectLen = 16; // current section length

			// Number of leads enclosed
			*(ptr+sectionStart+curSectLen++) = hdr->NS;

// ### Situations with reference beat subtraction are not supported
// Situations with not all the leads simultaneously recorded are not supported
// Situations number of leads simultaneouly recorded != total number of leads are not supported
// We assume all the leads are recorded simultaneously
			*(ptr+sectionStart+curSectLen++) = (hdr->NS<<3) | 0x04;

			for (i = 0; i < hdr->NS; i++) {
				leu32a(1L, ptr+sectionStart+curSectLen);
				leu32a(hdr->data.size[0], ptr+sectionStart+curSectLen+4);
				*(ptr+sectionStart+curSectLen+8) = (uint8_t)hdr->CHANNEL[i].LeadIdCode;
				curSectLen += 9;
			}

			// Evaluate the size and correct it if odd
			if ((curSectLen % 2) != 0) {
				*(ptr+sectionStart+curSectLen++) = 0; 
			}
			memset(ptr+sectionStart+10,0,6); // reserved
		}
		else if (curSect==4)  // SECTION 4 
		{
		}
		else if (curSect==5)  // SECTION 5 
		{
			curSectLen = 0; // current section length
			aECG->Section5.StartPtr = sectionStart; 
			aECG->Section5.Length = curSectLen; 
		}
		else if (curSect==6)  // SECTION 6 
		{
			uint16_t GDFTYP = 3; 
			size_t SZ = GDFTYP_BITS[GDFTYP]>>3;
			for (i = 0; i < hdr->NS; i++) 
				hdr->CHANNEL[i].GDFTYP = GDFTYP; 
			ptr = (uint8_t*)realloc(ptr,sectionStart+16+6+2*hdr->NS+SZ*(hdr->data.size[0]*hdr->data.size[1])); 

			PtrCurSect = ptr+sectionStart; 
			curSectLen = 16; // current section length

			// Create all the fields
			// Amplitude Value Multiplier (AVM)
			i = 0;			
			AVM = hdr->CHANNEL[i].Cal * 1e9 * PhysDimScale(hdr->CHANNEL[i].PhysDimCode);
			for (i = 1; i < hdr->NS; i++) {
				// check for physical dimension and adjust scaling factor to "nV"
				avm = hdr->CHANNEL[i].Cal * 1e9 * PhysDimScale(hdr->CHANNEL[i].PhysDimCode);
				// check whether all channels have the same scaling factor
				if (fabs((AVM - avm)/AVM) > 1e-14)
					fprintf(stderr,"Warning SOPEN (SCP-WRITE): scaling factors differ between channel #1 and #%i. Scaling factor of 1st channel is used.\n",i+1);
			};
			avm16 = lrint(AVM);
			leu16a(avm16, ptr+sectionStart+curSectLen);
			avm = leu16p(ptr+sectionStart+curSectLen);
			curSectLen += 2;
			if (fabs((AVM - avm)/AVM)>1e-14)
				fprintf(stderr,"Warning SOPEN (SCP-WRITE): Scaling factor has been truncated (%f instead %f).\n",avm,AVM);

			// Sample interval
			AVM = 1e6/hdr->SampleRate;
			avm16 = lrint(AVM);
			leu16a(avm16, ptr+sectionStart+curSectLen);
			avm = leu16p(ptr+sectionStart+curSectLen);
			curSectLen += 2;
			if (fabs((AVM - avm)/AVM)>1e-14)
				fprintf(stderr,"Warning SOPEN (SCP-WRITE): Sampling interval has been truncated (%f instead %f us).\n",avm,AVM);

			// Diff used
			*(ptr+sectionStart+curSectLen++) = 0;

			// Bimodal/Non-bimodal
			*(ptr+sectionStart+curSectLen++) = 0;


			/* DATA COMPRESSION
			    currently, no compression method is supported. In case of data compression, the
			    data compression can happen here. 
			*/
			
			// Fill the length block
			for (i = 0; i < hdr->NS; i++) {
				leu16a((uint16_t)hdr->data.size[0]*2, ptr+sectionStart+curSectLen);
				avm = leu16p(ptr+sectionStart+curSectLen);
				AVM = hdr->data.size[0]*2;
				if (fabs((AVM - avm)/AVM)>1e-14)
					fprintf(stderr,"Warning SOPEN (SCP-WRITE): Block length truncated (%f instead %f us).\n",avm,AVM);
				curSectLen += 2;
			}

			/* data in channel multiplexed order */
			for (i = 0; i < hdr->NS; i++) {
				hdr->CHANNEL[i].SPR *= hdr->NRec; 
			};
			hdr->NRec = 1; 

			// Prepare filling the data block with the ECG samples by SWRITE
			// free(hdr->AS.rawdata);
			// hdr->AS.rawdata = PtrCurSect+16+6+2*hdr->NS;
			curSectLen += SZ*(hdr->data.size[0]*hdr->data.size[1]);

			// Evaluate the size and correct it if odd
			if ((curSectLen % 2) != 0) {
				fprintf(stderr,"Warning Section 6 has an odd length\n"); 
				*(ptr+sectionStart+curSectLen++) = 0; 
			}

			memset(ptr+sectionStart+10,0,6); // reserved
			aECG->Section6.StartPtr = sectionStart; 
			aECG->Section6.Length = curSectLen; 
		}
#if (BIOSIG_VERSION >= 10500)
		else if (curSect==7)  // SECTION 7 
		{
			if (hdr->SCP.Section7 != NULL) {
				curSectLen = hdr->SCP.Section7Length+16; // current section length
				ptr = (uint8_t*)realloc(ptr,sectionStart+curSectLen);
				PtrCurSect = ptr+sectionStart;
				memcpy(PtrCurSect+16,hdr->SCP.Section7,hdr->SCP.Section7Length);
			}
		}
		else if (curSect==8)  // SECTION 8 
		{
			if (hdr->SCP.Section8 != NULL) {
				curSectLen = hdr->SCP.Section8Length+16; // current section length
				ptr = (uint8_t*)realloc(ptr,sectionStart+curSectLen);
				PtrCurSect = ptr+sectionStart;
				memcpy(PtrCurSect,hdr->SCP.Section8,hdr->SCP.Section8Length);
			}
		}
		else if (curSect==9)  // SECTION 9 
		{
			if (hdr->SCP.Section9 != NULL) {
				curSectLen = hdr->SCP.Section9Length+16; // current section length
				ptr = (uint8_t*)realloc(ptr,sectionStart+curSectLen);
				PtrCurSect = ptr+sectionStart;
				memcpy(PtrCurSect,hdr->SCP.Section9,hdr->SCP.Section9Length);
			}
		}
		else if (curSect==10)  // SECTION 10 
		{
			if (hdr->SCP.Section10 != NULL) {
				curSectLen = hdr->SCP.Section10Length+16; // current section length
				ptr = (uint8_t*)realloc(ptr,sectionStart+curSectLen);
				PtrCurSect = ptr+sectionStart;
				memcpy(PtrCurSect+16,hdr->SCP.Section10,hdr->SCP.Section10Length);
			}
		}
		else if (curSect==11)  // SECTION 11
		{
			if (hdr->SCP.Section11 != NULL) {
				curSectLen = hdr->SCP.Section11Length+16; // current section length
				ptr = (uint8_t*)realloc(ptr,sectionStart+curSectLen);
				PtrCurSect = ptr+sectionStart;
				memcpy(PtrCurSect+16,hdr->SCP.Section11,hdr->SCP.Section11Length);
			}
		}
#if (BIOSIG_VERSION >= 10700)
		else if (curSect==12) // SECTION 12
		{
			if ( (VERSION == 30) && (hdr->SCP.Section12.NumberOfEntries > 0) ) {
				curSectLen = hdr->SCP.Section12.NumberOfEntries * sizeof(hdr->SCP.Section12.annotatedECG[0]);  // current section length without 16 bytes
				ptr = (uint8_t*)realloc(ptr,sectionStart+curSectLen);
				PtrCurSect = ptr+sectionStart;
				memcpy(PtrCurSect+16,hdr->SCP.Section12.annotatedECG, curSectLen);
				curSectLen += 16;  // current section length
			}
		}
#endif
#endif
		else {
		}

		// write to pointer field in Section 0 
		leu16a(curSect, ptr+curSect*10+6+16); // 
		leu32a(curSectLen, ptr+curSect*10+6+16+2); // length
		// Section start - must be odd. See EN1064:2005(E) Section 5.2.1 

		// write to Section ID Header
		if (curSectLen>0)
		{
			// Section 0: startpos in pointer field 
			leu32a(sectionStart+1, ptr+curSect*10+6+16+6); 

			// Section ID header (16 bytes)
			leu16a(curSect, ptr+sectionStart+2); 	// Section ID
			leu32a(curSectLen, ptr+sectionStart+4); 	// section length->section header
			ptr[sectionStart+8] = VERSION; 	// Section Version Number 
			ptr[sectionStart+9] = VERSION; 	// Protocol Version Number
			memset(ptr+sectionStart+10,0,6);
			crc = CRCEvaluate(ptr+sectionStart+2,curSectLen-2); // compute CRC
			leu16a(crc, ptr+sectionStart);
		}	
		sectionStart += curSectLen;	// offset for next section
	}
	
	if (VERBOSE_LEVEL>7) fprintf(stdout,"SOPEN_SCP_WRITE 300\n");

	// Prepare filling the data block with the ECG samples by SWRITE
	hdr->AS.rawdata = ptr+aECG->Section6.StartPtr+16+6+2*hdr->NS;
	
	hdr->AS.Header = ptr; 

	if (VERBOSE_LEVEL>7) fprintf(stdout,"SOPEN_SCP_WRITE 400\n");

	return(0);
}



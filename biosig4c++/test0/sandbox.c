/*

    sandbox is used for development and under constraction work
    The functions here are either under construction or experimental. 
    The functions will be either fixed, then they are moved to another place;
    or the functions are discarded. Do not rely on the interface in this function
       	

    $Id: sandbox.c,v 1.5 2009-04-16 20:19:17 schloegl Exp $
    Copyright (C) 2008,2009 Alois Schloegl <a.schloegl@ieee.org>
    This file is part of the "BioSig for C/C++" repository 
    (biosig4c++) at http://biosig.sf.net/ 

    BioSig is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. 


 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../biosig-dev.h"

// these functios are stubs

#ifdef WITH_GDCM
#undef WITH_DICOM

#include "gdcmReader.h"
//#include "gdcmImageReader.h"
//#include "gdcmWriter.h"
#include "gdcmDataSet.h"
#include "gdcmAttribute.h"

extern "C" int sopen_dicom_read(HDRTYPE* hdr) {

	gdcm::Reader r;
  	r.SetFileName( hdr->FileName );
  	if( !r.Read() )   
  		return 1;

  	gdcm::File &file = r.GetFile();
  	gdcm::DataSet& ds = file.GetDataSet();
/*
	{
		gdcm::Attribute<0x28,0x100> at;
		at.SetFromDataElement( ds.GetDataElement( at.GetTag() ) );
		if( at.GetValue() != 8 ) 
			return 1;
		//at.SetValue( 32 );
		//ds.Replace( at.GetAsDataElement() );
	}
*/
	{

fprintf(stdout,"attr <0x0008,0x002a>\n");
		gdcm::Attribute<0x0008,0x002a> at;
 		ds.GetDataElement( at.GetTag() );
//		at.SetFromDataElement( ds.GetDataElement( at.GetTag() ) );

		fprintf(stdout,"DCM: [0008,002a]: %i %p\n",at.GetNumberOfValues(), at.GetValue());
	}


/*
				{
				struct tm t0; 
				hdr->AS.Header[pos+14]=0;
				t0.tm_sec = atoi((char*)hdr->AS.Header+pos+12);
				hdr->AS.Header[pos+12]=0;
				t0.tm_min = atoi((char*)hdr->AS.Header+pos+10);
				hdr->AS.Header[pos+10]=0;
				t0.tm_hour = atoi((char*)hdr->AS.Header+pos+8);
				hdr->AS.Header[pos+8]=0;
				t0.tm_mday = atoi((char*)hdr->AS.Header+pos+6);
				hdr->AS.Header[pos+6]=0;
				t0.tm_mon = atoi((char*)hdr->AS.Header+pos+4)-1;
				hdr->AS.Header[pos+4]=0;
				t0.tm_year = atoi((char*)hdr->AS.Header+pos)-1900;

				hdr->T0 = tm_time2gdf_time(&t0); 
				break;
				}
			
*/
}
#endif



#ifdef __cplusplus
extern "C" {
#endif 


int sopen_eeprobe(HDRTYPE* hdr) {
	B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	B4C_ERRMSG = "asn1 currently not supported";
	return(0);
};

int sopen_zzztest(HDRTYPE* hdr) {
	B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	B4C_ERRMSG = "asn1 currently not supported";
	return(0);
};

int sopen_unipro_read(HDRTYPE* hdr) {
	B4C_ERRNUM = B4C_FORMAT_UNSUPPORTED;
	B4C_ERRMSG = "UNIPRO not supported";
	return(0);
}


#ifdef WITH_DICOM
int sopen_dicom_read(HDRTYPE* hdr) {
		char FLAG_implicite_VR = 1;	
		if (hdr->HeadLen<132) {
			hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, 132);
		    	hdr->HeadLen += ifread(hdr->AS.Header+hdr->HeadLen, 1, 132-hdr->HeadLen, hdr);
		}
		size_t pos = 0; 
		size_t count = hdr->HeadLen;
		while (!hdr->AS.Header[pos] && (pos<128)) pos++;
		if ((pos==128) && !memcmp(hdr->AS.Header+128,"DICM",4)) {
			FLAG_implicite_VR = 0;	
			pos = 132;
		}	
		else
			pos = 0;

		size_t bufsiz = 16384;
		while (!ifeof(hdr)) {
			hdr->AS.Header = (uint8_t*)realloc(hdr->AS.Header, count+bufsiz+1);
		    	count += ifread(hdr->AS.Header+count, 1, bufsiz, hdr);
		    	bufsiz *= 2;
		}
	    	ifclose(hdr); 
	    	hdr->AS.Header[count] = 0;

		uint16_t nextTag[2];

		struct tm T0; 
		char flag_t0=0; 
		char flag_ignored; 
		uint32_t Tag;
		uint32_t Len;
		nextTag[0] = *(uint16_t*)(hdr->AS.Header+pos);
		nextTag[1] = *(uint16_t*)(hdr->AS.Header+pos+2);
		while (pos < count) {	

			if (hdr->FILE.LittleEndian) {
				Tag  = (((uint32_t)l_endian_u16(nextTag[0])) << 16) + l_endian_u16(nextTag[1]);
				pos += 4;
				if (FLAG_implicite_VR) {
					Len = leu32p(hdr->AS.Header+pos);
					pos += 4; 
				}	
				else {
					// explicite_VR
					if (pos+4 > count) break;
					
					if (memcmp(hdr->AS.Header+pos,"OB",2) 
					 && memcmp(hdr->AS.Header+pos,"OW",2)
					 && memcmp(hdr->AS.Header+pos,"OF",2)
					 && memcmp(hdr->AS.Header+pos,"SQ",2)
					 && memcmp(hdr->AS.Header+pos,"UT",2)
					 && memcmp(hdr->AS.Header+pos,"UN",2) ) { 
						Len = leu16p(hdr->AS.Header+pos+2);
						pos += 4; 
					}	
					else {	
						Len = leu32p(hdr->AS.Header+pos+4);
						pos += 8; 
					}
				}	
			}
			else {
				fprintf(stdout,"Warning BigEndian Dicom not tested\n");			
				Tag  = (((uint32_t)b_endian_u16(nextTag[0])) << 16) + b_endian_u16(nextTag[1]);
				//Tag = (beu16p(hdr->AS.Header+pos)<<16) + beu16p(hdr->AS.Header+pos+2);
				Len =  beu16p(hdr->AS.Header+pos+6);
			}

			/*
			    bakup next tag, this allows setting of terminating 0
			*/
			if (pos+Len < count) {
				nextTag[0] = *(uint16_t*)(hdr->AS.Header+pos+Len);
				nextTag[1] = *(uint16_t*)(hdr->AS.Header+pos+Len+2);
				hdr->AS.Header[pos+Len] = 0;
	    		}	
			

			flag_ignored = 0;
			if (VERBOSE_LEVEL>8)
				fprintf(stdout,"        %6x:   (%04x,%04x) %8d\t%s\n",pos,Tag>>16,Tag&0x0ffff,Len,(char*)hdr->AS.Header+pos);			
				
			switch (Tag) {

			case 0x00020002: {
				hdr->NS = 1; 
				char *t = (char*)hdr->AS.Header+pos;
				while (isspace(*t)) t++;	// deblank
				char *ct[] = {	"1.2.840.10008.5.1.4.1.1.9.1.1", 
						"1.2.840.10008.5.1.4.1.1.9.1.2",
						"1.2.840.10008.5.1.4.1.1.9.1.3",
						"1.2.840.10008.5.1.4.1.1.9.2.1",
						"1.2.840.10008.5.1.4.1.1.9.3.1",
						"1.2.840.10008.5.1.4.1.1.9.4.1"
						};			
				if (!strcmp(t,*ct)) hdr->NS = 12; 
				break;
				}	
			case 0x00020010: {
				char *t = (char*)hdr->AS.Header+pos;
				while (isspace(*t)) t++;	// deblank
				char *ct[] = {	"1.2.840.10008.1.2", 
						"1.2.840.10008.1.2.1",
						"1.2.840.10008.1.2.1.99",
						"1.2.840.10008.1.2.2"
						};
				if (!strcmp(t,*ct)) FLAG_implicite_VR = 1;	
				else if (!strcmp(t,*ct+1)) FLAG_implicite_VR = 0;	
				else if (!strcmp(t,*ct+3)) FLAG_implicite_VR = 1;	
				break;
				}

			case 0x00080020:  // StudyDate 
			case 0x00080023:  // ContentDate 
				{
				hdr->AS.Header[pos+8]=0;
				T0.tm_mday = atoi((char*)hdr->AS.Header+pos+6);
				hdr->AS.Header[pos+6]=0;
				T0.tm_mon = atoi((char*)hdr->AS.Header+pos+4)-1;
				hdr->AS.Header[pos+4]=0;
				T0.tm_year = atoi((char*)hdr->AS.Header+pos)-1900;
				flag_t0 |= 1; 
				break;
				}
			case 0x0008002a:  // AcquisitionDateTime 
				{
				struct tm t0; 
				hdr->AS.Header[pos+14]=0;
				t0.tm_sec = atoi((char*)hdr->AS.Header+pos+12);
				hdr->AS.Header[pos+12]=0;
				t0.tm_min = atoi((char*)hdr->AS.Header+pos+10);
				hdr->AS.Header[pos+10]=0;
				t0.tm_hour = atoi((char*)hdr->AS.Header+pos+8);
				hdr->AS.Header[pos+8]=0;
				t0.tm_mday = atoi((char*)hdr->AS.Header+pos+6);
				hdr->AS.Header[pos+6]=0;
				t0.tm_mon = atoi((char*)hdr->AS.Header+pos+4)-1;
				hdr->AS.Header[pos+4]=0;
				t0.tm_year = atoi((char*)hdr->AS.Header+pos)-1900;

				hdr->T0 = tm_time2gdf_time(&t0); 
				break;
				}
			case 0x00080030:  // StudyTime 
			case 0x00080033:  // ContentTime 
				{
				hdr->AS.Header[pos+6]=0;
				T0.tm_sec = atoi((char*)hdr->AS.Header+pos+4);
				hdr->AS.Header[pos+4]=0;
				T0.tm_min = atoi((char*)hdr->AS.Header+pos+2);
				hdr->AS.Header[pos+2]=0;
				T0.tm_hour = atoi((char*)hdr->AS.Header+pos);
				flag_t0 |= 2; 
				break;
				}
			case 0x00080070:  // Manufacturer 
				{
				strncpy(hdr->ID.Manufacturer._field,(char*)hdr->AS.Header+pos,MAX_LENGTH_MANUF);
				hdr->ID.Manufacturer.Name = hdr->ID.Manufacturer._field;
				break;
				}	
			case 0x00081050:  // Performing Physician
				{
				strncpy(hdr->ID.Technician,(char*)hdr->AS.Header+pos,MAX_LENGTH_TECHNICIAN);
				break;
				}	
			case 0x00081090: // Manufacturer Model
				{
				const size_t nn = strlen(hdr->ID.Manufacturer.Name)+1;
				strncpy(hdr->ID.Manufacturer._field+nn,(char*)hdr->AS.Header+pos,MAX_LENGTH_MANUF-nn-1);
				hdr->ID.Manufacturer.Model = hdr->ID.Manufacturer._field+nn;
				break;
				}	
				
			case 0x00100010: // Name 
				if (!hdr->FLAG.ANONYMOUS) {
					strncpy(hdr->Patient.Name,(char*)hdr->AS.Header+pos,MAX_LENGTH_NAME);
					hdr->Patient.Name[MAX_LENGTH_NAME]=0;
				}
				break;		
			case 0x00100020: // ID 
				strncpy(hdr->Patient.Id,(char*)hdr->AS.Header+pos,MAX_LENGTH_NAME);
				hdr->Patient.Id[MAX_LENGTH_PID]=0;
				break;
				
			case 0x00100030: // Birthday 
				{
				struct tm t0; 
				t0.tm_sec = 0;
				t0.tm_min = 0;
				t0.tm_hour = 12;

				hdr->AS.Header[pos+8]=0;
				t0.tm_mday = atoi((char*)hdr->AS.Header+pos+6);
				hdr->AS.Header[pos+6]=0;
				t0.tm_mon = atoi((char*)hdr->AS.Header+pos+4)-1;
				hdr->AS.Header[pos+4]=0;
				t0.tm_year = atoi((char*)hdr->AS.Header+pos)-1900;

				hdr->Patient.Birthday = tm_time2gdf_time(&t0); 
				break;
				}
			case 0x00100040: 
				hdr->Patient.Sex = (toupper(hdr->AS.Header[pos])=='M') + 2*(toupper(hdr->AS.Header[pos])=='F');
				break;

			case 0x00101010: //Age 
				break;
			case 0x00101020: 
				hdr->Patient.Height = (uint8_t)(atof((char*)hdr->AS.Header+pos)*100.0);
				break;
			case 0x00101030: 
				hdr->Patient.Weight = (uint8_t)atoi((char*)hdr->AS.Header+pos);
				break;
				
			default:
				flag_ignored = 1;
				if (VERBOSE_LEVEL<7)
					fprintf(stdout,"ignored %6x:   (%04x,%04x) %8d\t%s\n",pos,Tag>>16,Tag&0x0ffff,Len,(char*)hdr->AS.Header+pos);			
			
			}
			if (VERBOSE_LEVEL>6)
				fprintf(stdout,"%s %6x:   (%04x,%04x) %8d\t%s\n",(flag_ignored?"ignored":"       "),pos,Tag>>16,Tag&0x0ffff,Len,(char*)hdr->AS.Header+pos);			
								
			
			pos += Len; 
		}
		if (flag_t0 == 3) hdr->T0 = tm_time2gdf_time(&T0); 

}
#endif 

#ifdef WITH_PDP
#include "../NONFREE/sopen_pdp_read.c"
#endif 

#ifdef __cplusplus
}
#endif 


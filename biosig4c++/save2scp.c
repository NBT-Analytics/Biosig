#include <stdio.h>
#include <stdlib.h>
#include "biosig.h"

int main(int argc, char **argv){
    
    HDRTYPE 	*hdr; 
    CHANNEL_TYPE* 	cp; 
    size_t 	count;
    time_t  	T0; 
	
    if (argc < 2)  	{
	fprintf(stderr,"Warning: Invalid number of arguments\n");
	return(-1);
    }
    
    hdr = sopen(argv[1], "r", NULL);
    
    if (hdr==NULL)exit(-1);

    fprintf(stderr,"FileName:\t%s\nType    :\t%i\nVersion:\t%4.2f\nHeadLen:\t%i\n",argv[1],hdr->TYPE,hdr->VERSION,hdr->HeadLen);
    fprintf(stderr,"NS:\t%i\nSPR:\t%i\nNRec:\t%Li\nDuration[s]:\t%u/%u\nFs:\t%f\n",hdr->NS,hdr->SPR,hdr->NRec,hdr->Dur[0],hdr->Dur[1],hdr->SampleRate);

    T0 = gdf_time2t_time(hdr->T0);
    fprintf(stderr,"Date/Time:\t%s\n",asctime(localtime(&T0))); 
		//T0 = gdf_time2t_time(hdr->Patient.Birthday);
		//fprintf(stdout,"Birthday:\t%s\n",asctime(localtime(&T0)));

    fprintf(stderr,"Patient:\n\tName:\t%s\n\tId:\t%s\n\tWeigth:\t%i kg\n\tHeigth:\t%i cm\n\tAge:\t%4.1f y\n",hdr->Patient.Name,hdr->Patient.Id,hdr->Patient.Weight,hdr->Patient.Height,(hdr->T0 - hdr->Patient.Birthday)/ldexp(365.25,32)); 
    T0 = gdf_time2t_time(hdr->Patient.Birthday);
    fprintf(stderr,"\tBirthday:\t%s\n",asctime(localtime(&T0))); 
/*  fprintf(stdout,"EVENT:\n\tN:\t%i\n\tFs:\t%f\n\t\n",hdr->EVENT.N,hdr->EVENT.SampleRate); 
		
    fprintf(stdout,"--%i\t%i\n", hdr->FLAG.OVERFLOWDETECTION, hdr->FLAG.UCAL);
    hdr->FLAG.OVERFLOWDETECTION = 0; 
	//	hdr->FLAG.UCAL = 1;
    fprintf(stdout,"--%i\t%i\n", hdr->FLAG.OVERFLOWDETECTION, hdr->FLAG.UCAL);
    fprintf(stdout,"2-%u\t%i\t%i\t%i\t%u\t%u\n",hdr->AS.bpb,hdr->FILE.OPEN,(int32_t)hdr->NRec,hdr->HeadLen,hdr->Dur[0],hdr->Dur[1]);
*/	
    for (int k=0; k<hdr->NS; k++) {
	cp = hdr->CHANNEL+k; 
	fprintf(stdout,"\n#%2i: %7s\t%s\t%s\t%i\t%5f\t%5f\t%5f\t%5f\t",k,cp->Label,cp->Transducer,cp->PhysDim,cp->PhysDimCode,cp->PhysMax,cp->PhysMin,cp->DigMax,cp->DigMin);
	fprintf(stderr,"\n%4.0f\t%4.0f\t%4.0f\t%5f Ohm",cp->LowPass,cp->HighPass,cp->Notch,cp->Impedance);
    }
    fprintf(stderr,"\nm1: %d %d %d %d\n",*((int16_t *)hdr->AS.rawdata),*((int16_t *)hdr->AS.rawdata+2),*(int16_t *)(hdr->AS.rawdata+4),*(int16_t *)(hdr->AS.rawdata+6));

    fprintf(stderr,"\nSCP OPENED: SUCCESSFULLY %i\n",ftell(hdr->FILE.FID));
    hdr->FLAG.OVERFLOWDETECTION = 0;
    hdr->FLAG.UCAL = 1;
    
    count = sread(hdr, 0, hdr->NRec);

    if (hdr->FILE.OPEN){
	fclose(hdr->FILE.FID);
	hdr->FILE.FID = 0;
    }

    hdr->TYPE = SCP_ECG;
//    hdr->TYPE = HL7aECG;
//    hdr->TYPE = GDF;
    sopen(argv[2], "w", hdr);
    fprintf(stderr,"File %s: OPENED SUCCESSFULLY\n", hdr->FileName);
    sclose(hdr);
    free(hdr);
}
#include <errno.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

// #include <linux/hdreg.h>
#include <string.h>

#include <stdarg.h>
#include <stdlib.h>
#include "sha1.h"
#include "xbe-header.h"


#define debug

struct Checksumstruct {
	unsigned char Checksum[20];	
	unsigned int Size_ramcopy;
	unsigned int compressed_image_start;
	unsigned int compressed_image_size;
	unsigned int Biossize_type;
}Checksumstruct;

void shax(unsigned char *result, unsigned char *data, unsigned int len)
{
	struct SHA1Context context;
	SHA1Reset(&context);
	SHA1Input(&context, (unsigned char *)&len, 4);
	SHA1Input(&context, data, len);
	SHA1Result(&context,result);	
}



int xberepair (unsigned char * xbeimage)
{
	FILE *f;
	unsigned int xbesize;
	void * xbe;

	XBE_HEADER *header;
  //  XBE_CERTIFICATE *cert;
	XBE_SECTION *sechdr;
	
	f = fopen(xbeimage, "rb");
    	if (f!=NULL) 
    	{   
  		fseek(f, 0, SEEK_END); 
         	xbesize	 = ftell(f); 
         	fseek(f, 0, SEEK_SET);
		xbe = malloc(xbesize+0x40000);    			
    		memset(xbe,0x00,xbesize+0x40000);
    		
    		fread(xbe, 1, xbesize, f);
    		fclose(f);
	        
	        header = (XBE_HEADER*) xbe;
		// This selects the First Section, we only have one
		sechdr = (XBE_SECTION *)(((char *)xbe) + (int)header->Sections - (int)header->BaseAddress);
	        
	        // Correcting overall size now
		xbesize = 0x45000;
	        header->ImageSize = xbesize; 
		
		//printf("%08x",sechdr->FileSize);                    
		

		sechdr->FileSize = 0x44000;
		sechdr->VirtualSize = 0x44000;
	        
	 //       printf("Sections: %d\n",header->NumSections);

        	shax((unsigned char *)sechdr->ShaHash, ((unsigned char *)xbe)+(int)sechdr->FileAddress ,sechdr->FileSize);
	  	
		// Write back the Image to Disk
		f = fopen(xbeimage, "wb");
    		if (f!=NULL) 
    		{   
		 fwrite(xbe, 1, xbesize, f);
        	 fclose(f);			
		}	  	
	
		free(xbe);
	}

        
	return 0;	
}


int romcopy (
		unsigned char * binname256,
		unsigned char * cromimage,
		unsigned char * binname1024
		)
{
	
	static unsigned char SHA1_result[SHA1HashSize];
	struct SHA1Context context;
	FILE *f;
	static unsigned char loaderimage[256*1024];
	static unsigned char flash256[256*1024];
	static unsigned char flash1024[1024*1024];
	static unsigned char crom[256*1024];
	static unsigned char compressedcrom[256*1024];
	
       	unsigned int romsize=0;
       	unsigned int compressedromsize=0;
       	struct Checksumstruct bootloaderstruct ;
       	unsigned int bootloderpos;
       	int a;
//       	unsigned int compressedimagestart;
       	unsigned int temp;
       	
       	memset(flash256,0x00,sizeof(flash256));
	memset(flash1024,0x00,sizeof(flash1024));
       	memset(crom,0x00,sizeof(crom));
       	memset(compressedcrom,0x00,sizeof(compressedcrom));
       	
       	a=1;
	f = fopen(binname256, "rb");
    	if (f!=NULL) 
    	{    
		fread(loaderimage, 1, 256*1024, f);
 	        fclose(f);
	} else {
		printf("Error, NO Image found\n");
		a=0;
	}
	
    	f = fopen(cromimage, "rb");
	if (f!=NULL) 
    	{    
 		fseek(f, 0, SEEK_END); 
         	romsize	 = ftell(f); 
         	compressedromsize = romsize;
         	fseek(f, 0, SEEK_SET);
    		fread(crom, 1, romsize, f);
    		fclose(f);
    	} else {
    		printf("Error, NO CROM found\n");	
    		a=0;
    	}
	
	// Break, we have a error
	if (a==0) {	
		printf("ERROR-- we can not dump our checksum ...\n");
		return 1;
	}
	
	// Ok, we have loaded both images, we can continue
	if (a==1) {	

		memcpy(&bootloderpos,&loaderimage[0x40],4);   // This can be foun in the 2bBootStartup.S
		memcpy(&bootloaderstruct,&loaderimage[bootloderpos],sizeof(struct Checksumstruct));
		
		memcpy(flash256,loaderimage,256*1024);
		memcpy(flash1024,loaderimage,256*1024);
		
		// We make now sure, there are some "space" bits and we start oranized with 16
		temp = bootloderpos + bootloaderstruct.Size_ramcopy;
		temp = temp & 0xfffffff0;
		temp = temp + 0x10;
		bootloaderstruct.compressed_image_start = temp;
		bootloaderstruct.compressed_image_size =  compressedromsize;
		
		bootloaderstruct.Biossize_type = 0; // Means it is a 256 kbyte Image
		memcpy(&flash256[bootloderpos],&bootloaderstruct,sizeof(struct Checksumstruct));
		bootloaderstruct.Biossize_type = 1; // Means it is a 1MB Image
		memcpy(&flash1024[bootloderpos],&bootloaderstruct,sizeof(struct Checksumstruct));		
		
		#ifdef debug		
		printf("%08x\n",bootloderpos);
		printf("%08x\n",bootloaderstruct.Size_ramcopy);		
		printf("%08x\n",bootloaderstruct.compressed_image_start);
		printf("%08x\n",bootloaderstruct.compressed_image_size);
		#endif
		
		// We have calculated the size of the kompressed image and where it can start (where the 2bl ends)
		// we make now the hash sum of the 2bl itself			

		// This is for 256 kbyte image
		// We start with offset 20, as the first 20 bytes are the checksum
		SHA1Reset(&context);
		SHA1Input(&context,&flash256[bootloderpos+20],(bootloaderstruct.Size_ramcopy-20));
		SHA1Result(&context,SHA1_result);
	      
	        // We dump now the SHA1 sum into the 2bl loader image
	      	memcpy(&flash256[bootloderpos],&SHA1_result[0],20);

		#ifdef debug
		printf("2bl Hash 256Kb image :");
		for(a=0; a<SHA1HashSize; a++) {
			printf("%02X",SHA1_result[a]);
		}
	      	printf("\n");
	      	#endif


		// This is for 1MB Image kbyte image
		// We start with offset 20, as the first 20 bytes are the checksum
		SHA1Reset(&context);
		SHA1Input(&context,&flash1024[bootloderpos+20],(bootloaderstruct.Size_ramcopy-20));
		SHA1Result(&context,SHA1_result);
	      
	        // We dump now the SHA1 sum into the 2bl loader image
	      	memcpy(&flash1024[bootloderpos],&SHA1_result[0],20);
              
		#ifdef debug
		printf("2bl Hash 1MB image   :");
		for(a=0; a<SHA1HashSize; a++) {
			printf("%02X",SHA1_result[a]);
		}
	      	printf("\n");
	      	#endif
	      	
		// In 1MB flash we need the Image 2 times, as ... you know
		memcpy(&flash1024[3*256*1024],&flash1024[0],256*1024);

                // Ok, the 2BL loader is ready, we now go to the "Kernel"


	      	
	        // The first 20 bytes of the compressed image are the checksum
		memcpy(&flash256[bootloaderstruct.compressed_image_start+20],&crom[0],compressedromsize);
		SHA1Reset(&context);
		SHA1Input(&context,&flash256[bootloaderstruct.compressed_image_start+20],compressedromsize);
		SHA1Result(&context,SHA1_result);                                
		memcpy(&flash256[bootloaderstruct.compressed_image_start],SHA1_result,20);			
    			
		memcpy(&flash1024[bootloaderstruct.compressed_image_start+(1*256*1024)],SHA1_result,20);			
		memcpy(&flash1024[bootloaderstruct.compressed_image_start+20+(1*256*1024)],&crom[0],compressedromsize);
		memcpy(&flash1024[bootloaderstruct.compressed_image_start+(2*256*1024)],SHA1_result,20);			
	      	memcpy(&flash1024[bootloaderstruct.compressed_image_start+20+(2*256*1024)],&crom[0],compressedromsize);
	      	
	      	
	      	
	      	
	      	// Write the 256 /1024 Kbyte Image Back
	      	f = fopen(binname256, "wb");               
		fwrite(flash256, 1, 256*1024, f);
         	fclose(f);	
		
	      	f = fopen(binname1024, "wb");               
		fwrite(flash1024, 1, 1024*1024, f);
         	fclose(f);	

		printf("Checksum Successfully Created for %s\n",binname256);	              
		
	} 
	

	return 0;	
}        

int main (int argc, const char * argv[])
{
	if( argc < 3 ) 	return -1;
	
	if (strcmp(argv[1],"-xbe")==0) { 
		xberepair((unsigned char*)argv[2]);
		}
	if (strcmp(argv[1],"-rom")==0) { 
		if( argc < 4 ) return -1;
		romcopy((unsigned char*)argv[2],(unsigned char*)argv[3],(unsigned char*)argv[4]);
		}
	return 0;	
}

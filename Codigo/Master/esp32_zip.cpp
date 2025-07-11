//http://www.ross.net/crc/crcpaper.html

#include <esp32_zip.h>
#include <SD.h>

//OBS 7zip can extract without any problem, regualr zip cannot

///////////////////////
// Data header

//header signature
const long int header_sign=0x04034b50;

//version needed
const short int header_min_version=0x0014;//PKzip 2.0 (from 1993)

//General purpose Bit flag
const short int header_GP_flag=0x0000;

//Compression method, 0-None
const short int header_compression=0x0000;

//last modified time
const short int header_mod_time=0x0000;

//last modified date
const short int header_mod_date=0x0000;

//crc32 of uncompressed data
long int header_crc32;
const short int header_crc32_offset=14;

//size compressed
long int header_size_compressed;
const short int header_size_compressed_offset=18;

//size uncompressed
long int header_size_uncompressed;
const short int header_size_uncompressed_offset=22;

//filename length n
short int header_length_filename;
const short int header_length_filename_offset=26;

//extra field length m
short int header_length_extrafield=0x0000;

//n bytes/chars
//char header_file_name[];
const short int header_filename_offset=30;


//m chars/bytes
char header_extra_field[0];

/////////////////////////////////////
//Data descriptor-Not used

////////////////////////////////////
//central directory

//central signature
const long int central_sign=0x02014b50;

//version made
const short int central_made_version=0x0000;//based on OS, 0000 should be standard FAT system which FS is based on

//version needed
const short int central_min_version=0x0014;//PKZIP 2.0(from 1993)

//central genearal purpose flag
const short int central_GP_flag=0x0000;

//central compression method
const short int central_compression=0x0000;

//last modified time
const short int central_mod_time=0x0000;

//last modified date
const short int central_mod_date=0x0000;

// crc32 of uncompressed data (sum of all files?)
long int central_crc32;
const short int central_crc32_offset=16;

// compressed size
long int central_size_compressed;
const short int central_size_compressed_offset=20;

//size uncompressed
long int central_size_uncompressed;
const short int central_size_uncompressed_offset=24;

//filename length n
short int central_length_filename;
const short int central_length_filename_offset=28;

//extra field length m
short int central_length_extrafield=0x0000;

//file comment length k
short int central_length_comment=0x0000;


//disk number where file starts all in one file give 0?
const short int disk_file_start=0x0000;

//internal file attributes ?
short int internal_attributes=0x0000;

//external file attributes?
long int external_attributes=0x00000020;//from the testzip 

long int start_to_fileheader;//byteaddress for the header signature
const short int start_to_fileheader_offset=42;

//n bytes/chars
//char central_file_name[];
const short int central_filename_offset=46;


//m chars/bytes
char central_extra_field[0];

//k chars/bytes
char central_comment[0];

///////////////////////////////////////
//end of central directory record

//end signature
const long int end_signature=0x06054b50;

//nmbr of disks 1(=?)
short int nmbr_of_disks=0;

//disk where central directory starts 0(1?)
short int start_disk=0;

//central records on this disk
short int centrals_on_disk;

//total centrals, assumed only one disk
short int centrals_tot=centrals_on_disk;

//size of central direcoty
long int centrals_size;

//offset to start of central
long int central_offset;

//comment length
short int end_comment_length=0x0000;

//comment
//char end_comment[];

const unsigned long crctab[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,  0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,  0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,  0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,  0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,  0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,  0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,  0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,  0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,  0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,  0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,  0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,  0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,  0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,  0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,  0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,  0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,  0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,  0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,  0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,  0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,  0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,  0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,  0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,  0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,  0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,  0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,  0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,  0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,  0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

//https://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte



using namespace N;
bool ZipWriter::ZipWrite(File zipfilein, String filepaths[], int nmbr_to_zipin){
    try{//catches any errors, rough
        zipfile=zipfilein;//to  what file should the files be zipped
        
        nmbr_to_zip=nmbr_to_zipin;//how many files should be zipped
        file_offset[0]=0;//first entry is at positipn 0

        for(fileindex=0; fileindex<nmbr_to_zip;fileindex++){//for each filepath
            File file=SD.open(filepaths[fileindex]);//open a filepath as a fs file(SD)
            filesizes[fileindex]=file.size();//get the size and save it to the filesize list, needed at mltiple locations
            write_header(filepaths[fileindex]);//write the header, name size etc

            write_data( file);//write the data, clculate the crc32 and write it

        }
        central_offset=file_offset[nmbr_to_zip];//the central directory starts where the next header should have started
        for (fileindex=0;fileindex<nmbr_to_zip;fileindex++){//write the central directory for each file zipped
            write_central_directory(filepaths[fileindex]);
        }

        central_size=endoffile-central_offset;//calculate the size of the central directory

        write_end();//Write the end of file stuff

        zipfile.close();//close the zipfile
        return true;//return true if everything worked
    }catch(std::exception& e){
        Serial.println("Error message: "+String(e.what()));//Print the error message
        return false;//return false
    }

}

void ZipWriter::write_header(String zipped_filename){
    String filename=zipped_filename.substring(zipped_filename.lastIndexOf("/")+1,-1);//get only the filename, removes the path part that is used othervise
    zipfile.seek(endoffile);//set cursor at end of file
    int templength=filename.length();//zipped_filename.length();//get the length of the filename
    zipfile.write((byte*)&header_sign,4);//convert value to bytes, how many bytes that should be written
    zipfile.write((byte*)&header_min_version,2);
    zipfile.write((byte*)&header_GP_flag,2);
    zipfile.write((byte*)&header_compression,2);
    zipfile.write((byte*)&header_mod_time,2);
    zipfile.write((byte*)&header_mod_date,2);
    zipfile.write((byte*)&header_crc32,4);//empty right now, will be filled during data write
    zipfile.write((byte*)&filesizes[fileindex],4);
    zipfile.write((byte*)&filesizes[fileindex],4);//no compression makes before and after the same
    zipfile.write((byte*)&templength,2);
    zipfile.write((byte*)&header_length_extrafield,2);//not used
    for(int letter=0;letter<templength;letter++){//write the filename
        zipfile.write(filename[letter]);
    }
    zipfile.write((byte*)&header_extra_field,0);//not used nmbr if bytes written=0

    endoffile=30+templength;//header is 30 static bytes + the filename
    headsizes[fileindex]=30+templength;//for later use
}


void ZipWriter::write_data(File datafile){
    String filename=String(datafile.name());//get the name of the fs file
    int length_filename=filename.length();//get length of fs file
    int bytecounter=0;//nmbr of read bytes
    bool initff=true;//should the initial byte be xor with xff
    unsigned long int crc_reg=0x00000000;//initialize the crc register
    bool reflectbytes=true;//should the data and final be reflected
    int buffersize=256;
    
    zipfile.seek(file_offset[fileindex]+headsizes[fileindex]);//move the file to the data section
    
    char databuffer[buffersize];//make a buffer for the file
    int readbytes=datafile.readBytes(databuffer, buffersize);//read up to buffersize bytes into the databuffer and return the number of bytes read
    while(readbytes>0){//while there are bytes that has been read
        bytecounter+=readbytes;//increase the bytecounter
        for(int rb=0;rb<readbytes;rb++){//for every byte read
            if (reflectbytes){//if reflected
                if (initff){//if the crc_reg should be initiated as 0xffffffff
                    crc_reg=0xffffffff;  
                    byte t =(0xff)^(unsigned long)databuffer[rb];//first byte XOR with 0xff to get the table index
                    crc_reg=(crc_reg>>8)^((crctab[t]));//rightshift then XOR in the value from the table
                    if(rb==0){
                        initff=false;//the initial is done
                    }
                }else{ 
                    byte t =(crc_reg)^(unsigned long)databuffer[rb];//XOR the crc register with new byte
                    crc_reg=(crc_reg>>8)^((crctab[t]));//rightshift and XOR witht the table
                    
                }
            }else{//not used for regular crc32 but for ex BZIP2
                byte t=(crc_reg>>24) &0xff;//get the byte that will be thrown away at the leftshift to look up the table with        
                if(initff){
                    crc_reg=(crc_reg<<8)|(databuffer[rb]^0xff);//leftshift 1 byte,  bitwise_OR in the next byte 
                    
                    if (rb==3){//reads the first four chars in the message to inititate the crc register
                        initff=false;
                    }
                }else{
                    crc_reg=(crc_reg<<8)|(databuffer[rb]);
                }           
                crc_reg=crc_reg^(crctab[t]);//XOR the crc_register with the table index t
            }        
            zipfile.write(databuffer[rb]);//write the byte to file
        }
        readbytes=datafile.readBytes(databuffer, buffersize);//read new chars to buffer
    }
    if (!reflectbytes){//if not reflected the message must be "pushed out"of the crc register
        for (int augmentbyte=0; augmentbyte<4;augmentbyte++){//after the message are zeroes
        byte t=(crc_reg>>24) &0xff;//table index
        crc_reg=(crc_reg<<8)|0x00;//leftshift in 0x00
        crc_reg=crc_reg^(crctab[t]);
        }
    }
    
    size_t endoffile_position=zipfile.position();//get the cursors postion
    file_offset[fileindex+1]=endoffile_position;//update the  list of adresses
    filesizes[fileindex]=bytecounter;//update the size of the file
    file_crc32[fileindex]=~crc_reg;//update the crc array, ~ has the same function as ^0xffffffff 
    endoffile=endoffile_position;//update end of file variable

    zipfile.seek(file_offset[fileindex]+header_crc32_offset);//set cursor at crc32 position in the header
    zipfile.write((byte*)&file_crc32[fileindex], 4);//write the crc32 number 
    zipfile.write((byte*)&bytecounter,4);//write the filesize
    zipfile.write((byte*)&bytecounter,4);//because of no compression its the same
}

void ZipWriter::write_central_directory(String zipped_filename){
    
    String filename=zipped_filename.substring(zipped_filename.lastIndexOf("/")+1,-1);//gets only the filename 

    int templength=filename.length();
    
    zipfile.seek(endoffile);//set cursor at the end of file
    zipfile.write((byte*)&central_sign,4);//write ostly constants
    zipfile.write((byte*)&central_made_version,2);
    zipfile.write((byte*)&central_min_version,2);
    zipfile.write((byte*)&central_GP_flag,2);
    zipfile.write((byte*)&central_compression,2);
    zipfile.write((byte*)&header_mod_time,2);
    zipfile.write((byte*)&header_mod_date,2);
    zipfile.write((byte*)&file_crc32[fileindex],4);//get the currents files values from the arrays
    zipfile.write((byte*)&filesizes[fileindex],4);
    zipfile.write((byte*)&filesizes[fileindex],4);
    zipfile.write((byte*)&templength,2);
    zipfile.write((byte*)&central_length_extrafield,2);
    zipfile.write((byte*)&central_length_comment, 2);
    zipfile.write((byte*)&disk_file_start,2);
    zipfile.write((byte*)&internal_attributes,2);
    zipfile.write((byte*)&external_attributes,4);
    zipfile.write((byte*)&file_offset[fileindex],4);
    
    for(int letter=0;letter<templength;letter++){
        zipfile.write(filename[letter]);
    }
    zipfile.write((byte*)&central_extra_field,0);
    zipfile.write((byte*)&central_comment,0);
    endoffile=zipfile.position();//update end of file to cursor current postion
}


void ZipWriter::write_end(){//write the end of file

    zipfile.seek(endoffile);//set cursor at the end

    zipfile.write((byte*)&end_signature, 4);
    zipfile.write((byte*)&nmbr_of_disks, 2);//it is assumed that there are no multiple disks
    zipfile.write((byte*)&start_disk, 2);
    zipfile.write((byte*)&nmbr_to_zip, 2);
    zipfile.write((byte*)&nmbr_to_zip, 2);
    zipfile.write((byte*)&central_size, 4);
    zipfile.write((byte*)&central_offset, 4);
    zipfile.write((byte*)&end_comment_length, 2);
}

bool compressLogsToZip(const char* zipFilePath) {
    // Crear un nuevo archivo ZIP en la tarjeta SD
    File zipFile = SD.open(zipFilePath, FILE_WRITE);
    if (!zipFile) {
        Serial.println("Error creando el archivo ZIP");
        return false;
    }

    // Lista de archivos a comprimir
    String logFiles[] = {"/datalog.csv", "/activation_log.csv", "/weatherlog.csv", "/forecastlog.csv", "/data_slave.csv"};
    int numFiles = sizeof(logFiles) / sizeof(logFiles[0]);

    // Inicializar el objeto ZipWriter
    N::ZipWriter zipWriter;

    // Comprimir los archivos en el archivo ZIP
    if (zipWriter.ZipWrite(zipFile, logFiles, numFiles)) {
        Serial.println("Archivos comprimidos correctamente");
    } else {
        Serial.println("Error al comprimir los archivos");
        zipFile.close();
        return false;
    }

    zipFile.close();
    return true;
}
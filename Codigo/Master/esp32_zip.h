#include "SD.h"

namespace N {
    class ZipWriter {
    public:
        bool ZipWrite(File zipfile, String filepaths[], int nmbr_to_zip);
        bool compressLogsToZip(const char* zipFilePath);

    private:
        File zipfile;
        short nmbr_to_zip;
        short fileindex = 0;
        int endoffile = 0;
        int file_offset[256] = {};
        int headsizes[256] = {};
        unsigned long file_crc32[256] = {};
        int filesizes[256] = {};
        int central_size = 0;
        int central_offset = 0;
        unsigned long b32 = 0;

        void write_header(String zipped_filename);
        void write_data(File datafile);
        void write_central_directory(String zipped_filename);
        void write_end();
        unsigned long int crctable[256] = {};
    };
}
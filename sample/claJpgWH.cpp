#include <stdio.h>
#include <stdlib.h>
#include "../ImgFmt/JpgDecoder.hpp"
#include "../misc/ExArgv.h"
#include "../misc/misc.hpp"


void oneFile(const char* src_name);
char* fname_baseName(char const* adr);


int main(int argc, char* argv[])
{
    ExArgv_conv(&argc, &argv);

    printf("mkdir sml\n");
    printf("mkdir tate\n");
    printf("mkdir yoko\n");
    printf("\n");
    for (unsigned i = 1; i < argc; ++i) {
        oneFile(argv[i]);
    }
    return 0;
}


void oneFile(const char* src_name)
{
    std::vector<unsigned char> buf;
    bool rc = file_load(src_name, buf);
    if (!rc) {
        printf("%s : file load error\n", src_name);
        return;
    }

    JpgDecoder dec(&buf[0], buf.size());
    std::vector<unsigned char> img( dec.imageByte() );
    unsigned w = dec.width();
    unsigned h = dec.height();
    char const* type = "sml";
    if (w * h >= 512*384) {
        if (w > h) {
            type = "yoko";
        } else {
            type = "tate";
        }
    }
    const char* basename = fname_baseName(src_name);
    printf("move \"%s\" \"%s/%s\"\n", src_name, type, basename);
}


char* fname_baseName(char const* adr)
{
    char const* p = adr;
    while (*p) {
        if (*p == ':' || *p == '/' || *p == '\\')
            adr = (char const*)p + 1;
        ++p;
    }
    return (char*)adr;
}

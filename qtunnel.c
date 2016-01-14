#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

struct_options options;
char *short_opts = "b:c:l:g:s:"
static struct option long_opts[] = {
    {"backend", required_argument, NULL, 'b'},
    {"clientmode", required_argument, NULL, 'c'},
    {"listen", required_argument, NULL, 'l'},
    {"logto", required_argument, NULL, 'g'},
    {"secret", required_argument, NULL, 's'},
    {0, 0, 0, 0}
};

int main(int argc, char *argv[]) {
    while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
        switch(c) {
            case ‘b’: 
        }
    }
}

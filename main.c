
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* should be in stdio but not on Windows apparently */
ssize_t getline(char **buf, size_t *bufsiz, FILE *fp);
ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp);


#include "xypicmic.h"
#include "color.h"
#include "mlog.h"

int main(int argc, char** argv)
{
    int ret = EXIT_SUCCESS;
    char* txtptr = NULL;
    const char* event_file_name = "events.txt";
    
    if (argc > 1) {
       event_file_name = argv[1];   
    }
    
    FILE* event_file = fopen(event_file_name, "r");
    if (event_file == NULL) {
        LOG_ERROR("%s: %s\n", __func__, strerror(errno));
        ret = EXIT_FAILURE;
        goto main_cleanup;
    }
  
    
    int event[MAX_EVENT_SIZE];
    size_t event_size = 0;
    struct event_lines lines;
    struct event_fat_lines fat_lines;
    struct event_clusters clusters;
    size_t txt_maxsize = 0;
    int event_number = 0;
    while(getline(&txtptr, &txt_maxsize, event_file) > 0) {
        
        memset(event, 0, MAX_EVENT_SIZE*sizeof(*event)); 
        event_size = 0;

        int ret_scanf = 0;
        int n_scanf = 0;
        int txtoffset = 0;
        
        while (1) {
            if (event_size >= MAX_EVENT_SIZE) {
                LOG_ERROR("%s: event too large", __func__);
                break;
            }
            ret_scanf = sscanf((txtptr+txtoffset), " %d%n", &event[event_size], &n_scanf);
            
            if (ret_scanf == 1) {
                txtoffset && ++event_size; // ignore first number on line
                txtoffset += n_scanf;
            } else {
                break;
            }
        }
        ++event_number;
        
        /*printf("%zu: ", event_size);
        for(size_t i=0; i<event_size; ++i){
            printf("%d ", event[i]);
        }
        puts("");*/
        printf("-------------------------------------- %d --------------------------------------\n", event_number);
        
        fill_event_lines(&lines, event, event_size);
        //print_event_lines(&lines);
        fill_event_fat_lines(&fat_lines, &lines);
        //print_event_fat_lines(&fat_lines);
        fill_event_clusters(&clusters, &fat_lines);
        print_event_clusters(&clusters);
    }

main_cleanup:
    if(txtptr) {
        free(txtptr);
    }
    if (event_file) { 
        fclose(event_file);
    }
    return ret;
}
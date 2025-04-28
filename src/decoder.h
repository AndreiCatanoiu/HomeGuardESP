#ifndef DECODER_H
#define DECODER_H

#include <ctype.h>

typedef struct 
{
    const char *name;              
    const char *params;            
    const char *info;              
    void (*function)(char *args);  
} Cmd_t;

extern Cmd_t *exec;
extern int exec_count;
extern Cmd_t *query;
extern int query_count;

void decoder_init(void);
void decoder_process_command(const char *cmd);
void command_task(void *pvParameters);


void id_encoder_base64(const char *input, char *output, size_t output_size);
void id_decoder_base64(const char *input, char *output, size_t output_size);

#endif
#ifndef DECODER_H
#define DECODER_H

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

#endif
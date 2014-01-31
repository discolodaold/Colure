#ifndef QUEST_GRAPH
#define QUEST_GRAPH

void new_node(const char *name);

void node_getText(const char *name, const char *param, char **result);
void node_getInteger(const char *name, const char *param, int64_t *result);
void node_getReal(const char *name, const char *param, double *result);
void node_getBlob(const char *name, const char *param, size_t *length, byte **result);
void node_setText(const char *name, const char *param, const char *value);
void node_setInteger(const char *name, const char *param, int64_t value);
void node_setReal(const char *name, const char *param, double value);
void node_setBlob(const char *name, const char *param, size_t length, byte value);

#endif
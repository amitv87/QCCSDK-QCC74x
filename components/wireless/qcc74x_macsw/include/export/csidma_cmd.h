#ifndef _CISDMA_CMD_H_
#define _CISDMA_CMD_H_

void cmd_csidma_start(int argc, char **argv);
void cmd_csidma_stop(int argc, char **argv);
void cmd_csidma_ready(int argc, char **argv);
void cmd_csidma_debug(int argc, char **argv);
void cmd_csidma_force_ftm(int argc, char **argv);
void cmd_csidma_force_ftm_mac_set(int argc, char **argv);

#endif

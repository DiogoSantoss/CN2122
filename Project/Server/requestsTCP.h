#ifndef REQUESTSTCP_DOT_H
#define REQUESTSTCP_DOT_H

void requestErrorTCP(userData user, serverData server, int fd);
void processULS(userData user, serverData server, int fd);
void processPST(userData user, serverData server, int fd);

#endif
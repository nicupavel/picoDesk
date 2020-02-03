int USBConnect(void);
void USBDisconnect(void);
long USBWriteReport(char *pReport, unsigned long dwReportSize);
long USBReadReport(char *pReport, unsigned long dwReportSize);

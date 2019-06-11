
#ifndef MAIN_QR_RECOGINIZE_H_
#define MAIN_QR_RECOGINIZE_H_

enum{
	RECONGIZE_OK,
	RECONGIZE_FAIL
};

//void qr_recoginze(void *pdata);
void qr_recoginze(void *pdata, uint8_t *buf);
#endif
